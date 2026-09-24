#ifndef REALBOTS_BOTCONTEXT_H
#define REALBOTS_BOTCONTEXT_H

#include "BotIdentity.h"
#include "BotLogin.h"
#include "BotPacket.h"
#include "BotStage.h"
#include "BotTaskArgs.h"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class RealBot;
class BotTask;
struct BotRoutine;

template<typename T> class PacketAwaiter;
class RawPacketAwaiter;
class StageAwaiter;
class DelayAwaiter;
class RunAwaiter;
class HoldAwaiter;
class LoginAwaiter;
class ConnectAwaiter;

// The API a BotRoutine or a hook-style BotTask sees. A thin, stateless facade over
// RealBot: it does not track "which task is asking" itself -- awaiters find their
// own owning BotRoutineTask through the coroutine handle they're given
// (bot-phase-1.md, "Talking to the server").
class BotContext
{
    public:
        explicit BotContext(RealBot& bot) : m_bot(bot) {}

        RealBot& GetBot() const { return m_bot; }

        BotStage GetStage() const;
        void SetStage(BotStage stage);
        BotIdentity const& GetIdentity() const;
        uint32 GetLatencyMs() const;
        uint32 GetTimeMs() const;

        void Disconnect(bool clean);
        void Log(char const* fmt, ...);

        // Pushes a task without waiting for it.
        void PushTask(std::unique_ptr<BotTask> task);

        // Adds a hook beside the stack: it sees OnPacket (before the stack's tasks) and
        // OnStageChanged, never Update, and never buries the running routine. Dropped on
        // disconnect unless persistent. Returns the handle RemoveHook takes.
        BotTask* AddHook(std::unique_ptr<BotTask> hook, bool persistent = false);
        void RemoveHook(BotTask* hook);

        // The connection is `connection` if given, else the opcode's default route (RealBot::Send).
        template<typename T>
        void Send(T const& packet, std::optional<BotConnectionKind> connection = std::nullopt);

        // Sends the packet on `connection` and, as it goes out, arms that connection's encryption
        // with `encryptKey` (RealBot::SendRaw's armEncryption): the CMSG_ENTER_ENCRYPTED_MODE_ACK.
        template<typename T>
        void SendAndArmEncryption(T const& packet, BotConnectionKind connection, std::array<uint8, 32> const& encryptKey);

        template<typename T>
        PacketAwaiter<T> Packet(uint32 timeoutMs, std::function<bool(T const&)> predicate = nullptr);

        RawPacketAwaiter RawPacket(uint32 opcode, uint32 timeoutMs);
        // The first of several opcodes to arrive (SMSG_CONNECT_TO or SMSG_LOGIN_VERIFY_WORLD, say).
        RawPacketAwaiter RawPacket(std::vector<uint32> opcodes, uint32 timeoutMs);
        StageAwaiter Stage(BotStage stage, uint32 timeoutMs);
        DelayAwaiter Delay(uint32 ms);

        // Runs another task to completion on top of this one and hands back its result.
        RunAwaiter Run(std::string taskName, BotTaskArgs args);
        RunAwaiter Run(BotRoutine routine);

        // Stays suspended until popped (removal, or the whole brain being cleared) or disconnected.
        HoldAwaiter Hold();

        // Drives BotLogin::Login on the bot's IoContext and hands back its Result -- the built-in
        // "login" task's whole body (Tasks/BuiltinBotTasks.cpp). The IO-thread completion is
        // marshalled onto this bot's own thread via Post before the coroutine resumes
        // (bot-plan.md, "IO threads only move bytes into a bot's inbox").
        LoginAwaiter Login(BotLogin::Options options);

        // Opens a world connection of the given kind and starts its V2 banner exchange. The bot
        // owns it from then on: every packet arrives in its inbox tagged with `kind`, hooks and
        // all, and the first thing the server sends (SMSG_AUTH_CHALLENGE) is for the routine's
        // next ctx.Packet. Resolves once the TCP connection is up (or failed), with timeoutMs
        // bounding the connect itself.
        ConnectAwaiter Connect(BotConnectionKind kind, std::string address, uint16_t port, uint32 timeoutMs);
        void CloseConnection(BotConnectionKind kind);

    private:
        RealBot& m_bot;
};

#endif
