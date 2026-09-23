#ifndef REALBOTS_BOTTASKREGISTRY_H
#define REALBOTS_BOTTASKREGISTRY_H

#include "BotAwaitables.h"
#include "BotTaskArgs.h"
#include "Policies/Singleton.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

// Names tasks so they can be started from a runner/.realbot command, a target's
// default task list, or ctx.Run("name", args) (bot-phase-1.md, "Behavior/").
class BotTaskRegistry
{
    public:
        using Factory = std::function<std::unique_ptr<BotTask>(BotContext&, BotTaskArgs const&)>;

        void RegisterTask(std::string name, Factory factory);

        template<typename RoutineFactory>
        void RegisterRoutine(std::string name, RoutineFactory routineFactory)
        {
            RegisterTask(name, [name, routineFactory](BotContext& ctx, BotTaskArgs const& args) -> std::unique_ptr<BotTask>
            {
                return std::make_unique<BotRoutineTask>(name, routineFactory(ctx, args));
            });
        }

        bool Contains(std::string const& name) const;
        std::unique_ptr<BotTask> Create(std::string const& name, BotContext& ctx, BotTaskArgs const& args) const;

    private:
        std::unordered_map<std::string, Factory> m_factories;
};

#define sBotTaskRegistry MaNGOS::Singleton<BotTaskRegistry>::Instance()

#endif
