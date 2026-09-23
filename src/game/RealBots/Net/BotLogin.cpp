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

    BotBnetLogin::Options bnetOptions;
    bnetOptions.LoginAddress = m_options.LoginAddress;
    bnetOptions.RestPort = m_options.RestPort;
    bnetOptions.RpcPort = m_options.RpcPort;
    bnetOptions.AccountName = m_options.AccountName;
    bnetOptions.Password = m_options.Password;
    bnetOptions.RealmName = m_options.RealmName;
    bnetOptions.RealmId = m_options.RealmId;
    bnetOptions.ClientBuild = m_options.ClientBuild;

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

    BotCharSelect::Options charOptions;
    charOptions.CreateIfNone = m_options.CreateIfNone;
    charOptions.CharacterName = m_options.CharacterName;
    charOptions.Race = m_options.Race;
    charOptions.Class = m_options.Class;
    charOptions.Gender = m_options.Gender;
    charOptions.FarClip = m_options.FarClip;

    std::shared_ptr<BotLogin> self = shared_from_this();
    std::shared_ptr<BotWorldConnection> connection = result.Connection;
    std::shared_ptr<BotCharSelect> charSelect = BotCharSelect::Create(connection);
    charSelect->EnterWorld(charOptions, [self, connection](BotCharSelect::Result result) { self->OnCharSelect(connection, std::move(result)); });
}

void BotLogin::OnCharSelect(std::shared_ptr<BotWorldConnection> connection, BotCharSelect::Result result)
{
    if (!result.Success)
    {
        connection->Close();
        Fail("char select: " + result.Error);
        return;
    }
    Succeed(connection, result.CharacterGuid);
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

void BotLogin::Succeed(std::shared_ptr<BotWorldConnection> connection, ClassicGuid const& guid)
{
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = true;
        result.Connection = std::move(connection);
        result.CharacterGuid = guid;
        callback(std::move(result));
    }
}
