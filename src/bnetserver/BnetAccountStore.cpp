#include "BnetAccountStore.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "Util.h"

#include <algorithm>

namespace
{
std::vector<uint8> DecodeHex(std::string const& hex)
{
    if (hex.empty() || (hex.size() % 2) != 0)
        return {};
    std::vector<uint8> out(hex.size() / 2);
    HexStrToByteArray(hex, out.data());
    return out;
}

// One ban row for an account, if any is in force.
struct BanInfo
{
    bool banned = false;
    bool permanent = false;
    uint32 unbanDate = 0;
    std::string reason;
};

BanInfo LoadBan(char const* table, uint32 id)
{
    BanInfo ban;
    std::unique_ptr<QueryResult> result = LoginDatabase.PQuery(
        "SELECT `bandate`, `unbandate`, `banreason` FROM `%s` WHERE `id` = %u AND `active` = 1 AND (`unbandate` > UNIX_TIMESTAMP() OR `unbandate` = `bandate`) LIMIT 1",
        table, id);
    if (!result)
        return ban;

    Field* fields = result->Fetch();
    uint64 const banDate = fields[0].GetUInt64();
    uint64 const unbanDate = fields[1].GetUInt64();
    ban.banned = true;
    ban.permanent = banDate == unbanDate;
    ban.unbanDate = uint32(std::min<uint64>(unbanDate, 0xFFFFFFFFull));
    ban.reason = fields[2].GetCppString();
    return ban;
}
}

AccountTypes BnetGameAccount::GetSecurityOn(uint32 realmId) const
{
    auto itr = securityByRealm.find(realmId);
    return itr != securityByRealm.end() ? itr->second : defaultSecurity;
}

BnetGameAccount const* BnetAccount::FindGameAccount(uint32 gameAccountId) const
{
    for (BnetGameAccount const& gameAccount : gameAccounts)
        if (gameAccount.id == gameAccountId)
            return &gameAccount;
    return nullptr;
}

bool BnetAccountStore::NormalizeLogin(std::string& login)
{
    std::wstring wide;
    if (!Utf8toWStr(login, wide))
        return false;

    std::transform(wide.begin(), wide.end(), wide.begin(), wcharToUpperOnlyLatin);
    return WStrToUtf8(wide, login);
}

nonstd::optional<BnetSrpCredentials> BnetAccountStore::LoadSrpCredentials(std::string const& normalizedLogin)
{
    std::string safeLogin = normalizedLogin;
    LoginDatabase.escape_string(safeLogin);

    std::unique_ptr<QueryResult> result = LoginDatabase.PQuery(
        "SELECT `id`, `srp_version`, `salt`, `verifier` FROM `battlenet_accounts` WHERE `email` = '%s'", safeLogin.c_str());
    if (!result)
        return nonstd::nullopt;

    Field* fields = result->Fetch();
    BnetSrpCredentials credentials;
    credentials.bnetAccountId = fields[0].GetUInt32();
    credentials.srpVersion = fields[1].GetUInt8();
    credentials.salt = DecodeHex(fields[2].GetCppString());
    credentials.verifier = DecodeHex(fields[3].GetCppString());

    if (credentials.salt.empty() || credentials.verifier.empty())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Battle.net account %u has no usable SRP salt/verifier; reset its password", credentials.bnetAccountId);
        return nonstd::nullopt;
    }
    return credentials;
}

nonstd::optional<BnetAccount> BnetAccountStore::LoadAccount(uint32 bnetAccountId)
{
    std::unique_ptr<QueryResult> result = LoginDatabase.PQuery(
        "SELECT `email` FROM `battlenet_accounts` WHERE `id` = %u", bnetAccountId);
    if (!result)
        return nonstd::nullopt;

    BnetAccount account;
    account.id = bnetAccountId;
    account.login = result->Fetch()[0].GetCppString();

    BanInfo const accountBan = LoadBan("battlenet_account_bans", bnetAccountId);
    account.banned = accountBan.banned;
    account.permanentlyBanned = accountBan.permanent;
    account.unbanDate = accountBan.unbanDate;

    result = LoginDatabase.PQuery(
        "SELECT `id`, `username`, `expansion` FROM `account` WHERE `battlenet_account` = %u ORDER BY `battlenet_index`", bnetAccountId);
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            BnetGameAccount gameAccount;
            gameAccount.id = fields[0].GetUInt32();
            gameAccount.name = fields[1].GetCppString();
            gameAccount.expansion = fields[2].GetUInt8();

            BanInfo const gameBan = LoadBan("account_banned", gameAccount.id);
            gameAccount.banned = gameBan.banned;
            gameAccount.permanentlyBanned = gameBan.permanent;
            gameAccount.unbanDate = gameBan.unbanDate;
            gameAccount.banReason = gameBan.reason;

            account.gameAccounts.push_back(std::move(gameAccount));
        } while (result->NextRow());
    }

    for (BnetGameAccount& gameAccount : account.gameAccounts)
    {
        std::unique_ptr<QueryResult> access = LoginDatabase.PQuery(
            "SELECT `gmlevel`, `RealmID` FROM `account_access` WHERE `id` = %u", gameAccount.id);
        if (!access)
            continue;

        do
        {
            Field* fields = access->Fetch();
            AccountTypes const security = AccountTypes(std::min<uint32>(fields[0].GetUInt32(), SEC_ADMINISTRATOR));
            int32 const realmId = fields[1].GetInt32();
            if (realmId < 0)
                gameAccount.defaultSecurity = security;
            else
                gameAccount.securityByRealm[uint32(realmId)] = security;
        } while (access->NextRow());
    }

    return account;
}

bool BnetAccountStore::IsIpBanned(std::string const& ip)
{
    std::string safeIp = ip;
    LoginDatabase.escape_string(safeIp);
    std::unique_ptr<QueryResult> result = LoginDatabase.PQuery(
        "SELECT 1 FROM `ip_banned` WHERE `ip` = '%s' AND (`unbandate` = `bandate` OR `unbandate` > UNIX_TIMESTAMP()) LIMIT 1", safeIp.c_str());
    return result != nullptr;
}

std::map<uint32, uint32> BnetAccountStore::LoadCharacterCounts(uint32 gameAccountId)
{
    std::map<uint32, uint32> counts;
    std::unique_ptr<QueryResult> result = LoginDatabase.PQuery(
        "SELECT `realmid`, `numchars` FROM `realmcharacters` WHERE `acctid` = %u", gameAccountId);
    if (!result)
        return counts;

    do
    {
        Field* fields = result->Fetch();
        counts[fields[0].GetUInt32()] = fields[1].GetUInt32();
    } while (result->NextRow());
    return counts;
}

nonstd::optional<BnetAccountStore::LastPlayedCharacter> BnetAccountStore::LoadLastPlayedCharacter(uint32 gameAccountId, uint8 region, uint8 battlegroup)
{
    std::unique_ptr<QueryResult> result = LoginDatabase.PQuery(
        "SELECT `realmId`, `characterName`, `characterGuid`, `lastPlayedTime` FROM `account_last_played_character` "
        "WHERE `accountId` = %u AND `region` = %u AND `battlegroup` = %u", gameAccountId, uint32(region), uint32(battlegroup));
    if (!result)
        return nonstd::nullopt;

    Field* fields = result->Fetch();
    LastPlayedCharacter character;
    character.realmId = fields[0].GetUInt32();
    character.name = fields[1].GetCppString();
    character.guid = fields[2].GetUInt64();
    character.lastPlayedTime = fields[3].GetUInt32();
    return character;
}

bool BnetAccountStore::StoreJoinInfo(uint32 gameAccountId, std::string const& sessionKeyHex, std::string const& ip,
    uint8 locale, std::string const& os)
{
    std::string safeIp = ip;
    LoginDatabase.escape_string(safeIp);
    std::string safeOs = os;
    LoginDatabase.escape_string(safeOs);

    return LoginDatabase.PExecute(DbExecMode::MustBeSync,
        "UPDATE `account` SET `session_key_bnet` = '%s', `last_ip` = '%s', `last_login` = NOW(), `locale` = %u, `os` = '%s' WHERE `id` = %u",
        sessionKeyHex.c_str(), safeIp.c_str(), uint32(locale), safeOs.c_str(), gameAccountId);
}

void BnetAccountStore::StoreLogin(uint32 bnetAccountId, std::string const& ip)
{
    std::string safeIp = ip;
    LoginDatabase.escape_string(safeIp);
    LoginDatabase.PExecute("UPDATE `battlenet_accounts` SET `last_ip` = '%s', `last_login` = NOW() WHERE `id` = %u",
        safeIp.c_str(), bnetAccountId);
}
