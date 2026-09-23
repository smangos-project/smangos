#include "BotTaskRegistry.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1(BotTaskRegistry);

void BotTaskRegistry::RegisterTask(std::string name, Factory factory)
{
    m_factories[std::move(name)] = std::move(factory);
}

bool BotTaskRegistry::Contains(std::string const& name) const
{
    return m_factories.contains(name);
}

std::unique_ptr<BotTask> BotTaskRegistry::Create(std::string const& name, BotContext& ctx, BotTaskArgs const& args) const
{
    auto it = m_factories.find(name);
    if (it == m_factories.end())
        return nullptr;

    return it->second(ctx, args);
}
