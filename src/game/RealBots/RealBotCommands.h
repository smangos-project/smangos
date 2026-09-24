#ifndef REALBOTS_REALBOTCOMMANDS_H
#define REALBOTS_REALBOTCOMMANDS_H

#include <functional>
#include <string>

// One command-verb implementation shared by contrib/realbotrunner's console and, once step 7 wires
// it in, mangosd's `.realbot` chat command (bot-phase-1.md, "Commands"): targets, add, remove,
// list, info, task, tasks, status. Every verb goes through sRealBotMgr and writes its output one
// line at a time through a caller-supplied sink instead of std::cout/chat directly, so both hosts
// get identical behaviour for free. Host-specific things (the runner's "quit"/"exit", any chat
// permission check) stay in the host, not here.
namespace RealBotCommands
{
    using Sink = std::function<void(std::string const&)>;

    // Parses and executes one command line (verb plus arguments, exactly as typed at a console or
    // in a .realbot chat command). Unknown verbs print a short hint through sink rather than
    // throwing or doing nothing silently.
    void Handle(std::string const& line, Sink const& sink);

    // The command list, one line per verb -- what the "help" verb prints.
    void PrintHelp(Sink const& sink);
}

#endif
