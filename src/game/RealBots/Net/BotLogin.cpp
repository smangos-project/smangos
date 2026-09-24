#include "BotLogin.h"
#include "Log.h"

std::shared_ptr<BotLogin> BotLogin::Create(IO::IoContext* ioContext)
{
    return std::shared_ptr<BotLogin>(new BotLogin(ioContext));
}

BotLogin::BotLogin(IO::IoContext* ioContext) : m_ioContext(ioContext)
{
}

void BotLogin::Login(Options options, LoginCallback callback)
{
    m_options = std::move(options);
    m_callback = std::move(callback);

    if (m_options.StopAt == BotStage::LoginPortal)
    {
        StopEarly();
        return;
    }

    BotBnetLogin::Options bnetOptions;
    bnetOptions.LoginAddress = m_options.LoginAddress;
    bnetOptions.RestPort = m_options.RestPort;
    bnetOptions.RpcPort = m_options.RpcPort;
    bnetOptions.AccountName = m_options.AccountName;
    bnetOptions.Password = m_options.Password;
    bnetOptions.RealmName = m_options.RealmName;
    bnetOptions.RealmId = m_options.RealmId;
    bnetOptions.ClientBuild = m_options.ClientBuild;
    bnetOptions.StopAt = m_options.StopAt;

    std::shared_ptr<BotLogin> self = shared_from_this();
    std::shared_ptr<BotBnetLogin> bnetLogin = BotBnetLogin::Create(m_ioContext);
    bnetLogin->Login(bnetOptions, [self](BotBnetLogin::Result result) { self->OnBnetLogin(std::move(result)); });
}

void BotLogin::OnBnetLogin(BotBnetLogin::Result result)
{
    if (!result.Success)
    {
        Fail("bnet login: " + result.Error);
        return;
    }
    if (result.Stopped || m_options.StopAt == BotStage::WorldConnecting)
    {
        StopEarly();
        return;
    }

    BotWorldHandshake::Options handshakeOptions;
    handshakeOptions.WorldAddress = result.WorldAddress;
    handshakeOptions.WorldPort = result.WorldPort;
    handshakeOptions.SessionKey = result.SessionKey;
    handshakeOptions.RealmJoinTicket = result.RealmJoinTicket;
    handshakeOptions.RealmId = m_options.RealmId;
    handshakeOptions.VerifyServerSignature = m_options.VerifyServerSignature;

    std::shared_ptr<BotLogin> self = shared_from_this();
    std::shared_ptr<BotWorldHandshake> handshake = BotWorldHandshake::Create(m_ioContext);
    handshake->Handshake(handshakeOptions, [self](BotWorldHandshake::Result result) { self->OnWorldHandshake(std::move(result)); });
}

void BotLogin::OnWorldHandshake(BotWorldHandshake::Result result)
{
    if (!result.Success)
    {
        Fail("world handshake: " + result.Error);
        return;
    }
    if (m_options.StopAt == BotStage::WorldHandshake)
    {
        // No redirect has happened yet -- this is still the Realm connection, just encrypted
        // and authenticated, sitting idle before CharSelect's CMSG_ENUM_CHARACTERS.
        StopEarly(result.Connection);
        return;
    }

    BotCharSelect::Options charOptions;
    charOptions.CreateIfNone = m_options.CreateIfNone;
    charOptions.CharacterName = m_options.CharacterName;
    charOptions.Race = m_options.Race;
    charOptions.Class = m_options.Class;
    charOptions.Gender = m_options.Gender;
    charOptions.FarClip = m_options.FarClip;
    charOptions.SessionKey = result.SessionKey;
    charOptions.VerifyServerSignature = m_options.VerifyServerSignature;
    charOptions.StopAt = m_options.StopAt;

    std::shared_ptr<BotLogin> self = shared_from_this();
    std::shared_ptr<BotCharSelect> charSelect = BotCharSelect::Create(m_ioContext, result.Connection);
    charSelect->EnterWorld(charOptions, [self](BotCharSelect::Result result) { self->OnCharSelect(std::move(result)); });
}

void BotLogin::OnCharSelect(BotCharSelect::Result result)
{
    if (!result.Success)
    {
        if (result.Connection)
            result.Connection->Close();
        if (result.RealmConnection)
            result.RealmConnection->Close();
        Fail("char select: " + result.Error);
        return;
    }
    if (result.Stopped)
    {
        StopEarly(result.Connection, result.RealmConnection, result.CharacterGuid);
        return;
    }
    Succeed(result.Connection, result.RealmConnection, result.CharacterGuid);
}

void BotLogin::Fail(std::string const& error)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotLogin: %s", error.c_str());
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = false;
        result.Error = error;
        callback(std::move(result));
    }
}

void BotLogin::Succeed(std::shared_ptr<BotWorldConnection> connection, std::shared_ptr<BotWorldConnection> realmConnection, ClassicGuid const& guid)
{
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = true;
        result.Connection = std::move(connection);
        result.RealmConnection = std::move(realmConnection);
        result.CharacterGuid = guid;
        callback(std::move(result));
    }
}

void BotLogin::StopEarly(std::shared_ptr<BotWorldConnection> connection, std::shared_ptr<BotWorldConnection> realmConnection, ClassicGuid const& guid)
{
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = true;
        result.Stopped = true;
        result.Connection = std::move(connection);
        result.RealmConnection = std::move(realmConnection);
        result.CharacterGuid = guid;
        callback(std::move(result));
    }
}
