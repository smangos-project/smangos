#include "RealBotCommands.h"
#include "BotStage.h"
#include "BotTaskArgs.h"
#include "RealBotConfig.h"
#include "RealBotMgr.h"
#include "RealBotTarget.h"
#include <algorithm>
#include <optional>
#include <sstream>
#include <unordered_map>

namespace
{
std::string TrimLeadingSpace(std::string s)
{
    size_t const pos = s.find_first_not_of(' ');
    return pos == std::string::npos ? std::string() : s.substr(pos);
}

void PrintTargets(RealBotCommands::Sink const& sink)
{
    for (RealBotTarget const& target : sRealBotMgr.GetTargets().All())
    {
        std::ostringstream line;
        line << target.Name << "\t" << target.LoginAddress << ":" << target.RestPort << "/" << target.RpcPort << "\ttags:";
        for (std::string const& tag : target.Tags)
            line << " " << tag;

        if (std::optional<std::pair<size_t, size_t>> counts = sRealBotMgr.GetLeaseCounts(target.Name))
            line << "\taccounts " << counts->first << "/" << counts->second << " free";

        sink(line.str());
    }
}

void HandleAdd(std::istringstream& iss, RealBotCommands::Sink const& sink)
{
    std::string targetName;
    iss >> targetName;
    if (targetName.empty())
        targetName = RealBotConfig::DefaultTarget();

    uint32_t count = 1;
    uint32_t parsedCount = 0;
    if (iss >> parsedCount) // absent (or unparsable) leaves count at its default of 1
        count = parsedCount;

    if (targetName.empty())
    {
        sink("add needs a target name (or set RealBots.DefaultTarget) -- try 'targets'");
        return;
    }

    for (uint32_t i = 0; i < std::max<uint32_t>(1, count); ++i)
    {
        std::string error;
        std::string const name = sRealBotMgr.Add(targetName, "", &error);
        sink(name.empty() ? ("add: " + error) : ("bot '" + name + "' is connecting"));
    }
}

void HandleRemove(std::istringstream& iss)
{
    std::string name;
    iss >> name;
    if (name == "all")
        sRealBotMgr.RemoveAll();
    else
        sRealBotMgr.Remove(name);
}

void HandleList(RealBotCommands::Sink const& sink)
{
    for (std::string const& name : sRealBotMgr.List())
        if (std::optional<RealBotInfo> info = sRealBotMgr.GetInfo(name))
            sink(info->Name + "\t" + BotStageName(info->Stage) + "\t" + std::to_string(info->LatencyMs) + " ms");
}

void HandleInfo(std::istringstream& iss, RealBotCommands::Sink const& sink)
{
    std::string name;
    iss >> name;
    std::optional<RealBotInfo> info = sRealBotMgr.GetInfo(name);
    if (!info)
    {
        sink("no such bot '" + name + "'");
        return;
    }
    std::string stageLine = BotStageName(info->Stage);
    if (info->Stage == BotStage::Failed)
        stageLine += " (was " + std::string(BotStageName(info->FailedStage)) + ")";
    sink("stage       " + stageLine);
    sink("latency     " + std::to_string(info->LatencyMs) + " ms");
    sink("character   " + (info->CharacterName.empty() ? "-" : info->CharacterName));
    sink("error       " + (info->LastError.empty() ? "-" : info->LastError));
}

void HandleTask(std::istringstream& iss, RealBotCommands::Sink const& sink)
{
    std::string name, taskName, rest;
    iss >> name >> taskName;
    std::getline(iss, rest);
    if (!sRealBotMgr.RunTask(name, taskName, BotTaskArgs::Parse(TrimLeadingSpace(rest))))
        sink("no such bot or task");
}

void HandleTasks(std::istringstream& iss, RealBotCommands::Sink const& sink)
{
    std::string name;
    iss >> name;
    std::optional<RealBotInfo> info = sRealBotMgr.GetInfo(name);
    if (!info)
    {
        sink("no such bot '" + name + "'");
        return;
    }

    for (size_t i = 0; i < info->TaskStack.size(); ++i)
        sink(info->TaskStack[i] + (i == 0 ? " (top)" : ""));
}

void HandleStatus(RealBotCommands::Sink const& sink)
{
    std::vector<std::string> const names = sRealBotMgr.List();

    std::unordered_map<BotStage, size_t> byStage;
    for (std::string const& name : names)
        if (std::optional<RealBotInfo> info = sRealBotMgr.GetInfo(name))
            ++byStage[info->Stage];

    sink("bots        " + std::to_string(names.size()));
    for (auto const& [stage, count] : byStage)
        sink("  " + std::string(BotStageName(stage)) + "\t" + std::to_string(count));

    sink("targets     " + std::to_string(sRealBotMgr.GetTargets().All().size()));
}
}

void RealBotCommands::PrintHelp(Sink const& sink)
{
    sink("targets                    lists every configured target and its free account count");
    sink("add [target] [count]       leases account(s) from [target] (or RealBots.DefaultTarget) and adds that many bots");
    sink("remove <name|all>          removes a bot");
    sink("list                       lists every bot and its stage");
    sink("info <name>                shows one bot's stage, latency, character, last error");
    sink("task <name> <task> [args]  pushes a named task onto a running bot");
    sink("tasks <name>               lists a bot's task stack, top first");
    sink("status                     summarizes every bot by stage and every target");
    sink("help                       shows this text");
}

void RealBotCommands::Handle(std::string const& line, Sink const& sink)
{
    std::istringstream iss(line);
    std::string verb;
    iss >> verb;
    if (verb.empty())
        return;

    if (verb == "targets")
        PrintTargets(sink);
    else if (verb == "add")
        HandleAdd(iss, sink);
    else if (verb == "remove")
        HandleRemove(iss);
    else if (verb == "list")
        HandleList(sink);
    else if (verb == "info")
        HandleInfo(iss, sink);
    else if (verb == "task")
        HandleTask(iss, sink);
    else if (verb == "tasks")
        HandleTasks(iss, sink);
    else if (verb == "status")
        HandleStatus(sink);
    else if (verb == "help")
        PrintHelp(sink);
    else
        sink("unknown command '" + verb + "' -- try 'help'");
}
