#include "BotRoutine.h"
#include "BotContext.h"

void BotRoutine::promise_type::unhandled_exception()
{
    Result = BotTaskResult::Failed;

    try
    {
        std::rethrow_exception(std::current_exception());
    }
    catch (std::exception const& e)
    {
        ErrorMessage = e.what();
    }
    catch (...)
    {
        ErrorMessage = "unknown exception";
    }
}

BotRoutineTask::BotRoutineTask(std::string name, BotRoutine routine)
    : m_name(std::move(name)), m_routine(std::move(routine))
{
    m_routine.GetHandle().promise().Owner = this;
}

void BotRoutineTask::OnStart(BotContext&)
{
    // Runs eagerly, up to the first suspension, from inside BotBrain::PushTask:
    // that guarantees this task is already on the stack before anything it
    // pushes in turn (ctx.Run) gets pushed, and it lets an awaiter's own clock
    // start at 0 instead of losing however much of the current tick already
    // elapsed before this was reached from RealBot::Tick.
    m_routine.Resume();
}

void BotRoutineTask::ResumeAndCheckException()
{
    m_pendingAwaiter = nullptr;
    m_routine.Resume();
}

BotTaskResult BotRoutineTask::Update(BotContext& ctx, uint32 diff)
{
    if (m_pendingAwaiter && m_pendingAwaiter->Poll(ctx.GetBot(), diff))
        ResumeAndCheckException();

    if (m_routine.IsDone())
        return m_routine.GetResult();

    return BotTaskResult::Running;
}

BotPacketAction BotRoutineTask::OnPacket(BotContext& ctx, BotPacket& packet, BotPacketDir dir)
{
    if (m_pendingAwaiter && m_pendingAwaiter->OnPacket(ctx.GetBot(), packet, dir))
        ResumeAndCheckException();

    return BotPacketAction::Continue;
}

void BotRoutineTask::OnConnectionLost(BotContext& ctx, BotConnectionKind kind)
{
    if (m_pendingAwaiter && m_pendingAwaiter->OnConnectionLost(ctx.GetBot(), kind))
        ResumeAndCheckException();
}

void BotRoutineTask::OnChildComplete(BotContext& ctx, BotTaskResult result)
{
    if (m_pendingAwaiter && m_pendingAwaiter->OnChildComplete(result))
        ResumeAndCheckException();
}
