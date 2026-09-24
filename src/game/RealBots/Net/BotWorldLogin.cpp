#include "BotWorldLogin.h"
#include "BotAwaitables.h"
#include "BotContext.h"
#include "ClassicOpcodes.h"
#include "Crypto/Authentication/WorldHandshakeCrypto.h"
#include "Crypto/BigNumber.h"
#include "Crypto/Ed25519.h"
#include "Log.h"
#include <algorithm>
#include <cstdio>

namespace
{
// Records why a routine failed, and logs it once at the point of failure the way the callback-based
// steps this replaced did.
BotTaskResult Fail(char const* who, std::string& error, std::string message)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "%s: %s", who, message.c_str());
    error = std::move(message);
    return BotTaskResult::Failed;
}

// Why a wait for `what` came back empty: it is either the timeout or, for a packet wait, the
// connection closing under it (BotContext's awaiters resolve empty on both).
std::string NoPacket(char const* what)
{
    return std::string("timed out waiting for ") + what + " (or the connection closed)";
}

// How many times the Instance connection is tried before the login is abandoned (vibemangos:
// WorldSession::HandleConnectToFailedOpcode, TrinityCore: WorldSocket::HandleConnectToFailed).
constexpr uint32 MaxInstanceConnectAttempts = 5;

std::array<uint8, 32> RandomChallenge()
{
    BigNumber challengeBn;
    challengeBn.SetRand(32 * 8);
    std::vector<uint8> bytes = challengeBn.AsByteArray(32, false);

    std::array<uint8, 32> challenge{};
    std::copy(bytes.begin(), bytes.end(), challenge.begin());
    return challenge;
}

// The server signs SMSG_ENTER_ENCRYPTED_MODE under a fixed key both reference implementations
// hardcode (WorldHandshakeCrypto::EnterEncryptedModePrivateKey); returns whether the signature on
// `message` is the one this session's encrypt key calls for.
bool VerifyEnterEncryptedModeSignature(std::array<uint8, 32> const& encryptKey, ClassicEnterEncryptedMode const& message, std::string& error)
{
    std::array<uint8, 64> toSign = WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(encryptKey, message.Enabled);

    Ed25519 ed25519;
    if (!ed25519.LoadFromByteArray(WorldHandshakeCrypto::EnterEncryptedModePrivateKey()))
    {
        error = "failed to load Ed25519 key";
        return false;
    }

    std::vector<uint8> context(WorldHandshakeCrypto::EnableEncryptionContext().begin(), WorldHandshakeCrypto::EnableEncryptionContext().end());
    if (!ed25519.VerifyWithContext(toSign, message.Signature, context))
    {
        error = "SMSG_ENTER_ENCRYPTED_MODE signature verification failed";
        return false;
    }
    return true;
}
}

BotRoutine BotWorldHandshakeRoutine(BotContext& ctx, BotWorldHandshakeParams params, BotWorldHandshakeOutcome& outcome)
{
    char const* const who = "BotWorldHandshake";

    ctx.SetStage(BotStage::WorldConnecting);
    BotConnectResult connected = co_await ctx.Connect(BotConnectionKind::Realm, params.WorldAddress, params.WorldPort, params.ConnectTimeoutMs);
    if (!connected.Success)
        co_return Fail(who, outcome.Error, "world " + connected.Error);

    ctx.SetStage(BotStage::WorldHandshake);
    if (params.StopAt == BotStage::WorldHandshake)
    {
        // The socket is up and the server's banner exchange done; nothing of the handshake itself
        // has been answered. SMSG_AUTH_CHALLENGE, when it arrives, is only seen by hooks.
        outcome.Success = true;
        outcome.Stopped = true;
        co_return BotTaskResult::Done;
    }

    std::optional<ClassicAuthChallenge> challenge = co_await ctx.Packet<ClassicAuthChallenge>(params.PacketTimeoutMs);
    if (!challenge)
        co_return Fail(who, outcome.Error, NoPacket("SMSG_AUTH_CHALLENGE"));

    std::array<uint8, 32> const localChallenge = RandomChallenge();
    std::array<uint8, 32> const& serverChallenge = challenge->ServerChallenge;

    std::array<uint8, 40> const sessionKey = WorldHandshakeCrypto::ComputeSessionKey(params.JoinSessionKey, localChallenge, serverChallenge);
    std::array<uint8, 32> const encryptKey = WorldHandshakeCrypto::ComputeEncryptKey(sessionKey, localChallenge, serverChallenge);

    ClassicAuthSession session;
    session.RegionId = params.RegionId;
    session.BattlegroupId = params.BattlegroupId;
    session.RealmId = params.RealmId;
    session.LocalChallenge = localChallenge;
    session.Digest = WorldHandshakeCrypto::ComputeAuthCheckDigest(params.JoinSessionKey, localChallenge, serverChallenge);
    session.RealmJoinTicket = params.RealmJoinTicket;
    ctx.Send(session, BotConnectionKind::Realm);

    std::optional<ClassicEnterEncryptedMode> enterEncryptedMode = co_await ctx.Packet<ClassicEnterEncryptedMode>(params.PacketTimeoutMs);
    if (!enterEncryptedMode)
        co_return Fail(who, outcome.Error, NoPacket("SMSG_ENTER_ENCRYPTED_MODE"));

    if (params.VerifyServerSignature)
    {
        std::string signatureError;
        if (!VerifyEnterEncryptedModeSignature(encryptKey, *enterEncryptedMode, signatureError))
            co_return Fail(who, outcome.Error, std::move(signatureError));
    }

    // The ack goes out unencrypted; encryption is armed as it is sent (only if it is: a hook that
    // drops it leaves the connection in plain mode, and the server waits forever for it).
    if (enterEncryptedMode->Enabled)
        ctx.SendAndArmEncryption(ClassicEnterEncryptedModeAck{}, BotConnectionKind::Realm, encryptKey);
    else
        ctx.Send(ClassicEnterEncryptedModeAck{}, BotConnectionKind::Realm);

    std::optional<ClassicAuthResponse> response = co_await ctx.Packet<ClassicAuthResponse>(params.PacketTimeoutMs);
    if (!response)
        co_return Fail(who, outcome.Error, NoPacket("SMSG_AUTH_RESPONSE"));

    outcome.AuthResult = response->Result;
    if (response->Result != 0)
        co_return Fail(who, outcome.Error, "SMSG_AUTH_RESPONSE result " + std::to_string(response->Result));

    outcome.SessionKey = sessionKey;
    outcome.Success = true;
    co_return BotTaskResult::Done;
}

BotRoutine BotInstanceConnectRoutine(BotContext& ctx, BotInstanceConnectParams params, BotInstanceConnectOutcome& outcome)
{
    char const* const who = "BotInstanceConnect";

    BotConnectResult connected = co_await ctx.Connect(BotConnectionKind::Instance, params.WorldAddress, params.WorldPort, params.ConnectTimeoutMs);
    if (!connected.Success)
        co_return Fail(who, outcome.Error, "instance " + connected.Error);

    std::optional<ClassicAuthChallenge> challenge = co_await ctx.Packet<ClassicAuthChallenge>(params.PacketTimeoutMs);
    if (!challenge)
        co_return Fail(who, outcome.Error, NoPacket("SMSG_AUTH_CHALLENGE"));

    std::array<uint8, 32> const localChallenge = RandomChallenge();
    std::array<uint8, 32> const& serverChallenge = challenge->ServerChallenge;

    std::array<uint8, 32> const encryptKey = WorldHandshakeCrypto::ComputeEncryptKey(params.SessionKey, localChallenge, serverChallenge);

    ClassicAuthContinuedSession session;
    session.DosResponse = 0;
    session.Key = params.PackedKey;
    session.LocalChallenge = localChallenge;
    session.Digest = WorldHandshakeCrypto::ComputeContinuedSessionDigest(params.SessionKey, params.PackedKey, localChallenge, serverChallenge);
    ctx.Send(session, BotConnectionKind::Instance);

    std::optional<ClassicEnterEncryptedMode> enterEncryptedMode = co_await ctx.Packet<ClassicEnterEncryptedMode>(params.PacketTimeoutMs);
    if (!enterEncryptedMode)
        co_return Fail(who, outcome.Error, NoPacket("SMSG_ENTER_ENCRYPTED_MODE"));

    if (params.VerifyServerSignature)
    {
        std::string signatureError;
        if (!VerifyEnterEncryptedModeSignature(encryptKey, *enterEncryptedMode, signatureError))
            co_return Fail(who, outcome.Error, std::move(signatureError));
    }

    if (enterEncryptedMode->Enabled)
        ctx.SendAndArmEncryption(ClassicEnterEncryptedModeAck{}, BotConnectionKind::Instance, encryptKey);
    else
        ctx.Send(ClassicEnterEncryptedModeAck{}, BotConnectionKind::Instance);

    std::optional<ClassicResumeComms> resume = co_await ctx.Packet<ClassicResumeComms>(params.PacketTimeoutMs);
    if (!resume)
        co_return Fail(who, outcome.Error, NoPacket("SMSG_RESUME_COMMS"));

    outcome.Success = true;
    co_return BotTaskResult::Done;
}

BotRoutine BotCharSelectRoutine(BotContext& ctx, BotCharSelectParams params, BotCharSelectOutcome& outcome)
{
    char const* const who = "BotCharSelect";

    ctx.SetStage(BotStage::CharSelect);
    if (params.StopAt == BotStage::CharSelect)
    {
        // Authenticated and encrypted, the character screen not yet asked for anything.
        outcome.Success = true;
        outcome.Stopped = true;
        co_return BotTaskResult::Done;
    }

    ctx.Send(ClassicEnumCharacters{}, BotConnectionKind::Realm);

    std::optional<ClassicEnumCharactersResult> characters = co_await ctx.Packet<ClassicEnumCharactersResult>(params.PacketTimeoutMs);
    if (!characters)
        co_return Fail(who, outcome.Error, NoPacket("SMSG_ENUM_CHARACTERS_RESULT"));

    if (characters->CharacterCount > 0)
    {
        outcome.CharacterGuid = characters->FirstCharacterGuid;
    }
    else
    {
        if (!params.CreateIfNone)
            co_return Fail(who, outcome.Error, "no characters and CreateIfNone is false");

        ClassicCreateCharacter create;
        create.Name = params.CharacterName;
        create.Race = params.Race;
        create.Class = params.Class;
        create.Gender = params.Gender;
        ctx.Send(create, BotConnectionKind::Realm);

        std::optional<ClassicCreateCharResponse> created = co_await ctx.Packet<ClassicCreateCharResponse>(params.PacketTimeoutMs);
        if (!created)
            co_return Fail(who, outcome.Error, NoPacket("SMSG_CREATE_CHAR"));
        if (created->Result != CLASSIC_CHAR_CREATE_SUCCESS)
            co_return Fail(who, outcome.Error, "character creation failed, result " + std::to_string(created->Result));

        outcome.CharacterGuid = created->Guid;
    }

    ctx.SetStage(BotStage::EnteringWorld);
    if (params.StopAt == BotStage::EnteringWorld)
    {
        // A character is resolved (outcome.CharacterGuid) but CMSG_PLAYER_LOGIN is not sent.
        outcome.Success = true;
        outcome.Stopped = true;
        co_return BotTaskResult::Done;
    }

    ClassicPlayerLogin login;
    login.Guid = outcome.CharacterGuid;
    login.FarClip = params.FarClip;
    ctx.Send(login, BotConnectionKind::Realm);

    // The world session either starts right here or, the usual case against both reference
    // servers, the server first redirects it to a second connection.
    std::optional<BotPacket> arrived = co_await ctx.RawPacket({ CLASSIC_SMSG_LOGIN_VERIFY_WORLD, CLASSIC_SMSG_CONNECT_TO }, params.PacketTimeoutMs);
    if (!arrived)
        co_return Fail(who, outcome.Error, NoPacket("SMSG_CONNECT_TO or SMSG_LOGIN_VERIFY_WORLD"));

    arrived->Data.rpos(0);
    if (arrived->Opcode == CLASSIC_SMSG_CONNECT_TO)
    {
        ClassicConnectTo connectTo;
        if (!connectTo.Read(arrived->Data))
            co_return Fail(who, outcome.Error, "malformed SMSG_CONNECT_TO");

        char addressStr[16];

        // Up to five attempts, the way both reference servers count them: each failure is reported
        // on the Realm connection (CMSG_CONNECT_TO_FAILED, echoing the attempt's serial) and the
        // server answers with the next SMSG_CONNECT_TO, until the fifth failure aborts the login.
        for (uint32 attempt = 1;; ++attempt)
        {
            std::snprintf(addressStr, sizeof(addressStr), "%u.%u.%u.%u",
                connectTo.AddressIPv4[0], connectTo.AddressIPv4[1], connectTo.AddressIPv4[2], connectTo.AddressIPv4[3]);

            BotInstanceConnectParams instanceParams;
            instanceParams.WorldAddress = addressStr;
            instanceParams.WorldPort = connectTo.Port;
            instanceParams.SessionKey = params.SessionKey;
            instanceParams.PackedKey = connectTo.Key;
            instanceParams.VerifyServerSignature = params.VerifyServerSignature;
            instanceParams.ConnectTimeoutMs = params.ConnectTimeoutMs;
            instanceParams.PacketTimeoutMs = params.PacketTimeoutMs;

            BotInstanceConnectOutcome instanceOutcome;
            co_await ctx.Run(BotInstanceConnectRoutine(ctx, std::move(instanceParams), instanceOutcome));
            if (instanceOutcome.Success)
                break;

            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "%s: instance connect attempt %u failed: %s", who, attempt, instanceOutcome.Error.c_str());
            ctx.CloseConnection(BotConnectionKind::Instance);

            ClassicConnectToFailed failed;
            failed.Serial = connectTo.Serial;
            failed.Connection = connectTo.ConnectionType;
            ctx.Send(failed, BotConnectionKind::Realm);

            if (attempt >= MaxInstanceConnectAttempts)
                co_return Fail(who, outcome.Error, "instance connect failed " + std::to_string(attempt) + " times, last: " + instanceOutcome.Error);

            std::optional<ClassicConnectTo> next = co_await ctx.Packet<ClassicConnectTo>(params.PacketTimeoutMs);
            if (!next)
                co_return Fail(who, outcome.Error, "instance connect: " + instanceOutcome.Error + "; then " + NoPacket("SMSG_CONNECT_TO after CMSG_CONNECT_TO_FAILED"));
            connectTo = *next;
        }

        std::optional<ClassicLoginVerifyWorld> verify = co_await ctx.Packet<ClassicLoginVerifyWorld>(params.PacketTimeoutMs);
        if (!verify)
            co_return Fail(who, outcome.Error, NoPacket("SMSG_LOGIN_VERIFY_WORLD"));
    }
    else
    {
        ClassicLoginVerifyWorld verify;
        if (!verify.Read(arrived->Data))
            co_return Fail(who, outcome.Error, "malformed SMSG_LOGIN_VERIFY_WORLD");
    }

    outcome.Success = true;
    co_return BotTaskResult::Done;
}

BotRoutine BotWorldLoginRoutine(BotContext& ctx, BotWorldLoginParams params, BotWorldLoginOutcome& outcome)
{
    BotWorldHandshakeOutcome handshake;
    co_await ctx.Run(BotWorldHandshakeRoutine(ctx, std::move(params.Handshake), handshake));
    if (!handshake.Success)
    {
        outcome.Error = "world handshake: " + handshake.Error;
        co_return BotTaskResult::Failed;
    }
    if (handshake.Stopped)
    {
        outcome.Success = true;
        outcome.Stopped = true;
        co_return BotTaskResult::Done;
    }

    params.CharSelect.SessionKey = handshake.SessionKey;

    BotCharSelectOutcome charSelect;
    co_await ctx.Run(BotCharSelectRoutine(ctx, std::move(params.CharSelect), charSelect));
    if (!charSelect.Success)
    {
        outcome.Error = "char select: " + charSelect.Error;
        co_return BotTaskResult::Failed;
    }

    outcome.Success = true;
    outcome.Stopped = charSelect.Stopped;
    outcome.CharacterGuid = charSelect.CharacterGuid;
    co_return BotTaskResult::Done;
}
