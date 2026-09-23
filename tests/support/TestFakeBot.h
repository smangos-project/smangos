#ifndef REALBOTS_TESTFAKEBOT_H
#define REALBOTS_TESTFAKEBOT_H

#include "BotAwaitables.h"
#include "BotTaskRegistry.h"
#include "RealBot.h"
#include <memory>
#include <utility>

// A RealBot with no connection at all, ticked by hand and fed packets directly.
// Exercises the coroutine layer without a server (bot-phase-1.md, "Tested without
// a server").
class TestFakeBot : public RealBot
{
    public:
        // factory must return a BotRoutine created by calling a named coroutine
        // function; factory itself must not be marked -> BotRoutine with a
        // co_await/co_return body. This call returns immediately while the
        // routine can still be resumed many Ticks later, so unlike RunTest it
        // cannot keep factory's closure alive that long -- a capturing lambda
        // that is itself the coroutine would leave the frame's implicit `this`
        // dangling the moment this call returns.
        template<typename Factory>
        void PushRoutine(Factory&& factory)
        {
            BotRoutine routine = factory(GetContext());
            GetBrain().PushTask(std::make_unique<BotRoutineTask>("test", std::move(routine)), GetContext());
        }

        template<typename T>
        void InjectIncoming(T const& packet)
        {
            BotPacket raw;
            raw.Opcode = T::Opcode;
            packet.Write(raw.Data);
            QueueIncoming(std::move(raw));
        }

        void InjectIncomingRaw(uint32 opcode, ByteBuffer data)
        {
            BotPacket raw;
            raw.Opcode = opcode;
            raw.Data = std::move(data);
            QueueIncoming(std::move(raw));
        }
};

#endif
