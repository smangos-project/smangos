// The Battle.net login server: the REST login portal and the RPC port the modern client uses to log in
// and pick a realm.

#include "BnetServer.h"

#include "Common.h"
#include "Database/DatabaseEnv.h"

#include "Auth/LoginThrottle.h"
#include "BnetListener.h"
#include "BnetLoginPortal.h"
#include "BnetLoginTickets.h"
#include "Bnet/BnetRealmList.h"
#include "BnetRestSession.h"
#include "BnetRpcSession.h"
#include "BnetSslContext.h"
#include "Config/Config.h"
#include "IO/Multithreading/CreateThread.h"
#include "Log.h"
#include "Util.h"

#include <openssl/ssl.h>

#include <atomic>

static SSL_CTX* s_sslCtx = nullptr;
static std::unique_ptr<IO::IoContext> s_ioCtx;
static std::unique_ptr<IO::Networking::AsyncSocketAcceptor> s_restListener;
static std::unique_ptr<IO::Networking::AsyncSocketAcceptor> s_rpcListener;
static std::thread s_ioThread;
static std::thread s_housekeepingThread;
static std::atomic<bool> s_stopHousekeeping{ false };

static void RunHousekeeping()
{
    constexpr uint32 kCleanupIntervalSecs = 60;
    uint32 const numLoops = sConfig.GetIntDefault("MaxPingTime", 30) * 60;
    uint32 loopCounter = 0;
    uint32 cleanupCounter = 0;

    while (!s_stopHousekeeping)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (++cleanupCounter >= kCleanupIntervalSecs)
        {
            cleanupCounter = 0;
            CleanupStaleLoginThrottles();
            sBnetLoginTickets.CleanupExpired();
            BnetLoginPortal::CleanupSessions();
        }

        if ((++loopCounter) == numLoops)
        {
            loopCounter = 0;
            sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "Ping MySQL to keep connection alive");
            LoginDatabase.Ping();
        }
    }
}

bool BnetServerStart()
{
    if (sBnetRealmList.GetRealms().empty())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "No valid realms specified.");
        return false;
    }

    uint16 const restPort = uint16(sConfig.GetIntDefault("LoginREST.Port", 8081));
    uint16 const rpcPort = uint16(sConfig.GetIntDefault("BattlenetPort", 1119));

    BnetPortalSettings portalSettings;
    portalSettings.externalHostname = sConfig.GetStringDefault("LoginREST.ExternalAddress", "127.0.0.1");
    portalSettings.localHostname = sConfig.GetStringDefault("LoginREST.LocalAddress", "127.0.0.1");
    portalSettings.restPort = restPort;
    portalSettings.rpcPort = rpcPort;
    BnetLoginPortal::Configure(portalSettings);
    sBnetLoginTickets.SetDuration(sConfig.GetIntDefault("LoginREST.TicketDuration", 3600));

    s_sslCtx = BnetCreateServerSslContext(
        sConfig.GetStringDefault("CertificatesFile", "config/bnetserver.cert.pem"),
        sConfig.GetStringDefault("PrivateKeyFile", "config/bnetserver.key.pem"));
    if (!s_sslCtx)
        return false;

    LoginDatabase.BeginTransaction();
    LoginDatabase.Execute("UPDATE `account_banned` SET `active` = 0 WHERE `unbandate`<=UNIX_TIMESTAMP() AND `unbandate`<>`bandate`");
    LoginDatabase.Execute("UPDATE `battlenet_account_bans` SET `active` = 0 WHERE `unbandate`<=UNIX_TIMESTAMP() AND `unbandate`<>`bandate`");
    LoginDatabase.Execute("DELETE FROM `ip_banned` WHERE `unbandate`<=UNIX_TIMESTAMP() AND `unbandate`<>`bandate`");
    LoginDatabase.CommitTransaction();

    std::string const bindIp = sConfig.GetStringDefault("Bnet.BindIP", "0.0.0.0");
    std::vector<std::string> const trustedProxyIps = SplitStringByDelimiter(sConfig.GetStringDefault("TrustedProxyServers", ""), ',');

    s_ioCtx = IO::IoContext::CreateIoContext();
    if (s_ioCtx == nullptr)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Failed to create IoContext");
        BnetServerStop();
        return false;
    }

    SSL_CTX* sslCtx = s_sslCtx;
    s_restListener = BnetStartListener(s_ioCtx.get(), bindIp, restPort, trustedProxyIps,
        [sslCtx](std::unique_ptr<IO::Networking::AsyncSocket> socket, std::string clientIp)
        {
            BnetRestSession::Create(std::move(socket), sslCtx, std::move(clientIp))->Start();
        });
    if (!s_restListener)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "bnetserver cannot bind the login portal to %s:%u - is the port already in use?", bindIp.c_str(), uint32(restPort));
        BnetServerStop();
        return false;
    }

    s_rpcListener = BnetStartListener(s_ioCtx.get(), bindIp, rpcPort, trustedProxyIps,
        [sslCtx](std::unique_ptr<IO::Networking::AsyncSocket> socket, std::string clientIp)
        {
            BnetRpcSession::Create(std::move(socket), sslCtx, std::move(clientIp))->Start();
        });
    if (!s_rpcListener)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "bnetserver cannot bind the RPC port to %s:%u - is the port already in use?", bindIp.c_str(), uint32(rpcPort));
        BnetServerStop();
        return false;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Login portal on %s:%u, RPC on %s:%u", bindIp.c_str(), uint32(restPort), bindIp.c_str(), uint32(rpcPort));

    s_ioThread = IO::Multithreading::CreateThread("MainIoCtx", []()
    {
        s_ioCtx->RunUntilShutdown();
    });
    s_housekeepingThread = IO::Multithreading::CreateThread("BnetHousekeeping", RunHousekeeping);
    return true;
}

void BnetServerStop()
{
    s_stopHousekeeping = true;
    if (s_housekeepingThread.joinable())
        s_housekeepingThread.join();

    if (s_restListener)
        s_restListener->ClosePortAndStopAcceptingNewConnections();
    if (s_rpcListener)
        s_rpcListener->ClosePortAndStopAcceptingNewConnections();

    if (s_ioCtx)
        s_ioCtx->Shutdown();
    if (s_ioThread.joinable())
        s_ioThread.join();

    s_restListener.reset();
    s_rpcListener.reset();
    s_ioCtx.reset();

    if (s_sslCtx)
    {
        SSL_CTX_free(s_sslCtx);
        s_sslCtx = nullptr;
    }
}
