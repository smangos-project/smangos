#ifndef REALBOTS_BOTAWAITABLES_H
#define REALBOTS_BOTAWAITABLES_H

#include "BotRoutine.h"
#include "BotConnector.h"
#include "BotContext.h"
#include "BotWorldConnection.h"
#include "IO/Networking/IpAddress.h"
#include "RealBot.h"
#include <coroutine>
#include <functional>
#include <optional>
#include <vector>

// Common suspend/register bookkeeping every awaiter below needs: never skip
// suspension, and register with the coroutine's owning BotRoutineTask so it gets
// polled from RealBot::Tick.
class BotAwaiterBase : public IBotAwaiter
{
    public:
        bool await_ready() const noexcept { return false; }

        void await_suspend(BotRoutine::HandleType handle)
        {
            handle.promise().Owner->SetPendingAwaiter(this);
        }

    protected:
        bool TimedOut(uint32 diffMs, uint32 timeoutMs)
        {
            m_elapsedMs += diffMs;
            return m_elapsedMs >= timeoutMs;
        }

    private:
        uint32 m_elapsedMs = 0;
};

// ctx.Packet<T>(timeoutMs[, predicate]) -> std::optional<T>
template<typename T>
class PacketAwaiter : public BotAwaiterBase
{
    public:
        PacketAwaiter(RealBot& bot, uint32 timeoutMs, std::function<bool(T const&)> predicate)
            : m_bot(bot), m_timeoutMs(timeoutMs), m_predicate(std::move(predicate)) {}

        bool Poll(RealBot&, uint32 diffMs) override
        {
            return TimedOut(diffMs, m_timeoutMs);
        }

        bool OnPacket(RealBot&, BotPacket const& packet, BotPacketDir dir) override
        {
            if (dir != BotPacketDir::Incoming || packet.Opcode != T::Opcode)
                return false;

            ByteBuffer body = packet.Data;
            body.rpos(0);

            T value;
            if (!value.Read(body))
                return false;

            if (m_predicate && !m_predicate(value))
                return false;

            m_result = std::move(value);
            return true;
        }

        std::optional<T> await_resume() { return std::move(m_result); }

    private:
        RealBot& m_bot;
        uint32 m_timeoutMs;
        std::function<bool(T const&)> m_predicate;
        std::optional<T> m_result;
};

// ctx.RawPacket(opcode, timeoutMs) -> std::optional<BotPacket>
class RawPacketAwaiter : public BotAwaiterBase
{
    public:
        RawPacketAwaiter(RealBot& bot, uint32 opcode, uint32 timeoutMs);
        RawPacketAwaiter(RealBot& bot, std::vector<uint32> opcodes, uint32 timeoutMs);

        bool Poll(RealBot&, uint32 diffMs) override;
        bool OnPacket(RealBot&, BotPacket const& packet, BotPacketDir dir) override;
        std::optional<BotPacket> await_resume() { return std::move(m_result); }

    private:
        std::vector<uint32> m_opcodes;
        uint32 m_timeoutMs;
        std::optional<BotPacket> m_result;
};

// ctx.Stage(stage, timeoutMs) -> bool
class StageAwaiter : public BotAwaiterBase
{
    public:
        StageAwaiter(RealBot& bot, BotStage stage, uint32 timeoutMs);

        bool Poll(RealBot& bot, uint32 diffMs) override;
        bool await_resume() const { return m_reached; }

    private:
        BotStage m_target;
        uint32 m_timeoutMs;
        bool m_reached = false;
};

// ctx.Delay(ms)
class DelayAwaiter : public BotAwaiterBase
{
    public:
        explicit DelayAwaiter(uint32 ms) : m_ms(ms) {}

        bool Poll(RealBot&, uint32 diffMs) override { return TimedOut(diffMs, m_ms); }
        void await_resume() const {}

    private:
        uint32 m_ms;
};

// ctx.Run(...) -> BotTaskResult
class RunAwaiter : public BotAwaiterBase
{
    public:
        RunAwaiter(BotBrain& brain, BotContext& ctx, std::unique_ptr<BotTask> child)
            : m_brain(brain), m_ctx(ctx), m_child(std::move(child)) {}

        void await_suspend(BotRoutine::HandleType handle)
        {
            BotAwaiterBase::await_suspend(handle);
            m_brain.PushTask(std::move(m_child), m_ctx);
        }

        bool Poll(RealBot&, uint32) override { return false; }
        bool OnChildComplete(BotTaskResult result) override { m_result = result; return true; }
        BotTaskResult await_resume() const { return m_result; }

    private:
        BotBrain& m_brain;
        BotContext& m_ctx;
        std::unique_ptr<BotTask> m_child;
        BotTaskResult m_result = BotTaskResult::Failed;
};

// ctx.Hold(): never resolves on its own; the frame is destroyed, never resumed,
// when the brain is cleared (BotBrain::Clear).
class HoldAwaiter : public BotAwaiterBase
{
    public:
        bool Poll(RealBot&, uint32) override { return false; }
        BotTaskResult await_resume() const { return BotTaskResult::Done; }
};

// ctx.Login(options) -> BotLogin::Result. Kicks off BotLogin::Login on the bot's IoContext from
// await_suspend; the completion (on the IO thread) is marshalled onto the bot's own thread via
// RealBot::Post into a block shared with the coroutine frame, so Poll (only ever called from
// RealBot::Tick) never touches anything the IO thread might still be writing to. Uses a
// std::weak_ptr<RealBot> rather than the bot reference directly, exactly like
// RealBot::AttachConnections's own callbacks -- the bot can be removed while a login is in
// flight, and the completion must become a harmless no-op rather than reach into a dead object.
class LoginAwaiter : public BotAwaiterBase
{
    public:
        LoginAwaiter(RealBot& bot, BotLogin::Options options)
            : m_bot(bot), m_options(std::move(options)) {}

        void await_suspend(BotRoutine::HandleType handle)
        {
            BotAwaiterBase::await_suspend(handle);

            std::shared_ptr<Shared> shared = m_shared;
            std::weak_ptr<RealBot> weakBot = m_bot.weak_from_this();
            std::shared_ptr<BotLogin> login = BotLogin::Create(m_bot.GetIoContext());
            login->Login(m_options, [weakBot, shared](BotLogin::Result result)
            {
                if (std::shared_ptr<RealBot> bot = weakBot.lock())
                {
                    bot->Post([shared, result = std::move(result)]() mutable
                    {
                        shared->Result = std::move(result);
                        shared->Done = true;
                    });
                }
            });
        }

        bool Poll(RealBot&, uint32) override { return m_shared->Done; }
        BotLogin::Result await_resume() { return std::move(m_shared->Result); }

    private:
        struct Shared
        {
            bool Done = false;
            BotLogin::Result Result;
        };

        RealBot& m_bot;
        BotLogin::Options m_options;
        std::shared_ptr<Shared> m_shared = std::make_shared<Shared>();
};

struct BotConnectResult
{
    bool Success = false;
    std::string Error;
};

// ctx.Connect(kind, address, port, timeoutMs) -> BotConnectResult. The TCP connect runs on a
// BotConnector thread; the completion is marshalled onto the bot's own thread via RealBot::Post,
// and only there does the new BotWorldConnection get attached (Poll, from RealBot::Tick), so the
// bot's connection slots are never touched from another thread. The connection is created with
// the bot's own callbacks (RealBot::MakeWorldCallbacks) before its banner exchange starts, so the
// first packets the server sends can never arrive at nobody.
class ConnectAwaiter : public BotAwaiterBase
{
    public:
        ConnectAwaiter(RealBot& bot, BotConnectionKind kind, std::string address, uint16_t port, uint32 timeoutMs)
            : m_bot(bot), m_kind(kind), m_address(std::move(address)), m_port(port), m_timeoutMs(timeoutMs) {}

        ConnectAwaiter(ConnectAwaiter const&) = delete;
        ConnectAwaiter& operator=(ConnectAwaiter const&) = delete;
        ConnectAwaiter(ConnectAwaiter&&) = default;

        ~ConnectAwaiter()
        {
            // Destroyed without ever being polled to completion (the bot was disconnected while
            // this was in flight): a connection that arrives later has no owner, so it is closed
            // instead of attached. Both this and the completion run on the bot's thread.
            if (m_shared)
            {
                m_shared->Abandoned = true;
                if (m_shared->Connection)
                    m_shared->Connection->Close();
            }
        }

        void await_suspend(BotRoutine::HandleType handle)
        {
            BotAwaiterBase::await_suspend(handle);

            auto ip = IO::Networking::IpAddress::TryParseFromString(m_address);
            if (!ip.has_value())
            {
                m_shared->Result.Error = "bad address '" + m_address + "'";
                m_shared->Done = true;
                return;
            }
            IO::Networking::IpEndpoint endpoint(*ip, m_port);

            std::shared_ptr<Shared> shared = m_shared;
            std::weak_ptr<RealBot> weakBot = m_bot.weak_from_this();
            RealBot::WorldCallbacks callbacks = m_bot.MakeWorldCallbacks(m_kind);
            IO::IoContext* ioContext = m_bot.GetIoContext();

            BotConnector::ConnectAsync(endpoint, std::chrono::milliseconds(m_timeoutMs),
                [weakBot, shared, callbacks, ioContext](nonstd::expected<IO::Networking::SocketDescriptor, IO::NetworkError> descriptor)
                {
                    std::shared_ptr<BotWorldConnection> connection;
                    std::string error;
                    if (descriptor.has_value())
                    {
                        IO::Networking::AsyncSocket socket(ioContext, std::move(descriptor.value()));
                        connection = BotWorldConnection::Create(std::move(socket), callbacks.OnPacket, callbacks.OnDisconnect);
                        connection->StartHandshake();
                    }
                    else
                    {
                        error = "connect failed: " + descriptor.error().ToString();
                    }

                    std::shared_ptr<RealBot> bot = weakBot.lock();
                    if (!bot)
                    {
                        if (connection)
                            connection->Close();
                        return;
                    }

                    bot->Post([shared, connection = std::move(connection), error = std::move(error)]() mutable
                    {
                        if (shared->Abandoned)
                        {
                            if (connection)
                                connection->Close();
                            return;
                        }
                        shared->Connection = std::move(connection);
                        shared->Result.Error = std::move(error);
                        shared->Done = true;
                    });
                });
        }

        bool Poll(RealBot& bot, uint32) override
        {
            if (!m_shared->Done)
                return false;

            if (m_shared->Connection)
            {
                bot.AttachConnection(m_kind, std::move(m_shared->Connection));
                m_shared->Result.Success = true;
            }
            return true;
        }

        BotConnectResult await_resume() { return std::move(m_shared->Result); }

    private:
        struct Shared
        {
            bool Done = false;
            bool Abandoned = false;
            std::shared_ptr<BotWorldConnection> Connection;
            BotConnectResult Result;
        };

        RealBot& m_bot;
        BotConnectionKind m_kind;
        std::string m_address;
        uint16_t m_port;
        uint32 m_timeoutMs;
        std::shared_ptr<Shared> m_shared = std::make_shared<Shared>();
};

template<typename T>
void BotContext::Send(T const& packet, std::optional<BotConnectionKind> connection)
{
    m_bot.Send(packet, connection);
}

template<typename T>
void BotContext::SendAndArmEncryption(T const& packet, BotConnectionKind connection, std::array<uint8, 32> const& encryptKey)
{
    BotPacket raw;
    raw.Opcode = T::Opcode;
    packet.Write(raw.Data);
    m_bot.SendRaw(std::move(raw), connection, &encryptKey);
}

template<typename T>
PacketAwaiter<T> BotContext::Packet(uint32 timeoutMs, std::function<bool(T const&)> predicate)
{
    return PacketAwaiter<T>(m_bot, timeoutMs, std::move(predicate));
}

inline LoginAwaiter BotContext::Login(BotLogin::Options options)
{
    return LoginAwaiter(m_bot, std::move(options));
}

inline ConnectAwaiter BotContext::Connect(BotConnectionKind kind, std::string address, uint16_t port, uint32 timeoutMs)
{
    return ConnectAwaiter(m_bot, kind, std::move(address), port, timeoutMs);
}

#endif
