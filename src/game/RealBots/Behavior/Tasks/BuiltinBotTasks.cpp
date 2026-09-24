#include "BuiltinBotTasks.h"
#include "BotAwaitables.h"
#include "BotContext.h"
#include "BotLogin.h"
#include "BotTaskRegistry.h"
#include "RealBot.h"

namespace
{
// The default bottom task of a new bot (bot-phase-1.md, "Login"): drives BotLogin::Login through
// the whole account-in to InWorld-out chain via ctx.Login, then holds so later tasks (ctx.Run,
// RealBotMgr's "task" command) run on top of it. RealBotMgr::Add is what fills in
// RealBot::GetLoginOptions() before this task starts.
BotRoutine LoginRoutine(BotContext& ctx)
{
    RealBot& bot = ctx.GetBot();
    RealBotLoginOptions const& opts = bot.GetLoginOptions();

    ctx.SetStage(BotStage::LoginPortal);

    BotLogin::Options options;
    options.LoginAddress = opts.LoginAddress;
    options.RestPort = opts.RestPort;
    options.RpcPort = opts.RpcPort;
    options.AccountName = opts.AccountName;
    options.Password = opts.Password;
    options.RealmName = opts.RealmName;
    options.RealmId = opts.RealmId;
    options.ClientBuild = opts.ClientBuild;
    options.VerifyServerSignature = opts.VerifyServerSignature;
    options.CreateIfNone = opts.CreateIfNone;
    options.CharacterName = opts.CharacterName;
    options.Race = opts.Race;
    options.Class = opts.Class;
    options.Gender = opts.Gender;
    options.FarClip = opts.FarClip;
    options.StopAt = opts.StopAt;

    BotLogin::Result result = co_await ctx.Login(options);
    if (!result.Success)
    {
        bot.SetLastError(result.Error);
        ctx.Log("login failed: %s", result.Error.c_str());
        ctx.SetStage(BotStage::Failed);
        co_return BotTaskResult::Failed;
    }

    if (result.Connection || result.RealmConnection)
        bot.AttachConnections(result.Connection, result.RealmConnection);
    bot.GetIdentity().AccountName = opts.AccountName;
    if (!opts.CharacterName.empty())
        bot.GetIdentity().CharacterName = opts.CharacterName;
    bot.GetIdentity().CharacterGuid = result.CharacterGuid;

    // Options::StopAt (bot-phase-1.md's "stop_at=<stage>") holds the routine wherever BotLogin
    // stopped instead of treating it as InWorld -- result.Stopped is only ever true when it
    // matches opts.StopAt exactly (BotLogin never stops on its own).
    ctx.SetStage(result.Stopped ? opts.StopAt : BotStage::InWorld);

    co_return co_await ctx.Hold();
}

// Does nothing beyond holding -- a placeholder top task for a bot that should just sit InWorld.
BotRoutine IdleRoutine(BotContext& ctx)
{
    co_return co_await ctx.Hold();
}

// Waits delayMs, then disconnects (clean or not) -- mainly for exercising Disconnect/logout by
// hand from the runner/.realbot console ("task <bot> disconnect_after ms=5000").
BotRoutine DisconnectAfterRoutine(BotContext& ctx, uint32 delayMs, bool clean)
{
    co_await ctx.Delay(delayMs);
    ctx.Disconnect(clean);
    co_return BotTaskResult::Done;
}
}

void RegisterBuiltinBotTasks()
{
    sBotTaskRegistry.RegisterRoutine("login", [](BotContext& ctx, BotTaskArgs const&)
    {
        return LoginRoutine(ctx);
    });

    sBotTaskRegistry.RegisterRoutine("idle", [](BotContext& ctx, BotTaskArgs const&)
    {
        return IdleRoutine(ctx);
    });

    sBotTaskRegistry.RegisterRoutine("disconnect_after", [](BotContext& ctx, BotTaskArgs const& args)
    {
        return DisconnectAfterRoutine(ctx, args.GetUInt32("ms", 0), args.GetBool("clean", true));
    });
}
