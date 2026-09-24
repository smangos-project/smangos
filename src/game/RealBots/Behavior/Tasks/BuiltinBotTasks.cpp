#include "BuiltinBotTasks.h"
#include "BotAwaitables.h"
#include "BotContext.h"
#include "BotLogin.h"
#include "BotTaskRegistry.h"
#include "BotWorldLogin.h"
#include "RealBot.h"

namespace
{
// A failed login step: the reason is kept where "info" and TestBot::GetLastError() read it, and
// the bot is done (Failed closes its connections and drops its tasks).
BotTaskResult FailLogin(BotContext& ctx, std::string const& error)
{
    ctx.GetBot().SetLastError(error);
    ctx.Log("login failed: %s", error.c_str());
    ctx.SetStage(BotStage::Failed);
    return BotTaskResult::Failed;
}

// The default bottom task of a new bot (bot-phase-1.md, "Login"): the whole account-in to
// InWorld-out sequence, then it holds so later tasks (ctx.Run, RealBotMgr's "task" command) run on
// top of it. The Battle.net half runs on the IO threads behind ctx.Login; the world half are
// routines on this bot (Net/BotWorldLogin.h), run one after another with ctx.Run, so their packets
// go through the bot's normal dispatch, hooks included. RealBotMgr::Add is what fills in
// RealBot::GetLoginOptions() before this task starts. Options::StopAt (bot-phase-1.md's
// "stop_at=<stage>") holds the routine at that stage instead of continuing.
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
    options.ConnectTimeoutMs = opts.ConnectTimeoutMs;
    options.LoginTicketOverride = opts.LoginTicketOverride;
    options.StopAt = opts.StopAt;

    BotLogin::Result login = co_await ctx.Login(options);
    if (!login.Success)
        co_return FailLogin(ctx, login.Error);

    bot.GetIdentity().AccountName = opts.AccountName;
    bot.GetIdentity().Session.LoginTicket = login.LoginTicket;
    bot.GetIdentity().Session.WorldAddress = login.WorldAddress;
    bot.GetIdentity().Session.WorldPort = login.WorldPort;
    bot.GetIdentity().Session.JoinSessionKey = login.SessionKey;
    bot.GetIdentity().Session.RealmJoinTicket = login.RealmJoinTicket;
    bot.GetIdentity().Session.RealmId = opts.RealmId;
    if (login.Stopped)
    {
        ctx.SetStage(opts.StopAt);
        bot.SetHeld();
        co_return co_await ctx.Hold();
    }

    BotWorldLoginParams world;
    world.Handshake.WorldAddress = login.WorldAddress;
    world.Handshake.WorldPort = login.WorldPort;
    world.Handshake.JoinSessionKey = login.SessionKey;
    world.Handshake.RealmJoinTicket = login.RealmJoinTicket;
    world.Handshake.RealmId = opts.RealmId;
    world.Handshake.VerifyServerSignature = opts.VerifyServerSignature;
    world.Handshake.ConnectTimeoutMs = opts.ConnectTimeoutMs;
    world.Handshake.PacketTimeoutMs = opts.PacketTimeoutMs;
    world.Handshake.StopAt = opts.StopAt;
    world.CharSelect.CreateIfNone = opts.CreateIfNone;
    world.CharSelect.CharacterName = opts.CharacterName;
    world.CharSelect.Race = opts.Race;
    world.CharSelect.Class = opts.Class;
    world.CharSelect.Gender = opts.Gender;
    world.CharSelect.FarClip = opts.FarClip;
    world.CharSelect.VerifyServerSignature = opts.VerifyServerSignature;
    world.CharSelect.ConnectTimeoutMs = opts.ConnectTimeoutMs;
    world.CharSelect.PacketTimeoutMs = opts.PacketTimeoutMs;
    world.CharSelect.StopAt = opts.StopAt;

    BotWorldLoginOutcome outcome;
    co_await ctx.Run(BotWorldLoginRoutine(ctx, std::move(world), outcome));
    if (!outcome.Success)
        co_return FailLogin(ctx, outcome.Error);

    if (!opts.CharacterName.empty())
        bot.GetIdentity().CharacterName = opts.CharacterName;
    bot.GetIdentity().CharacterGuid = outcome.CharacterGuid;

    // Stopped is only ever true when it matches opts.StopAt exactly (nothing stops on its own).
    ctx.SetStage(outcome.Stopped ? opts.StopAt : BotStage::InWorld);
    if (outcome.Stopped)
        bot.SetHeld();

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
