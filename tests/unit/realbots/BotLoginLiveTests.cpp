// One-off verification of BotLogin against a real, running vibemangos target (bot-phase-1.md
// step 5's "first contact"), not the fakes BotLoginTests.cpp/BotBnetLoginTests.cpp/etc. use.
// Tagged [live], deliberately excluded from discovery by tests/unit/realbots/CMakeLists.txt's
// TEST_SPEC ("~[live]") -- `ctest -L unit` must stay hermetic (bot-plan.md's testing model puts
// real-network scenarios in the conformance suite, not built yet). Run by hand:
//   tests-realbots.exe "[live]" --success
// against a vibemangos instance already listening on the addresses/ports below.
#include "BotLogin.h"
#include "Crypto/Authentication/BnetSRP6.h"
#include "IO/Context/IoContext.h"
#include "Util.h"
#include <catch_amalgamated.hpp>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace
{
class WaitFlag
{
    public:
        void Set()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_set = true;
            m_cv.notify_all();
        }

        bool WaitFor(std::chrono::milliseconds timeout)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cv.wait_for(lock, timeout, [this]() { return m_set; });
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        bool m_set = false;
};

std::vector<uint8> HexDecode(std::string const& hex)
{
    std::vector<uint8> out(hex.size() / 2);
    HexStrToByteArray(hex, out.data());
    return out;
}
}

// No network at all -- checks a candidate password against the account's *stored* salt/verifier
// (queried by hand via the mysql CLI: `SELECT salt, verifier FROM battlenet_accounts WHERE
// email='LOL'`) using BnetSRP6::CheckCredentials, the same math the server itself uses. Useful
// for ruling "wrong password" in or out before spending a real network round trip on it -- this
// is exactly how BotRestClient's missing password-case-normalization bug (fixed this session,
// see its own header comment) was found: a lowercase-typed password that works against the real
// game client only matched this account's stored verifier once uppercased first, matching
// vibemangos's AccountMgr::CreateAccount normalizing both username and password before computing
// it. Update the salt/verifier hex below if the account's password is ever changed.
TEST_CASE("BotLogin (live): check a password against the account's stored SRP verifier", "[live]")
{
    std::string const srpUsername = BnetSRP6::ComputeSrpUsername("LOL");
    std::vector<uint8> const salt = HexDecode("E872F64B9C2A726F321D2FDDD84F71D77CBD6E26DB8D365567D3DC57594F90DB");
    std::vector<uint8> const verifier = HexDecode(
        "2FDA0F80070E91E4FBB0E0E40472CE032C78E29473971A61A9FF1C5A10B8A66F0EAD0894A0689E51DEAEC0B07082CE492D685620D52EB100DFB8A35667E877F"
        "3050141874F71B23093AFB2BED7E04B030B83D729B84B66F77B61828669BCDA6C7A6A4FBDE8A8A3F6045780817E6A3236B48073F6C0605179B434AED29CEC6C52");

    BnetSRP6 srp;
    srp.Initialize(salt, verifier);
    CHECK(srp.CheckCredentials(srpUsername, "LOL"));
}

TEST_CASE("BotLogin: reaches InWorld against a live vibemangos target", "[live]")
{
    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    BotLogin::Options options;
    options.LoginAddress = "127.0.0.1";
    options.RestPort = 8081;
    options.RpcPort = 1119;
    options.AccountName = "lol";
    options.Password = "lol";
    options.RealmName = "VibeMangos";
    options.ClientBuild = 58797;
    options.CreateIfNone = true;
    options.CharacterName = "Realbot1";
    options.Race = 1; // RACE_HUMAN
    options.Class = 1; // CLASS_WARRIOR

    std::shared_ptr<BotLogin> login = BotLogin::Create(ctx.get());

    WaitFlag done;
    BotLogin::Result result;
    login->Login(options, [&](BotLogin::Result r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(30)));
    INFO("error: " << result.Error);
    CHECK(result.Success);
    if (result.Success)
    {
        CHECK(result.Connection != nullptr);
        WARN("InWorld as character guid low=" << result.CharacterGuid.Low << " high=" << result.CharacterGuid.High);
    }

    if (result.Connection)
    {
        result.Connection->Close();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    ctx->Shutdown();
    ioThread.join();
}
