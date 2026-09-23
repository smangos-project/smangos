#include "BotCharSelect.h"
#include "BotInstanceConnect.h"
#include "ClassicOpcodes.h"
#include "IO/Networking/IpAddress.h"
#include "Log.h"
#include <cstdio>

std::shared_ptr<BotCharSelect> BotCharSelect::Create(IO::IoContext* ioContext, std::shared_ptr<BotWorldConnection> connection)
{
    return std::shared_ptr<BotCharSelect>(new BotCharSelect(ioContext, std::move(connection)));
}

BotCharSelect::BotCharSelect(IO::IoContext* ioContext, std::shared_ptr<BotWorldConnection> connection)
    : m_ioContext(ioContext), m_connection(std::move(connection))
{
}

void BotCharSelect::EnterWorld(Options options, CharSelectCallback callback)
{
    m_options = std::move(options);
    m_callback = std::move(callback);

    std::shared_ptr<BotCharSelect> self = shared_from_this();
    m_connection->SetPacketCallback([self](BotPacket packet) { self->OnPacket(std::move(packet)); });
    m_connection->SetDisconnectCallback([self]() { self->OnDisconnect(); });

    m_step = Step::WaitingEnumCharacters;
    m_connection->Send(ClassicEnumCharacters{});
}

void BotCharSelect::OnPacket(BotPacket packet)
{
    switch (m_step)
    {
        case Step::WaitingEnumCharacters:
            if (packet.Opcode == CLASSIC_SMSG_ENUM_CHARACTERS_RESULT)
                HandleEnumCharactersResult(packet);
            break;
        case Step::WaitingCreateCharResponse:
            if (packet.Opcode == CLASSIC_SMSG_CREATE_CHAR)
                HandleCreateCharResponse(packet);
            break;
        case Step::WaitingLoginVerifyWorld:
            if (packet.Opcode == CLASSIC_SMSG_LOGIN_VERIFY_WORLD)
                HandleLoginVerifyWorld(packet);
            else if (packet.Opcode == CLASSIC_SMSG_CONNECT_TO)
                HandleConnectTo(packet);
            break;
        case Step::Done:
            break;
    }
    // Anything else -- SMSG_UPDATE_OBJECT and whatever else the server sends before the packet
    // this step is waiting on -- is ignored here, not fatal (bot-phase-1.md: "SMSG_UPDATE_OBJECT
    // is received and skipped").
}

void BotCharSelect::OnDisconnect()
{
    if (m_step != Step::Done)
        Fail("world connection closed during CharSelect/EnteringWorld");
}

void BotCharSelect::HandleEnumCharactersResult(BotPacket& packet)
{
    ClassicEnumCharactersResult result;
    if (!result.Read(packet.Data))
    {
        Fail("malformed SMSG_ENUM_CHARACTERS_RESULT");
        return;
    }

    if (result.CharacterCount > 0)
    {
        m_step = Step::WaitingLoginVerifyWorld;
        SendPlayerLogin(result.FirstCharacterGuid);
        return;
    }

    if (!m_options.CreateIfNone)
    {
        Fail("no characters and CreateIfNone is false");
        return;
    }

    ClassicCreateCharacter create;
    create.Name = m_options.CharacterName;
    create.Race = m_options.Race;
    create.Class = m_options.Class;
    create.Gender = m_options.Gender;

    m_step = Step::WaitingCreateCharResponse;
    m_connection->Send(create);
}

void BotCharSelect::HandleCreateCharResponse(BotPacket& packet)
{
    ClassicCreateCharResponse response;
    if (!response.Read(packet.Data))
    {
        Fail("malformed SMSG_CREATE_CHAR");
        return;
    }

    if (response.Result != CLASSIC_CHAR_CREATE_SUCCESS)
    {
        Fail("character creation failed, result " + std::to_string(response.Result));
        return;
    }

    m_step = Step::WaitingLoginVerifyWorld;
    SendPlayerLogin(response.Guid);
}

void BotCharSelect::HandleLoginVerifyWorld(BotPacket& packet)
{
    ClassicLoginVerifyWorld verify;
    if (!verify.Read(packet.Data))
    {
        Fail("malformed SMSG_LOGIN_VERIFY_WORLD");
        return;
    }

    Succeed();
}

void BotCharSelect::HandleConnectTo(BotPacket& packet)
{
    ClassicConnectTo connectTo;
    if (!connectTo.Read(packet.Data))
    {
        Fail("malformed SMSG_CONNECT_TO");
        return;
    }

    // From here on only the Instance connection matters to this state machine; the Realm
    // connection stays open (SMSG_PONG/SMSG_LOGOUT_COMPLETE still route through it, see
    // vibemangos's Opcodes.cpp PacketRoute::Realm entries) but stops driving OnPacket.
    m_realmConnection = m_connection;
    m_connection->SetPacketCallback(nullptr);
    m_connection->SetDisconnectCallback(nullptr);

    char addressStr[16];
    snprintf(addressStr, sizeof(addressStr), "%u.%u.%u.%u",
        connectTo.AddressIPv4[0], connectTo.AddressIPv4[1], connectTo.AddressIPv4[2], connectTo.AddressIPv4[3]);

    BotInstanceConnect::Options instanceOptions;
    instanceOptions.WorldAddress = addressStr;
    instanceOptions.WorldPort = connectTo.Port;
    instanceOptions.SessionKey = m_options.SessionKey;
    instanceOptions.PackedKey = connectTo.Key;
    instanceOptions.VerifyServerSignature = m_options.VerifyServerSignature;

    std::shared_ptr<BotCharSelect> self = shared_from_this();
    std::shared_ptr<BotInstanceConnect> instanceConnect = BotInstanceConnect::Create(m_ioContext);
    instanceConnect->Connect(instanceOptions, [self](BotInstanceConnect::Result result)
    {
        if (!result.Success)
        {
            self->Fail("instance connect: " + result.Error);
            return;
        }

        self->m_connection = result.Connection;
        self->m_connection->SetPacketCallback([self](BotPacket packet) { self->OnPacket(std::move(packet)); });
        self->m_connection->SetDisconnectCallback([self]() { self->OnDisconnect(); });
        // m_step is already WaitingLoginVerifyWorld from HandleEnumCharactersResult/
        // HandleCreateCharResponse -- nothing else to do until it arrives on this connection.
    });
}

void BotCharSelect::SendPlayerLogin(ClassicGuid const& guid)
{
    m_characterGuid = guid;

    ClassicPlayerLogin login;
    login.Guid = guid;
    login.FarClip = m_options.FarClip;
    m_connection->Send(login);
}

void BotCharSelect::Fail(std::string const& error)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotCharSelect: %s", error.c_str());
    m_step = Step::Done;
    // Breaks the m_connection <-> callback(self) reference cycle SetPacketCallback/
    // SetDisconnectCallback created in EnterWorld() -- without this, self and the connection
    // keep each other alive forever (a real leak, found chasing an intermittent crash this
    // session; see BotWorldConnection.h's own note on Set*Callback for the retargeting model
    // this is part of). Safe to call from inside a dispatch: DispatchPacket/Close already copy
    // the callback out before invoking it.
    if (m_connection)
    {
        m_connection->SetPacketCallback(nullptr);
        m_connection->SetDisconnectCallback(nullptr);
    }
    if (m_callback)
    {
        CharSelectCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = false;
        result.Error = error;
        result.Connection = m_connection;
        result.RealmConnection = m_realmConnection;
        callback(std::move(result));
    }
}

void BotCharSelect::Succeed()
{
    m_step = Step::Done;
    if (m_connection)
    {
        m_connection->SetPacketCallback(nullptr);
        m_connection->SetDisconnectCallback(nullptr);
    }
    if (m_callback)
    {
        CharSelectCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = true;
        result.CharacterGuid = m_characterGuid;
        result.Connection = m_connection;
        result.RealmConnection = m_realmConnection;
        callback(std::move(result));
    }
}
