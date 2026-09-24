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
        Result result;
        result.Success = true;
        result.Stopped = true;
        Finish(std::move(result));
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

void BotLogin::OnBnetLogin(BotBnetLogin::Result bnet)
{
    if (!bnet.Success)
    {
        Fail("bnet login: " + bnet.Error);
        return;
    }

    Result result;
    result.Success = true;
    result.Stopped = bnet.Stopped || m_options.StopAt == BotStage::WorldConnecting;
    result.WorldAddress = std::move(bnet.WorldAddress);
    result.WorldPort = bnet.WorldPort;
    result.SessionKey = std::move(bnet.SessionKey);
    result.RealmJoinTicket = std::move(bnet.RealmJoinTicket);
    Finish(std::move(result));
}

void BotLogin::Fail(std::string const& error)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotLogin: %s", error.c_str());

    Result result;
    result.Success = false;
    result.Error = error;
    Finish(std::move(result));
}

void BotLogin::Finish(Result result)
{
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        callback(std::move(result));
    }
}
