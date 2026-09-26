#include "BuiltinBotTasks.h"
#include "BotAwaitables.h"
#include "BotContext.h"
#include "BotLogin.h"
#include "BotTaskRegistry.h"
#include "BotWorldLogin.h"
#include "Packets/MovementMessages.h"
#include "RealBot.h"

#include <cctype>
#include <cmath>
#include <string>

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

// A character name for a bot that wasn't given one: letters only, stable per account (digits become
// letters), so a pool never collides with itself.
std::string DefaultCharacterName(std::string const& accountName)
{
    std::string name;
    for (char c : accountName)
    {
        if (name.size() == 12)
            break;
        if (c >= '0' && c <= '9')
            name += char('a' + (c - '0'));
        else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            name += char(std::tolower(static_cast<unsigned char>(c)));
    }
    if (name.size() < 2)
        name += "bot";
    name[0] = char(std::toupper(static_cast<unsigned char>(name[0])));
    return name;
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
    world.CharSelect.CharacterName = opts.CharacterName.empty() ? DefaultCharacterName(opts.AccountName) : opts.CharacterName;
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

    // The enum result's own name wins (it's what the server actually resolved to, which for an
    // existing character need not match a caller-supplied name at all); the requested name is a
    // fallback for when that didn't parse (ClassicEnumCharactersResult's lenient path).
    bot.GetIdentity().CharacterName = outcome.CharacterName.empty() ? opts.CharacterName : outcome.CharacterName;
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

// Walks in a straight line to (x, y, z) at `speed` yards a second: a start message, a heartbeat
// every step, a stop at the destination. The terrain is not consulted; z is interpolated between
// where the bot starts and the given z.
BotRoutine MoveToRoutine(BotContext& ctx, float x, float y, float z, float speed)
{
    RealBot& bot = ctx.GetBot();
    ClassicGuid const guid = ctx.GetIdentity().CharacterGuid;

    std::optional<BotKnownObject> self = bot.GetKnownObject(guid);
    for (uint32 waited = 0; !self && waited < 5000; waited += 100)
    {
        co_await ctx.Delay(100);
        self = bot.GetKnownObject(guid);
    }
    if (!self)
        co_return BotTaskResult::Failed;

    float const startX = self->X;
    float const startY = self->Y;
    float const startZ = self->Z;
    float const distance = std::hypot(x - startX, y - startY);
    float const facing = std::atan2(y - startY, x - startX);

    ClassicMovementInfo status;
    status.Guid = guid;
    uint32 const heldFlags = bot.GetMovementFlags();
    status.Flags = heldFlags | CLASSIC_MOVEFLAG_FORWARD;
    status.X = startX;
    status.Y = startY;
    status.Z = startZ;
    status.O = facing < 0.0f ? facing + 6.2831853f : facing;
    status.Time = ctx.GetTimeMs();

    ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_FORWARD> start;
    start.Status = status;
    ctx.Send(start);
    bot.NoteOwnMovement(status);

    uint32 lastMs = status.Time;
    float walked = 0.0f;
    while (walked < distance)
    {
        co_await ctx.Delay(200);

        uint32 const now = ctx.GetTimeMs();
        walked = std::min(distance, walked + speed * float(now - lastMs) / 1000.0f);
        lastMs = now;

        float const fraction = distance > 0.0f ? walked / distance : 1.0f;
        status.X = startX + (x - startX) * fraction;
        status.Y = startY + (y - startY) * fraction;
        status.Z = startZ + (z - startZ) * fraction;
        status.Time = now;

        if (walked < distance)
        {
            MoveHeartbeat heartbeat;
            heartbeat.Status = status;
            ctx.Send(heartbeat);
        }
        bot.NoteOwnMovement(status);
    }

    status.Flags = heldFlags;
    status.Time = ctx.GetTimeMs();
    MoveStop stop;
    stop.Status = status;
    ctx.Send(stop);
    bot.NoteOwnMovement(status);

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

    sBotTaskRegistry.RegisterRoutine("move_to", [](BotContext& ctx, BotTaskArgs const& args)
    {
        return MoveToRoutine(ctx, args.GetFloat("x"), args.GetFloat("y"), args.GetFloat("z"), args.GetFloat("speed", 6.0f));
    });

    sBotTaskRegistry.RegisterRoutine("disconnect_after", [](BotContext& ctx, BotTaskArgs const& args)
    {
        return DisconnectAfterRoutine(ctx, args.GetUInt32("ms", 0), args.GetBool("clean", true));
    });
}
