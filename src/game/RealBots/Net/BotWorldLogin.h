#ifndef REALBOTS_BOTWORLDLOGIN_H
#define REALBOTS_BOTWORLDLOGIN_H

#include "BotRoutine.h"
#include "BotStage.h"
#include "Packets/Auth.h"
#include "Packets/Classic/Guid.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

class BotContext;

// The world half of bot-phase-1.md's "Login", as BotRoutines driven through BotContext
// (bot-phase-2.md, "Handshake through the normal dispatch"): WorldConnecting/WorldHandshake, then
// CharSelect/EnteringWorld with the SMSG_CONNECT_TO redirect to the Instance connection. They run
// on the bot's own thread like any other routine, so every packet, from SMSG_AUTH_CHALLENGE on,
// goes through the bot's inbox, its hooks and the packet log the same way an in-world packet does.
// Their crypto stays in WorldHandshakeCrypto; only the sequencing lives here.
//
// A bot's stage is the step in progress, and StopAt(X) holds the login as X begins, before any of
// X's own work (bot-phase-2.md: "StopAt(CharSelect)" is how a scenario looks at a completed
// handshake): the routines below set each stage as they enter it and stop there if asked.
//
// Each routine reports through an outcome struct owned by whoever ctx.Run()s it (the built-in
// "login" routine), never an exception: a failure sets Error and returns BotTaskResult::Failed,
// and the caller decides what stage and cleanup that means. The outcome must outlive the routine.
// Every wait in here is bounded by PacketTimeoutMs, so a server that goes silent fails the login
// instead of hanging it.

// Connect (Realm), V2 banner, SMSG_AUTH_CHALLENGE -> CMSG_AUTH_SESSION -> SMSG_ENTER_ENCRYPTED_MODE
// (signature optionally verified) -> CMSG_ENTER_ENCRYPTED_MODE_ACK, encryption armed as it goes out
// -> SMSG_AUTH_RESPONSE. Sets WorldConnecting while it connects and WorldHandshake once the socket
// is up (the server's SMSG_AUTH_CHALLENGE follows on its own). Takes what BotBnetLogin hands back.
// StopAt(WorldHandshake) holds there, before the challenge is answered; a successful
// SMSG_AUTH_RESPONSE is what the CharSelect stage, entered by the next routine, begins with.
struct BotWorldHandshakeParams
{
    std::string WorldAddress;
    uint16_t WorldPort = 0;
    std::vector<uint8> JoinSessionKey; // BotBnetLogin::Result::SessionKey
    std::string RealmJoinTicket;
    uint32 RealmId = 0;
    // Not validated by the server; sent for wire-shape parity.
    uint32 RegionId = 1;
    uint32 BattlegroupId = 1;
    bool VerifyServerSignature = true;
    uint32 ConnectTimeoutMs = 10000;
    uint32 PacketTimeoutMs = 10000;
    BotStage StopAt = BotStage::InWorld;
};

struct BotWorldHandshakeOutcome
{
    bool Success = false;
    bool Stopped = false; // StopAt(WorldHandshake) held the routine before the handshake ran
    uint32 AuthResult = 0; // BattlenetRpcErrorCode from SMSG_AUTH_RESPONSE; 0 == Ok
    // The 40-byte world session key CMSG_AUTH_SESSION established; the Instance connection's
    // CMSG_AUTH_CONTINUED_SESSION reuses it rather than doing a fresh key exchange.
    std::array<uint8, 40> SessionKey{};
    std::string Error;
};

BotRoutine BotWorldHandshakeRoutine(BotContext& ctx, BotWorldHandshakeParams params, BotWorldHandshakeOutcome& outcome);

// CMSG_ENUM_CHARACTERS -> the first character (or one created, when CreateIfNone) ->
// CMSG_PLAYER_LOGIN -> SMSG_CONNECT_TO, which redirects to an Instance connection
// (BotInstanceConnectRoutine) -> SMSG_LOGIN_VERIFY_WORLD there (bot-phase-1.md's InWorld marker).
// Sets CharSelect on entry (the authenticated, encrypted Realm connection the handshake left
// attached, nothing asked of it yet: StopAt(CharSelect) holds here) and EnteringWorld once a
// character is resolved (StopAt(EnteringWorld) holds here, before CMSG_PLAYER_LOGIN is sent).
struct BotCharSelectParams
{
    bool CreateIfNone = true;
    // Only used if a character actually needs creating; must be set (and unique on the target)
    // whenever CreateIfNone might trigger.
    std::string CharacterName;
    uint8 Race = 0;
    uint8 Class = 0;
    uint8 Gender = 0;
    float FarClip = 533.0f;
    // Needed only if the server redirects to an Instance connection (BotInstanceConnectRoutine's).
    std::array<uint8, 40> SessionKey{};
    bool VerifyServerSignature = true;
    uint32 ConnectTimeoutMs = 10000;
    uint32 PacketTimeoutMs = 10000;
    BotStage StopAt = BotStage::InWorld;
};

struct BotCharSelectOutcome
{
    bool Success = false;
    bool Stopped = false; // held at StopAt; CharacterGuid is only set if the character was resolved by then
    ClassicGuid CharacterGuid;
    std::string Error;
};

BotRoutine BotCharSelectRoutine(BotContext& ctx, BotCharSelectParams params, BotCharSelectOutcome& outcome);

// The second ("Instance") world connection SMSG_CONNECT_TO redirects to: connect, V2 banner,
// SMSG_AUTH_CHALLENGE -> CMSG_AUTH_CONTINUED_SESSION (reusing the Realm connection's session key)
// -> SMSG_ENTER_ENCRYPTED_MODE -> the ack, encryption armed -> SMSG_RESUME_COMMS (which replaces
// SMSG_AUTH_RESPONSE here; that only ever goes out on the Realm connection). The redirect is
// unconditional: every login goes through it.
struct BotInstanceConnectParams
{
    std::string WorldAddress;
    uint16_t WorldPort = 0;
    std::array<uint8, 40> SessionKey{}; // BotWorldHandshakeOutcome::SessionKey
    uint64 PackedKey = 0; // ConnectTo::Key
    bool VerifyServerSignature = true;
    uint32 ConnectTimeoutMs = 10000;
    uint32 PacketTimeoutMs = 10000;
};

struct BotInstanceConnectOutcome
{
    bool Success = false;
    std::string Error;
};

BotRoutine BotInstanceConnectRoutine(BotContext& ctx, BotInstanceConnectParams params, BotInstanceConnectOutcome& outcome);

// The whole world half in one: BotWorldHandshakeRoutine, then BotCharSelectRoutine with the session
// key the first one established. What the built-in "login" routine runs after the Battle.net half,
// and what tests drive directly against a fake world server. Failures carry which step they came
// from ("world handshake: ...", "char select: ...").
struct BotWorldLoginParams
{
    BotWorldHandshakeParams Handshake;
    BotCharSelectParams CharSelect; // SessionKey is filled in by the routine
};

struct BotWorldLoginOutcome
{
    bool Success = false;
    // One of the steps held at its StopAt stage (Handshake.StopAt/CharSelect.StopAt) instead of
    // finishing the login; the stage the bot is at then is that StopAt.
    bool Stopped = false;
    ClassicGuid CharacterGuid;
    std::string Error;
};

BotRoutine BotWorldLoginRoutine(BotContext& ctx, BotWorldLoginParams params, BotWorldLoginOutcome& outcome);

#endif
