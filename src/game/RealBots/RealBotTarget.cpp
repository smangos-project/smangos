#include "RealBotTarget.h"
#include "Config/Config.h"
#include "Log.h"
#include <algorithm>
#include <sstream>

namespace
{
std::vector<std::string> SplitWords(std::string const& text)
{
    std::vector<std::string> words;
    std::istringstream iss(text);
    std::string word;
    while (iss >> word)
        words.push_back(word);
    return words;
}

// "name:pass" -- the account name itself may contain '@' (Battle.net-style), so this splits on
// the first ':' rather than assuming any particular shape for the name half.
bool ParseAccount(std::string const& token, RealBotAccountCredential& out)
{
    size_t const colon = token.find(':');
    if (colon == std::string::npos)
        return false;

    out.Name = token.substr(0, colon);
    out.Password = token.substr(colon + 1);
    return !out.Name.empty() && !out.Password.empty();
}
}

bool RealBotTarget::HasTag(std::string const& tag) const
{
    return std::find(Tags.begin(), Tags.end(), tag) != Tags.end();
}

bool RealBotTarget::Load(std::string const& name, RealBotTarget& out, std::string& error)
{
    std::string const prefix = "RealBots.Target." + name + ".";

    if (!sConfig.IsSet((prefix + "LoginAddress").c_str()))
    {
        error = "missing " + prefix + "LoginAddress";
        return false;
    }

    RealBotTarget target;
    target.Name = name;
    target.LoginAddress = sConfig.GetStringDefault((prefix + "LoginAddress").c_str(), "");
    target.RestPort = uint16_t(sConfig.GetIntDefault((prefix + "RestPort").c_str(), 8081));
    target.RpcPort = uint16_t(sConfig.GetIntDefault((prefix + "RpcPort").c_str(), 1119));
    target.RealmName = sConfig.GetStringDefault((prefix + "RealmName").c_str(), "");
    target.RealmId = uint32_t(sConfig.GetIntDefault((prefix + "RealmId").c_str(), 1));
    target.ClientBuild = sConfig.GetIntDefault((prefix + "ClientBuild").c_str(), 58797);
    target.ClientVersion = sConfig.GetStringDefault((prefix + "ClientVersion").c_str(), "1.15.6");
    target.Tags = SplitWords(sConfig.GetStringDefault((prefix + "Tags").c_str(), ""));
    target.VerifyServerSignature = sConfig.GetBoolDefault((prefix + "VerifyServerSignature").c_str(), true);
    target.ConnectTimeoutMs = uint32_t(sConfig.GetIntDefault((prefix + "ConnectTimeoutMs").c_str(), 10000));
    target.StageTimeoutMs = uint32_t(sConfig.GetIntDefault((prefix + "StageTimeoutMs").c_str(), 30000));
    target.ServerCloseTimeoutMs = uint32_t(sConfig.GetIntDefault((prefix + "ServerCloseTimeoutMs").c_str(), 30000));
    target.IdleSessionMs = uint32_t(sConfig.GetIntDefault((prefix + "IdleSessionMs").c_str(), 120000));
    target.LoadBots = uint32_t(sConfig.GetIntDefault((prefix + "LoadBots").c_str(), 0));

    for (std::string const& token : SplitWords(sConfig.GetStringDefault((prefix + "Accounts").c_str(), "")))
    {
        RealBotAccountCredential account;
        if (ParseAccount(token, account))
            target.Accounts.push_back(std::move(account));
        else
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "RealBots: target '%s': malformed account entry '%s' (expected name:pass)",
                name.c_str(), token.c_str());
    }

    out = std::move(target);
    return true;
}

void RealBotTargetList::Load()
{
    m_targets.clear();
    for (std::string const& name : SplitWords(sConfig.GetStringDefault("RealBots.Targets", "")))
    {
        RealBotTarget target;
        std::string error;
        if (RealBotTarget::Load(name, target, error))
            m_targets.push_back(std::move(target));
        else
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "RealBots: %s", error.c_str());
    }
}

RealBotTarget const* RealBotTargetList::Find(std::string const& name) const
{
    for (RealBotTarget const& target : m_targets)
        if (target.Name == name)
            return &target;
    return nullptr;
}
