/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __WORLDSESSION_H
#define __WORLDSESSION_H

#include "Common.h"
#include "SharedDefines.h"
#include "ObjectGuid.h"
#include "AuctionHouseMgr.h"
#include "ItemDefines.h"
#include "GossipDef.h"
#include "Chat/AbstractPlayer.h"
#include "SniffFile.h"
#include "ClientDefines.h"
#include "Crypto/BigNumber.h"
#include "AccountData.h"
#include "PacketProcessing.h"
#include "UpdateData.h"
#include "LockedQueue.h"
#include "Packets/AuctionHouseMessages.h"
#include "Packets/BattlegroundMessages.h"
#include "Packets/ChannelMessages.h"
#include "Packets/Character.h"
#include "Packets/Auth.h"
#include "Packets/CharScreen.h"
#include "Packets/Session.h"
#include "Packets/Classic/Character.h"
#include "Packets/ClassicClient.h"
#include "Packets/Chat.h"
#include "Packets/ChatMessages.h"
#include "Packets/Combat.h"
#include "Packets/CombatMessages.h"
#include "Packets/DeathMessages.h"
#include "Packets/Auras.h"
#include "Packets/SpellCast.h"
#include "Packets/DuelMessages.h"
#include "Packets/GmTicketMessages.h"
#include "Packets/GroupMessages.h"
#include "Packets/GuildMessages.h"
#include "Packets/InspectMessages.h"
#include "Packets/Item.h"
#include "Packets/ItemMessages.h"
#include "Packets/LootMessages.h"
#include "Packets/MailMessages.h"
#include "Packets/Misc.h"
#include "Packets/MiscMessages.h"
#include "Packets/MountSpecialAnim.h"
#include "Packets/MovementMessages.h"
#include "Packets/Npc.h"
#include "Packets/NpcMessages.h"
#include "Packets/PetMessages.h"
#include "Packets/PetitionMessages.h"
#include "Packets/Query.h"
#include "Packets/QueryMessages.h"
#include "Packets/QuestMessages.h"
#include "Packets/Skill.h"
#include "Packets/Social.h"
#include "Packets/Spell.h"
#include "Packets/SpellBook.h"
#include "Packets/TaxiMessages.h"
#include "Packets/VendorMessages.h"
#include "Packets/TradeMessages.h"
#include "Packets/WorldTransfer.h"

#include <array>
#include <map>
#include <memory>

struct ItemPrototype;
struct AuctionEntry;
struct AuctionHouseEntry;

class ObjectGuid;
class Creature;
class Item;
class Object;
class Player;
class Unit;
class WorldPacket;
class WorldSocket;
class QueryResult;
class LoginQueryHolder;
class CharacterHandler;
class MovementInfo;
class WorldSession;
class MovementAnticheat;
class BigNumber;
class MasterPlayer;

struct OpcodeHandler;
struct OpcodeHandlerPacketImplDetails;
struct PlayerBotEntry;

enum PartyOperation
{
    PARTY_OP_INVITE = 0,
    PARTY_OP_UNINVITE = 1,
    PARTY_OP_LEAVE = 2,
};

enum PartyResult
{
    ERR_PARTY_RESULT_OK                 = 0,
    ERR_BAD_PLAYER_NAME_S               = 1,    // "Cannot find '%s'."
    ERR_TARGET_NOT_IN_GROUP_S           = 2,    // "%s is not in your party."
    ERR_GROUP_FULL                      = 4,    // "Your party is full."
    ERR_ALREADY_IN_GROUP_S              = 5,    // "%s is already in a group."
    ERR_NOT_IN_GROUP                    = 6,    // "You aren't in a party."
    ERR_NOT_LEADER                      = 7,    // "You are not the party leader."
    ERR_PLAYER_WRONG_FACTION            = 8,
    ERR_IGNORING_YOU_S                  = 9,    // "%s is ignoring you."
    ERR_INTERNAL_BATTLEGROUND           = 13,   // The client has no reason of its own for this
};

#define ACCOUNT_TUTORIALS_COUNT 8

enum TutorialDataState
{
    TUTORIALDATA_UNCHANGED = 0,
    TUTORIALDATA_CHANGED   = 1,
    TUTORIALDATA_NEW       = 2
};

enum BillingPlanFlags
{
    BILLING_FLAG_NONE         = 0x00,
    BILLING_FLAG_UNUSED       = 0x01,
    BILLING_FLAG_RECURRING    = 0x02,
    BILLING_FLAG_TRIAL        = 0x04,
    BILLING_FLAG_IGR          = 0x08,
    BILLING_FLAG_USAGE        = 0x10,
    BILLING_FLAG_TIME_MIXTURE = 0x20,
    BILLING_FLAG_RESTRICTED   = 0x40,
    BILLING_FLAG_ENABLE_CAIS  = 0x80
};

enum PlayTimeLimit : uint32
{
    PLAY_TIME_LIMIT_APPROACHING_PARTIAL = 2 * HOUR + 30 * MINUTE,
    PLAY_TIME_LIMIT_PARTIAL = 3 * HOUR,
    PLAY_TIME_LIMIT_APPROCHING_FULL = 4 * HOUR + 30 * MINUTE,
    PLAY_TIME_LIMIT_FULL = 5 * HOUR,
};

enum PlayTimeFlag : uint32
{
    PTF_APPROACHING_PARTIAL_PLAY_TIME = 0x1000,
    PTF_APPROACHING_NO_PLAY_TIME = 0x2000,
    PTF_UNHEALTHY_TIME = 0x80000000,
};

enum AntifloodOpcodeExecutionSpeed
{
    FLOOD_TOTAL_PACKETS = 0,
    FLOOD_SLOW_OPCODES,
    FLOOD_VERY_SLOW_OPCODES,
    FLOOD_MAX_OPCODES_TYPE,
};

enum AccountFlags
{
    ACCOUNT_FLAG_MUTED_FROM_PUBLIC_CHANNELS     = 0x1,
};

//class to deal with packet processing
//allows to determine if next packet is safe to be processed
class PacketFilter
{
    public:
        explicit PacketFilter(WorldSession* pSession) : m_pSession(pSession), m_processLogout(false), m_processType(PACKET_PROCESS_MAX_TYPE) {}
        virtual ~PacketFilter() {}

        virtual bool Process(std::unique_ptr<ClientPacket const> const&) { return true; }
        inline bool ProcessLogout() const { return m_processLogout; }
        inline PacketProcessing PacketProcessType() const { return m_processType; }
        inline void SetProcessType(PacketProcessing t) { m_processType = t; }

    protected:
        WorldSession* const m_pSession;
        bool m_processLogout;
        PacketProcessing m_processType;
};
//process only thread-safe packets in Map::Update()
class MapSessionFilter : public PacketFilter
{
    public:
        explicit MapSessionFilter(WorldSession* pSession) : PacketFilter(pSession)
        {
            m_processLogout = false;
            m_processType = PACKET_PROCESS_MAP;
        }
        ~MapSessionFilter() override {}

        bool Process(std::unique_ptr<ClientPacket const> const& packet) override;
};

//class used to filer only thread-unsafe packets from queue
//in order to update only be used in World::UpdateSessions()
class WorldSessionFilter : public PacketFilter
{
    public:
        explicit WorldSessionFilter(WorldSession* pSession) : PacketFilter(pSession)
        {
            m_processLogout = true;
            m_processType = PACKET_PROCESS_WORLD;
        }
        ~WorldSessionFilter() override {}
};

// Player session in the World
class WorldSession
{
    friend class CharacterHandler;
    public:
        WorldSession(uint32 id, std::shared_ptr<WorldSocket> sock, AccountTypes sec, time_t mute_time, LocaleConstant locale);
        ~WorldSession();

        uint32 GetGUID() const { return m_guid; }
        AccountTypes GetSecurity() const { return m_security; }
        uint32 GetAccountId() const { return m_accountId; }
        std::string GetUsername() const { return m_username; }
        void SetUsername(std::string const& s) { m_username = s; }
        uint32 GetLatency() const { return m_latency; }
        void SetLatency(uint32 latency) { m_latency = latency; }
        uint32 GetGameBuild() const { return m_gameBuild; }
        void SetGameBuild(uint32 v) { m_gameBuild = v; }
        ClientOSType GetOS() const { return m_clientOS; }
        void SetOS(ClientOSType os) { m_clientOS = os; }
        ClientPlatformType GetPlatform() const { return m_clientPlatform; }
        void SetPlatform(ClientPlatformType platform) { m_clientPlatform = platform; }
        uint32 GetDialogStatus(Player* pPlayer, Object* questgiver, uint32 defstatus);
        uint32 GetAccountMaxLevel() const { return m_characterMaxLevel; }
        void SetAccountFlags(uint32 f) { m_accountFlags = f; }
        uint32 GetAccountFlags() const { return m_accountFlags; }
        void SetVerifiedEmail(bool verified) { m_verifiedEmail = verified; }
        bool HasVerifiedEmail() const { return m_verifiedEmail; }
        bool HasTrialRestrictions() const;
        Player* GetPlayer() const { return _player; }
        char const* GetPlayerName() const;
        void SetSecurity(AccountTypes security) { m_security = security; }
        /// Might return "<BOT>" if player bot
        /// TODO rename me to GetRemoteIpString() when all the commits for native branch are done (otherwise too many files will be touched)
        std::string const& GetRemoteAddress() const { return m_remoteIpAddress; }
        void SetPlayer(Player* plr) { _player = plr; }
        void SetMasterPlayer(MasterPlayer* plr) { m_masterPlayer = plr; }
        void LoginPlayer(ObjectGuid playerGuid);
        std::shared_ptr<WorldSocket> GetSocket() { return m_socket; }

        // The Instance connection SMSG_CONNECT_TO redirects a session to (decision 5); null until
        // CMSG_AUTH_CONTINUED_SESSION attaches one.
        std::shared_ptr<WorldSocket> GetInstanceSocket() { return m_instanceSocket; }
        void SetInstanceSocket(std::shared_ptr<WorldSocket> socket);

        // The 40-byte world session key CMSG_AUTH_SESSION established on the Realm connection;
        // CMSG_AUTH_CONTINUED_SESSION's digest is checked against it rather than a fresh exchange.
        void SetWorldSessionKeyBytes(std::array<uint8, 40> const& key) { m_worldSessionKey = key; }
        std::array<uint8, 40> const& GetWorldSessionKeyBytes() const { return m_worldSessionKey; }

        // Resolves a CMSG_AUTH_CONTINUED_SESSION's key to the session SendConnectToInstance issued
        // it for. A synchronous, dedicated lookup rather than sWorld.FindSession(accountId): a
        // freshly authenticated session isn't visible there until the next world tick drains
        // World::AddSession's queue, which a same-host bot's round trip can easily beat.
        static WorldSession* FindPendingInstanceConnect(uint64 key);

        // Session in auth.queue currently
        void SetInQueue(bool state) { m_inQueue = state; }

        // Player online / socket offline system
        void SetDisconnectedSession(); // Remove from World::m_Session. Used when an account gets disconnected.
        bool UpdateDisconnected(uint32 diff);
        bool IsConnected() const { return m_connected; }
        void KickDisconnectedFromWorld() { m_disconnectTimer = 0; }

        bool PlayerLoading() const { return m_playerLoading; }
        bool PlayerLogout() const { return m_playerLogout; }
        bool PlayerLogoutWithSave() const { return m_playerLogout && m_playerSave; }

        bool CharacterScreenIdleKick(uint32 diff);
        uint32 m_idleTime;

        // Played time limit
        time_t GetCreateTime() const { return m_createTime; }
        time_t GetConsecutivePlayTime(time_t now) const { return (now - m_createTime) + m_previousPlayTime; }
        time_t GetPreviousPlayedTime() const { return m_previousPlayTime; }
        void SetPreviousPlayedTime(time_t playedTime) { m_previousPlayTime = playedTime; }
        void CheckPlayedTimeLimit(time_t now);
        void SendPlayTimeWarning(PlayTimeFlag flag, int32 timeLeftInSeconds);

        // Is the user engaged in a log out process?
        bool IsLogingOut() const { return m_logoutTime || m_playerLogout; }

        // Engage the logout process for the user
        void LogoutRequest(time_t requestTime)
        {
            m_logoutTime = requestTime;
        }

        // Is logout cooldown expired?
        bool ShouldLogOut(time_t currTime) const
        {
            return (m_logoutTime > 0 && currTime >= m_logoutTime + 20);
        }

        void LogoutPlayer(bool Save);
        void KickPlayer();

        // Session can be safely deleted if returns false
        bool ForcePlayerLogoutDelay();

        void DoLootRelease(ObjectGuid lguid);

        // Account mute time
        time_t m_muteTime;
        time_t m_lastPubChannelMsgTime;

        // Locales
        LocaleConstant GetSessionDbcLocale() const { return m_sessionDbcLocale; }
        int GetSessionDbLocaleIndex() const { return m_sessionDbLocaleIndex; }
        char const* GetMangosString(int32 entry) const;

        // Public chat cooldown restriction functionality
        // Intentionally session-based to avoid login/logout hijinks
        time_t GetLastPubChanMsgTime() const { return m_lastPubChannelMsgTime; }
        void SetLastPubChanMsgTime(time_t time) { m_lastPubChannelMsgTime = time; }

        // Bot system
        PlayerBotEntry* GetBot() const { return m_bot.get(); }
        void SetBot(std::shared_ptr<PlayerBotEntry> const& b) { m_bot = b; }

        // Anticheat
        void SetSessionKey(BigNumber const& sessionKey) { m_sessionKey = sessionKey; }
        void InitCheatData(Player* pPlayer);
        MovementAnticheat* GetCheatData();
        void ProcessAnticheatAction(char const* detector, char const* reason, uint32 cheatAction, uint32 banSeconds = 0 /* Perm ban */);
        uint32 GetFingerprint() const { return 0; } // TODO
        void CleanupFingerprintHistory() {} // TODO

        // Movement
        Unit* GetMoverFromGuid(ObjectGuid const& guid) const;
        ObjectGuid const& GetClientMoverGuid() const { return m_clientMoverGuid; }
        bool HasClientMovementControl() const { return !m_clientMoverGuid.IsEmpty(); }
        void RejectMovementPacketsFor(uint32 ms);

        void SetReceivedWhoRequest(bool v) { m_who_recvd = v; }
        bool ReceivedWhoRequest() const { return m_who_recvd; }
        bool m_who_recvd;

        void SetReceivedAHListRequest(bool v) { m_ah_list_recvd = v; }
        bool ReceivedAHListRequest() const { return m_ah_list_recvd; }
        bool m_ah_list_recvd;

        bool Update(PacketFilter& updater, uint32 diff = 0);
        void QueuePacket(std::unique_ptr<ClientPacket const> packet);
        void QueueBinaryPacket(std::unique_ptr<WorldPacket> const& binaryPacket);
        bool CanProcessPackets() const; // Returns true iif we can process packets (ie logged in Player, not a bot, etc ...
        void ProcessPackets(PacketFilter& updater);
        bool AllowPacket(uint32 opcode);
        void ClearIncomingPacketsByType(PacketProcessing type);
        inline bool HasRecentPacket(PacketProcessing type) const { return m_receivedPacketType[type]; }

        void StartSniffing()
        {
            if (!m_sniffFile)
            {
                std::string fileName = "packet_log_" + GetUsername() + "_" + std::to_string(time(nullptr)) + ".pkt";
                m_sniffFile = std::make_unique<SniffFile>(fileName.c_str());
                m_sniffFile->WriteHeader();
            }
        }
        void StopSniffing()
        {
            if (m_sniffFile)
                m_sniffFile.reset();
        }

    private:
        void SendPacketImpl(WorldPacket const* packet);

        // Issues a fresh ConnectToKey, sends SMSG_CONNECT_TO for it (serial = the attempt number,
        // decision 5's escalation), and registers the key for FindPendingInstanceConnect.
        void SendConnectToInstance(uint32 serial);
        // Drops this session's pending-instance-connect registration, if any: attached, given up
        // on, or the session itself going away.
        void ClearPendingInstanceConnect();

        // Kicks off the periodic SMSG_TIME_SYNC_REQUEST cadence (first right after
        // SMSG_LOGIN_VERIFY_WORLD, then every 10s); HandleClassicTimeSyncResponseOpcode pairs the
        // client's CMSG_TIME_SYNC_RESPONSE back to it by sequence index.
        void SendTimeSync();

        static void VerifyPacketWasCorrectlyRead(WorldPacket const& recvPacket, ClientPacket const& clientPacket);

    public:
        /// Sends a packet to the client.
        // Every legacy-opcode packet a session tried to send since startup, by opcode with its count,
        // most frequent first: what is left to port.
        static std::vector<std::pair<uint32, uint64>> GetUnportedSendCounts();
        void SendPacket(std::unique_ptr<ServerPacket const> packet);
        /// @deprecated Use SendPacket with ServerPacket class
        void SendPacket(WorldPacket const* packet);

        // Sends a Classic* wire struct (Packets/Classic/*.h): writes T::Opcode and T::Write, then
        // goes through the same path as any other SendPacket (legacy-lane check included, though a
        // classic opcode never is one).
        template<typename T>
        void SendClassic(T const& packet)
        {
            WorldPacket data = MakeClassicPacket(packet);
            SendPacket(&data);
        }
        void SendMovementPacket(WorldPacket const* packet);
        // Makes `mover` the unit this session's client moves, and tells the client.
        void SendActiveMover(Unit const* mover);
        void SendNotification(char const* format, ...) ATTR_PRINTF(2, 3);
        void SendNotification(int32 string_id, ...);
        void SendPetNameInvalid(uint32 error, ClassicGuid const& pet, int32 petNumber, std::string const& name);
        void SendPartyResult(PartyOperation operation, std::string const& memberName, PartyResult res);
        void SendAreaTriggerMessage(char const* Text, ...) ATTR_PRINTF(2, 3);
        void SendQueryTimeResponse();

        // Handle the authentication waiting queue (to be completed)
        void SendAuthWaitQue(uint32 position);

        void SendNameQueryOpcode(Player* p);

        // Trainer
        void SendTrainerList(ObjectGuid guid);
        void SendTrainerList(ObjectGuid guid, std::string const& strTitle);
        void SendTrainingFailure(ObjectGuid guid, uint32 serviceId, uint32 errorCode);

        // NPC
        void SendListInventory(ObjectGuid guid, uint8 menu_type = VENDOR_MENU_ALL);
        bool CheckBanker(ObjectGuid guid);
        void SendShowBank(ObjectGuid guid);
        void SendTabardVendorActivate(ObjectGuid guid);
        void SendSpiritResurrect();
        void SendBindPoint(Creature* npc);
        void SendAttackStop(Unit const* enemy);

        // Mail
        bool CheckMailBox(ObjectGuid guid);
        void SendMailResult(uint32 mailId, MailResponseType mailAction, MailResponseResult mailError, uint32 equipError = 0, uint32 item_guid = 0, uint32 item_count = 0);
        void SendNewMail();

        // Trade
        void SendTradeStatus(TradeStatus status);
        void SendUpdateTrade(bool trader_state = true);
        void SendCancelTrade(TradeStatus status);

        // Pet
        void SendPetNameQuery(ObjectGuid guid);
        void SendStablePet(ObjectGuid guid);
        void SendStableResult(uint8 res);
        bool CheckStableMaster(ObjectGuid guid);

        // Account Data
        AccountData* GetAccountData(NewAccountData::AccountDataType type) { return &m_accountData[type]; }
        void SetAccountData(NewAccountData::AccountDataType type, const std::string& data);
        void SendAccountDataTimes();
        void LoadGlobalAccountData();
        void LoadAccountData(std::unique_ptr<QueryResult> result, uint32 mask);

        void LoadTutorialsData();
        void SendTutorialsData();
        void SaveTutorialsData();
        uint32 GetTutorialInt(uint32 intId) const;
        void SetTutorialInt(uint32 intId, uint32 value)
        {
            if (m_tutorials[intId] != value)
            {
                m_tutorials[intId] = value;
                if (m_tutorialState == TUTORIALDATA_UNCHANGED)
                    m_tutorialState = TUTORIALDATA_CHANGED;
            }
        }

        // Auction
        void SendAuctionHello(Unit* unit);
        void SendAuctionCommandResult(AuctionEntry* auc, AuctionAction Action, AuctionError ErrorCode, InventoryResult invError = EQUIP_ERR_OK);
        void SendAuctionBidderNotification(AuctionEntry* auction, bool won);
        void SendAuctionOwnerBidNotification(AuctionEntry* auction);
        void SendAuctionOwnerNotification(AuctionEntry* auction, bool sold);
        void SendAuctionRemovedNotification(AuctionEntry* auction);
        void SendAuctionOutbiddedMail(AuctionEntry* auction);
        void SendAuctionCancelledToBidderMail(AuctionEntry* auction);
        AuctionHouseEntry const* GetCheckedAuctionHouseForAuctioneer(ObjectGuid guid);

        // Item Enchantment
        void SendItemEnchantTimeUpdate(ObjectGuid playerGuid, ObjectGuid itemGuid, uint32 slot, uint32 duration);

        // Taxi
        void SendTaxiStatus(ObjectGuid guid);
        void SendTaxiMenu(Creature* unit);
        void SendDoFlight(uint32 mountDisplayId, uint32 path, uint32 pathNode = 0);
        bool SendLearnNewTaxiNode(Creature* unit);
        void SendActivateTaxiReply(uint32 replyCode);

        // Guild Team
        void SendGuildCommandResult(uint32 typecmd, std::string const& str, uint32 cmdresult);
        void SendPetitionShowList(ObjectGuid guid);
        void SendSaveGuildEmblem(uint32 msg);

        // Battleground
        void SendBattleGroundJoinError(uint8 err);
        void SendBattleGroundList(ObjectGuid guid, BattleGroundTypeId bgTypeId);

        // Meetingstone
        void SendMeetingstoneFailed(uint8 status);
        void SendMeetingstoneSetqueue(uint32 areaid, uint8 status);

    public:                                                 // opcodes handlers
        template<typename TClientPacket>
        static std::unique_ptr<ClientPacket> Handle_GenericRead(WorldPacket& recvPacket)
        {
            auto packet = std::make_unique<TClientPacket>();
            packet->ReadFromWorldPacket(recvPacket);
            VerifyPacketWasCorrectlyRead(recvPacket, *packet);
            return packet;
        }

        template<typename TClientPacket, void (WorldSession::*THandler)(TClientPacket const& packet)>
        void Handle_GenericPacket(ClientPacket const& packet)
        {
            auto const& packetInCorrectForm = dynamic_cast<TClientPacket const&>(packet);
            (this->*THandler)(packetInCorrectForm);
        }

        // Same shape as Handle_GenericRead/Handle_GenericPacket, but for a classic wire struct: T's
        // own Read reports success with a bool instead of always succeeding, so a malformed body
        // comes back as nullptr rather than a packet VerifyPacketWasCorrectlyRead would misjudge.
        template<typename T>
        static std::unique_ptr<ClientPacket> Handle_ClassicRead(WorldPacket& recvPacket)
        {
            auto packet = std::make_unique<ClassicClient<T>>();
            packet->ReadFromWorldPacket(recvPacket);
            if (!packet->IsValid())
                return nullptr;
            VerifyPacketWasCorrectlyRead(recvPacket, *packet);
            return packet;
        }

        template<typename T, void (WorldSession::*THandler)(T const& packet)>
        void Handle_ClassicPacket(ClientPacket const& packet)
        {
            (this->*THandler)(dynamic_cast<ClassicClient<T> const&>(packet).Body);
        }

        void Handle_NULL(WorldPacket& recvPacket);          // not used
        void Handle_EarlyProccess(WorldPacket& recvPacket);// just mark packets processed in WorldSocket::OnRead
        void Handle_ServerSide(WorldPacket& recvPacket);    // sever side only, can't be accepted from client

        void HandleCharEnumOpcode(NullClientPacket const& packet);
        void HandleCharEnum(std::unique_ptr<QueryResult> result);
        void HandlePlayerLogin(LoginQueryHolder* holder);

        // Classic (modern-client) character screen; HandleCharEnumOpcode above stays for its own
        // legacy opcode, which nothing sends anymore but which still compiles (the legacy lane).
        void HandleClassicCharEnumOpcode(ClassicEnumCharacters const& packet);
        void HandleCharCreateOpcode(CharCreate const& packet);
        void HandleCharDeleteOpcode(CharDelete const& packet);
        void HandlePlayerLoginOpcode(PlayerLogin const& packet);
        void HandleClassicConnectToFailedOpcode(ConnectToFailed const& packet);
        void HandleClassicLogoutRequestOpcode(ClassicLogoutRequest const& packet);
        void HandleClassicTimeSyncResponseOpcode(TimeSyncResponse const& packet);
        void HandlePlayedTime(RequestPlayedTime const& packet);

        // Movement
        void HandleMoveRootAck(ClassicOpcodes opcode, ClassicMovementAck const& ack);
        template<ClassicOpcodes Op>
        void HandleClassicRootAck(ClientMovementAck<Op> const& packet) { HandleMoveRootAck(Op, packet.Ack); }
        void HandleMoveKnockBackAck(MoveKnockBackAck const& packet);
        void HandleMoveTeleportAckOpcode(MoveTeleportAck const& packet);
        void HandleForceSpeedChangeAck(ClassicOpcodes opcode, ClassicMovementAck const& ack, float speed);
        template<ClassicOpcodes Op>
        void HandleClassicSpeedAck(ClientMovementSpeedAck<Op> const& packet) { HandleForceSpeedChangeAck(Op, packet.Ack, packet.Speed); }
        void HandleMoveWorldportAckOpcode(WorldPortResponse const& packet);
        void HandleMoveWorldportAck();                // for server-side calls
        void HandleMovementOpcodes(uint32 opcode, MovementInfo& movementInfo);
        void HandleClassicMovement(ClassicOpcodes opcode, ClassicMovementInfo const& status);
        template<ClassicOpcodes Op>
        void HandleClassicMovementOpcode(ClientPlayerMovement<Op> const& packet) { HandleClassicMovement(Op, packet.Status); }
        void HandleSetActiveMoverOpcode(SetActiveMover const& packet);
        void HandleMoveInitActiveMoverCompleteOpcode(MoveInitActiveMoverComplete const& packet);
        void HandleMoveTimeSkippedOpcode(MoveTimeSkipped const& packet);
        void HandleMovementFlagChangeToggleAck(ClassicOpcodes opcode, ClassicMovementAck const& ack);
        template<ClassicOpcodes Op>
        void HandleClassicFlagAck(ClientMovementAck<Op> const& packet) { HandleMovementFlagChangeToggleAck(Op, packet.Ack); }
        void HandleMoveSplineDoneOpcode(MoveSplineDone const& packet);
        void HandleWorldTeleportOpcode(WorldPackets::Misc::WorldTeleport const& packet);
        void HandleMountSpecialAnimOpcode(MountSpecial const& packet);
        void HandleTeleportToUnitOpcode(WorldPackets::Misc::TeleportToUnit const& packet);

        void HandleInspectOpcode(Inspect const& packet);
        void HandleInspectPvpOpcode(InspectPvp const& packet);
        void HandleQueryInspectAchievementsOpcode(QueryInspectAchievements const& packet);
        void HandleShowingHelmOpcode(NullClientPacket const& packet);
        void HandleShowingCloakOpcode(NullClientPacket const& packet);
        void HandleRepairItemOpcode(RepairItem const& packet);

        void HandlePingOpcode(WorldPacket& recvPacket);
        void HandleAuthSessionOpcode(WorldPacket& recvPacket);
        void HandleRepopRequestOpcode(RepopRequest const& packet);
        void HandleAutostoreLootItemOpcode(LootItemRequest const& packet);
        void AutostoreLootSlot(uint32 lootSlot);
        void HandleLootMoneyOpcode(LootMoney const& packet);
        void HandleLootOpcode(LootUnit const& packet);
        void HandleLootReleaseOpcode(LootRelease const& packet);
        void HandleLootMasterGiveOpcode(MasterLootItem const& packet);
        void MasterLootSlot(ObjectGuid lootGuid, uint8 slotId, ObjectGuid playerGuid);
        void HandleWhoOpcode(WorldPackets::Misc::Who const& packet);
        void HandleLFGOpcode(NullClientPacket const& packet);
        void HandleLogoutRequestOpcode(NullClientPacket const& packet);
        void HandlePlayerLogoutOpcode(NullClientPacket const& packet);
        void HandleLogoutCancelOpcode(NullClientPacket const& packet);
        void HandleGMTicketGetSystemStatusOpcode(GetSystemStatus const& packet);
        void HandleGMTicketGetCaseStatusOpcode(GetCaseStatus const& packet);
        void HandleGMTicketAcknowledgeSurveyOpcode(AcknowledgeSurvey const& packet);
        void HandleGMTicketSubmitUserFeedbackOpcode(SubmitUserFeedback const& packet);
        void HandleGMTicketComplaintOpcode(Complaint const& packet);
        void HandleGMTicketBugReportOpcode(BugReport const& packet);

        void HandleSetPvP(SetPvP const& packet);
        void HandleZoneUpdateOpcode(WorldPackets::Misc::ZoneUpdate const& packet);
        void HandleSetSelectionOpcode(SetSelection const& packet);
        void HandleSetMouseOverTargetOpcode(WorldPackets::Misc::SetTarget const& packet);
        void HandleStandStateChangeOpcode(StandStateChange const& packet);
        void HandleEmoteOpcode(EmoteRequest const& packet);
        void HandleFriendListOpcode(SendContactList const& packet);
        void HandleSetContactNotesOpcode(SetContactNotes const& packet);
        void HandleAddFriendOpcode(AddFriend const& packet);
        void HandleDelFriendOpcode(DelFriend const& packet);
        void HandleAddIgnoreOpcode(AddIgnore const& packet);
        void HandleDelIgnoreOpcode(DelIgnore const& packet);
        void HandleBugOpcode(WorldPackets::Misc::Bug const& packet);
        void HandleSetAmmoOpcode(SetAmmo const& packet);
        void HandleItemNameQueryOpcode(WorldPackets::Query::ItemNameQuery const& packet);

        void HandleAreaTriggerOpcode(AreaTrigger const& packet);
        void HandleSetFactionAtWarOpcode(SetFactionAtWar const& packet);
        void HandleSetFactionNotAtWarOpcode(SetFactionNotAtWar const& packet);
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_9_4
        void HandleSetWatchedFactionOpcode(SetWatchedFaction const& packet);
        void HandleSetFactionInactiveOpcode(SetFactionInactive const& packet);
#endif

        void HandleUpdateAccountData(AccountDataUpdate const& packet);
        void HandleRequestAccountData(AccountDataRequest const& packet);
        void HandleSetActionButtonOpcode(SetActionButton const& packet);

        void HandleGameObjectUseOpcode(GameObjectUse const& packet);
        void HandleMeetingStoneJoinOpcode(WorldPackets::Misc::MeetingStoneJoin const& packet);
        void HandleMeetingStoneLeaveOpcode(NullClientPacket const& packet);
        void HandleMeetingStoneInfoOpcode(NullClientPacket const& packet);

        void HandleQueryPlayerNamesOpcode(QueryPlayerNames const& packet);
        void HandleQueryTimeOpcode(QueryTime const& packet);
        void HandleCreatureQueryOpcode(QueryCreature const& packet);
        void HandleGameObjectQueryOpcode(QueryGameObject const& packet);

        void HandleRequestRaidInfoOpcode(NullClientPacket const& packet);
        void HandlePartyInvite(PartyInviteClient const& packet);
        void HandlePartyInviteResponse(PartyInviteResponse const& packet);
        void HandlePartyUninvite(PartyUninvite const& packet);
        void HandleLeaveGroup(LeaveGroup const& packet);
        void HandleSetPartyLeader(SetPartyLeader const& packet);
        void HandleSetAssistantLeader(SetAssistantLeader const& packet);
        void HandleSetPartyAssignment(SetPartyAssignment const& packet);
        void HandleSetLootMethod(SetLootMethod const& packet);
        void HandleLootRoll(LootRoll const& packet);
        void HandleRequestPartyMemberStats(RequestPartyMemberStats const& packet);
        void HandleRequestPartyJoinUpdates(RequestPartyJoinUpdates const& packet);
        void HandleUpdateRaidTarget(UpdateRaidTarget const& packet);
        void HandleDoReadyCheck(DoReadyCheck const& packet);
        void HandleReadyCheckResponse(ReadyCheckResponseClient const& packet);
        void HandleConvertRaid(ConvertRaid const& packet);
        void HandleChangeSubGroup(ChangeSubGroup const& packet);
        void HandleSwapSubGroups(SwapSubGroups const& packet);

        void HandlePetitionBuyOpcode(PetitionBuy const& packet);
        void HandlePetitionShowSignOpcode(PetitionShowSignatures const& packet);
        void HandlePetitionQueryOpcode(QueryPetition const& packet);
        void HandlePetitionRenameOpcode(RenamePetition const& packet);
        void HandlePetitionSignOpcode(SignPetition const& packet);
        void HandlePetitionDeclineOpcode(DeclinePetition const& packet);
        void HandleOfferPetitionOpcode(OfferPetition const& packet);
        void HandleTurnInPetitionOpcode(TurnInPetition const& packet);

        void HandleGuildQueryOpcode(GuildQuery const& packet);
        void HandleGuildInviteOpcode(GuildInviteByName const& packet);
        void HandleGuildRemoveOpcode(GuildOfficerRemoveMember const& packet);
        void HandleGuildAcceptOpcode(AcceptGuildInvite const& packet);
        void HandleGuildDeclineOpcode(GuildDeclineInvitation const& packet);
        void HandleGuildRosterOpcode(GuildGetRoster const& packet);
        void HandleGuildGetRanksOpcode(GuildGetRanks const& packet);
        void HandleGuildPermissionsQueryOpcode(GuildPermissionsQuery const& packet);
        void HandleGuildPromoteOpcode(GuildPromoteMember const& packet);
        void HandleGuildDemoteOpcode(GuildDemoteMember const& packet);
        void HandleGuildLeaveOpcode(GuildLeave const& packet);
        void HandleGuildDisbandOpcode(GuildDelete const& packet);
        void HandleGuildLeaderOpcode(GuildSetGuildMaster const& packet);
        void HandleGuildMOTDOpcode(GuildUpdateMotdText const& packet);
        void HandleGuildSetMemberNoteOpcode(GuildSetMemberNote const& packet);
        void HandleGuildRankOpcode(GuildSetRankPermissions const& packet);
        void HandleGuildAddRankOpcode(GuildAddRank const& packet);
        void HandleGuildDelRankOpcode(GuildDeleteRank const& packet);
        void HandleGuildChangeInfoTextOpcode(GuildUpdateInfoText const& packet);
        void HandleSaveGuildEmblemOpcode(SaveGuildEmblem const& packet);

        void HandleTaxiNodeStatusQueryOpcode(TaxiNodeStatusQuery const& packet);
        void HandleTaxiQueryAvailableNodes(TaxiQueryAvailableNodes const& packet);
        void HandleEnableTaxiNodeOpcode(EnableTaxiNode const& packet);
        void HandleActivateTaxiOpcode(ActivateTaxi const& packet);

        void HandleTabardVendorActivateOpcode(TabardVendorActivate const& packet);
        void HandleBankerActivateOpcode(BankerActivate const& packet);
        void HandleBuyBankSlotOpcode(BuyBankSlot const& packet);
        void HandleTrainerListOpcode(TrainerList const& packet);
        void HandleTrainerBuySpellOpcode(TrainerBuySpell const& packet);
        void HandlePetitionShowListOpcode(PetitionShow const& packet);
        void HandleGossipHelloOpcode(GossipHello const& packet);
        void HandleGossipSelectOptionOpcode(GossipSelectOption const& packet);
        void HandleCloseInteractionOpcode(CloseInteraction const& packet);
        void HandleSpiritHealerActivateOpcode(SpiritHealerActivate const& packet);
        void HandleNpcTextQueryOpcode(QueryNpcText const& packet);
        void HandleBinderActivateOpcode(BinderActivate const& packet);
        void HandleListStabledPetsOpcode(WorldPackets::Npc::ListStabledPets const& packet);
        void HandleStablePet(WorldPackets::Npc::StablePet const& packet);
        void HandleUnstablePet(WorldPackets::Npc::UnstablePet const& packet);
        void HandleBuyStableSlot(WorldPackets::Npc::BuyStableSlot const& packet);
        void HandleStableRevivePet(NullClientPacket const& packet);
        void HandleStableSwapPet(WorldPackets::Npc::StableSwapPet const& packet);

        void HandleCanDuelOpcode(CanDuel const& packet);
        void HandleDuelResponseOpcode(DuelResponse const& packet);
        void HandleDuelAccepted(ClassicGuid const& arbiter);
        void HandleDuelCancelled();

        void HandleAcceptTradeOpcode(AcceptTrade const& packet);
        void HandleBeginTradeOpcode(BeginTrade const& packet);
        void HandleBusyTradeOpcode(BusyTrade const& packet);
        void HandleCancelTradeOpcode(CancelTrade const& packet);
        void HandleClearTradeItemOpcode(ClearTradeItem const& packet);
        void HandleIgnoreTradeOpcode(IgnoreTrade const& packet);
        void HandleInitiateTradeOpcode(InitiateTrade const& packet);
        void HandleSetTradeGoldOpcode(SetTradeGold const& packet);
        void HandleSetTradeItemOpcode(SetTradeItem const& packet);
        void HandleUnacceptTradeOpcode(UnacceptTrade const& packet);

        void HandleAuctionHelloOpcode(AuctionHelloRequest const& packet);
        void HandleAuctionListItems(AuctionListItems const& packet);
        void HandleAuctionListBidderItems(AuctionListBiddedItems const& packet);
        void HandleAuctionSellItem(AuctionSellItem const& packet);
        void HandleAuctionRemoveItem(AuctionRemoveItem const& packet);
        void HandleAuctionListOwnerItems(AuctionListOwnedItems const& packet);
        void HandleAuctionPlaceBid(AuctionPlaceBid const& packet);
        void HandleAuctionListPendingSales(AuctionListPendingSales const& packet);

        void HandleGetMailListOpcode(GetMailList const& packet);
        void HandleSendMailOpcode(SendMail const& packet);
        class AsyncMailSendRequest;
        void HandleSendMailRequest(AsyncMailSendRequest* req);
        void HandleSendMailCallback(AsyncMailSendRequest* req);
        void HandleMailTakeMoneyOpcode(MailTakeMoney const& packet);
        void HandleMailTakeItemOpcode(MailTakeItem const& packet);
        void HandleMailMarkAsReadOpcode(MailMarkAsRead const& packet);
        void HandleMailReturnToSenderOpcode(MailReturnToSender const& packet);
        void HandleMailDeleteOpcode(MailDelete const& packet);
        void HandleItemTextQueryOpcode(ItemTextQuery const& packet);
        void HandleMailCreateTextItemOpcode(MailCreateTextItem const& packet);
        void HandleQueryNextMailTimeOpcode(QueryNextMailTime const& packet);
        void HandleCancelChanneling(CancelChannelling const& packet);

        void HandleSplitItemOpcode(SplitItem const& packet);
        void HandleSwapInvItemOpcode(SwapInvItem const& packet);
        void HandleDestroyItemOpcode(DestroyItem const& packet);
        void HandleAutoEquipItemOpcode(AutoEquipItem const& packet);
        void HandleItemQuerySingleOpcode(WorldPackets::Item::QueryItem const& packet);
        void HandleSellItemOpcode(SellItem const& packet);
        void HandleBuyItemOpcode(BuyItem const& packet);
        void HandleListInventoryOpcode(ListInventory const& packet);
        void HandleAutoStoreBagItemOpcode(AutoStoreBagItem const& packet);
        void HandleReadItemOpcode(ReadItem const& packet);
        void HandleAutoEquipItemSlotOpcode(AutoEquipItemSlot const& packet);
        void HandleSwapItem(SwapItem const& packet);
        void HandleBuybackItem(BuyBackItem const& packet);
        void HandleAutoBankItemOpcode(AutoBankItem const& packet);
        void HandleAutoStoreBankItemOpcode(AutoStoreBankItem const& packet);
        void HandleWrapItemOpcode(WorldPackets::Item::WrapItem const& packet);

        void HandleAttackSwingOpcode(AttackSwing const& packet);
        void HandleAttackStopOpcode(AttackStop const& packet);
        void HandleSetSheathedOpcode(SetSheathed const& packet);

        void HandleUseItemOpcode(UseItem const& packet);
        void HandleOpenItemOpcode(WorldPackets::Spell::OpenItem const& packet);
        void HandleCastSpellOpcode(CastSpell const& packet);
        void HandleCancelCastOpcode(CancelCast const& packet);
        void HandleCancelAuraOpcode(CancelAura const& packet);
        void HandleCancelGrowthAuraOpcode(CancelGrowthAura const& packet);
        void HandleCancelAutoRepeatSpellOpcode(CancelAutoRepeatSpell const& packet);
        void HandleSetActionBarTogglesOpcode(SetActionBarToggles const& packet);
        void HandleLearnTalentOpcode(LearnTalent const& packet);
        void HandleTalentWipeConfirmOpcode(WorldPackets::Skill::TalentWipeConfirm const& packet);
        void HandleUnlearnSkillOpcode(UnlearnSkill const& packet);

        void HandleQuestgiverStatusQueryOpcode(QuestgiverStatusQuery const& packet);
        void HandleQuestgiverHelloOpcode(QuestgiverHello const& packet);
        void HandleQuestgiverAcceptQuestOpcode(QuestgiverAcceptQuest const& packet);
        void HandleQuestgiverQueryQuestOpcode(QuestgiverQueryQuest const& packet);
        void HandleQuestgiverChooseRewardOpcode(QuestgiverChooseReward const& packet);
        void HandleQuestgiverRequestRewardOpcode(QuestgiverRequestReward const& packet);
        void HandleQuestgiverStatusMultipleQuery(QuestGiverStatusMultipleQuery const& packet);
        void HandleQueryQuestInfoOpcode(QueryQuestInfo const& packet);
        uint8 GetQuestGiverStatus(Object* questgiver);
        void HandleQuestgiverCancel(QuestGiverCloseQuest const& packet);
        void HandleQuestLogRemoveQuest(QuestLogRemoveQuest const& packet);
        void HandleQuestConfirmAccept(QuestConfirmAccept const& packet);
        void HandleQuestgiverCompleteQuest(QuestgiverCompleteQuest const& packet);
        void HandlePushQuestToParty(PushQuestToParty const& packet);
        void HandleQuestPushResult(QuestPushResult const& packet);

        bool SanitizeChatMessage(std::string& msg, uint32 lang, uint32 msgType);
        bool SanitizeChatMessageAndProcessCommand(std::string& msg, uint32 lang, uint32 msgType);
        static bool IsLanguageAllowedForChatType(uint32 lang, uint32 msgType);
        void SendPlayerNotFoundNotice(std::string const& name);
        void SendWrongFactionNotice();
        void SendChatRestrictedNotice();
        void HandleChatMessageOpcode(WorldPackets::Chat::ChatMessage const& packet);
        // Thin wrappers over HandleChatMessageOpcode, one per modern CMSG_CHAT_MESSAGE_* opcode:
        // each just normalizes its own wire shape into a ChatMessage and forwards.
        void HandleChatMessageSayOpcode(ChatMessageSay const& packet);
        void HandleChatMessageYellOpcode(ChatMessageYell const& packet);
        void HandleChatMessageEmoteOpcode(ChatMessageEmote const& packet);
        void HandleChatMessageWhisperOpcode(ChatMessageWhisper const& packet);
        void HandleChatMessageChannelOpcode(ChatMessageChannel const& packet);
        void HandleChatMessageGuildOpcode(ChatMessageGuild const& packet);
        void HandleChatMessageOfficerOpcode(ChatMessageOfficer const& packet);
        void HandleChatMessagePartyOpcode(ChatMessageParty const& packet);
        void HandleChatMessageRaidOpcode(ChatMessageRaid const& packet);
        void HandleChatMessageRaidWarningOpcode(ChatMessageRaidWarning const& packet);
        void HandleChatMessageInstanceChatOpcode(ChatMessageInstanceChat const& packet);
        void HandleChatMessageAfkOpcode(ChatMessageAfk const& packet);
        void HandleChatMessageDndOpcode(ChatMessageDnd const& packet);
        void HandleTextEmoteOpcode(TextEmoteRequest const& packet);
        void HandleChatIgnoredOpcode(WorldPackets::Misc::ChatIgnored const& packet);
        uint32 ChatCooldown();

        void HandleReclaimCorpseOpcode(ReclaimCorpse const& packet);
        void HandleCorpseQueryOpcode(QueryCorpseLocationFromClient const& packet);
        void HandleResurrectResponseOpcode(ResurrectResponse const& packet);
        void HandleSummonResponseOpcode(SummonResponse const& packet);

        void HandleJoinChannelOpcode(JoinChannel const& packet);
        void HandleLeaveChannelOpcode(LeaveChannel const& packet);
        void HandleChannelListOpcode(ChannelList const& packet);
        void HandleChannelDisplayListOpcode(ChannelDisplayList const& packet);
        void HandleChannelPasswordOpcode(ChannelPassword const& packet);
        void HandleChannelSetOwnerOpcode(ChannelSetOwner const& packet);
        void HandleChannelOwnerOpcode(ChannelOwner const& packet);
        void HandleChannelModeratorOpcode(ChannelModerator const& packet);
        void HandleChannelUnmoderatorOpcode(ChannelUnmoderator const& packet);
        void HandleChannelInviteOpcode(ChannelInvite const& packet);
        void HandleChannelKickOpcode(ChannelKick const& packet);
        void HandleChannelBanOpcode(ChannelBan const& packet);
        void HandleChannelUnbanOpcode(ChannelUnban const& packet);
        void HandleChannelAnnouncementsOpcode(ChannelAnnouncements const& packet);
        void HandleChannelDeclineInviteOpcode(ChannelDeclineInvite const& packet);
        void HandleChannelSilenceAllOpcode(ChannelSilenceAll const& packet);
        void HandleChannelUnsilenceAllOpcode(ChannelUnsilenceAll const& packet);

        void HandleCompleteCinematic(NullClientPacket const& packet);
        void HandleNextCinematicCamera(NullClientPacket const& packet);

        void HandlePageTextQueryOpcode(QueryPageText const& packet);

        void HandleTutorialFlagOpcode(TutorialRequest const& packet);

        //Pet
        void HandlePetAction(PetAction const& packet);
        void HandlePetNameQueryOpcode(QueryPetName const& packet);
        void HandlePetSetAction(PetSetAction const& packet);
        void HandlePetAbandon(PetAbandon const& packet);
        void HandlePetRename(PetRename const& packet);
        void HandlePetCancelAuraOpcode(PetCancelAura const& packet);
        void HandleRequestPetInfoOpcode(RequestPetInfo const& packet);
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_6_1
        void HandlePetStopAttack(PetStopAttack const& packet);
        void HandlePetSpellAutocastOpcode(PetSpellAutocast const& packet);
#endif
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_8_4
        void HandlePetCastSpellOpcode(PetCastSpell const& packet);
#endif

        void HandleCharRenameOpcode(WorldPackets::Character::CharRename const& packet);
        static void HandleChangePlayerNameOpcodeCallBack(std::unique_ptr<QueryResult> result, uint32 accountId, std::string newname);

        //BattleGround
        void HandleBattlemasterHelloOpcode(BattlemasterHello const& packet);
        void HandleBattlemasterJoinOpcode(BattlemasterJoin const& packet);
        void RequestBgJoinQueue(ObjectGuid battlemaster, uint32 instanceId, BattleGroundTypeId bgTypeId, bool joinAsGroup);
        void HandlePVPLogDataOpcode(PVPLogDataRequest const& packet);
        void HandleRequestBattlefieldStatusOpcode(RequestBattlefieldStatus const& packet);
        void HandleBattleFieldPortOpcode(BattleFieldPort const& packet);
        void HandleBattlefieldListOpcode(BattlefieldListRequest const& packet);
        void HandleLeaveBattlefieldOpcode(LeaveBattlefield const& packet);
        void HandleGetPVPOptionsEnabledOpcode(GetPVPOptionsEnabled const& packet);
        void HandleRequestRatedPvpInfoOpcode(RequestRatedPvpInfo const& packet);
        void HandleReportPvPAFKOpcode(ReportPvPPlayerAFK const& packet);

        void HandleMinimapPing(MinimapPingClient const& packet);
        void HandleRandomRoll(RandomRollClient const& packet);
        void HandleFarSightOpcode(FarSightRequest const& packet);
        void HandleWhoisOpcode(WhoIsRequest const& packet);
        void HandleResetInstancesOpcode(NullClientPacket const& packet);

        void HandleAreaSpiritHealerQueryOpcode(AreaSpiritHealerQuery const& packet);
        void HandleAreaSpiritHealerQueueOpcode(AreaSpiritHealerQueue const& packet);
        void HandleSelfResOpcode(SelfRes const& packet);

    private:
        // private trade method
        void MoveItems(Item* myItems[], Item* hisItems[]);

        bool VerifyMovementInfo(MovementInfo const& movementInfo) const;
        void HandleMoverRelocation(Unit* pMover, MovementInfo& movementInfo);

        void ExecuteOpcode(OpcodeHandlerPacketImplDetails const& opHandle, ClientPacket const& packet);

        // logging helper
        void LogUnexpectedOpcode(ClientPacket const& packet, std::string const& reason);

        uint32 const m_guid; // unique identifier for each session
        std::shared_ptr<WorldSocket> m_socket;
        std::shared_ptr<WorldSocket> m_instanceSocket; // SMSG_CONNECT_TO's target; null until attached
        std::array<uint8, 40> m_worldSessionKey{}; // set once, by CMSG_AUTH_SESSION on the Realm connection
        uint64 m_pendingInstanceConnectKey = 0; // the most recently issued SMSG_CONNECT_TO's key
        uint32 m_pendingInstanceConnectSerial = 0; // ... and the attempt serial it went out with
        bool m_awaitingInstanceConnect = false; // true from the first SMSG_CONNECT_TO until one lands
        std::string m_remoteIpAddress; // might also be "<BOT>"
        LockedQueue<std::unique_ptr<ClientPacket const>, std::mutex> m_recvQueue[PACKET_PROCESS_MAX_TYPE];
        bool m_receivedPacketType[PACKET_PROCESS_MAX_TYPE];
        uint32 m_floodPacketsCount[FLOOD_MAX_OPCODES_TYPE];
        bool m_connected;
        uint32 m_disconnectTimer;
        uint32 m_latency;

        // Sent index -> WorldTimer::getMSTime() at send, until the matching CMSG_TIME_SYNC_RESPONSE
        // arrives.
        std::map<uint32, uint32> m_pendingTimeSyncRequests;
        uint32 m_timeSyncNextIndex = 0;
        uint32 m_timeSyncTimer = 0; // ms until the next SMSG_TIME_SYNC_REQUEST; 0 == not running yet
        int32 m_timeSyncClockDeltaMs = 0; // latest server-minus-client delta; not consumed yet

        uint32 m_accountId;
        std::string m_username;
        AccountTypes m_security;
        uint32 m_accountFlags;
        LocaleConstant m_sessionDbcLocale;
        int m_sessionDbLocaleIndex;
        ClientOSType m_clientOS;
        ClientPlatformType m_clientPlatform;
        uint32 m_gameBuild;
        bool m_verifiedEmail;
        std::shared_ptr<PlayerBotEntry> m_bot;
        std::unique_ptr<SniffFile> m_sniffFile;

        MovementAnticheat* m_cheatData;

        Player* _player;
        ObjectGuid m_currentPlayerGuid;
        // The mailbox last opened via a successful GetMailList -- the modern wire drops the
        // mailbox guid from MailReturnToSender, unlike every other mail opcode, so there is nothing
        // to re-check proximity against without caching it here.
        ObjectGuid m_currentMailboxGuid;
        ObjectGuid m_clientMoverGuid;
        uint32 m_moveRejectTime;
        time_t m_createTime;                                // when session was created
        time_t m_previousPlayTime;                          // play time from previous session less than 5 hours ago
        time_t m_logoutTime;                                // when its time to log out character
        bool m_inQueue;                                     // session wait in auth.queue
        bool m_playerLoading;                               // code processed in LoginPlayer
        bool m_playerLogout;                                // code processed in LogoutPlayer
        bool m_playerRecentlyLogout;
        bool m_playerSave;
        uint32 m_exhaustionState;
        uint32 m_charactersCount;                           // init with max, to prevent character creation before amount is recalculated in CharEnum handler
        uint32 m_characterMaxLevel;
        BigNumber m_sessionKey;
        AccountData m_accountData[NewAccountData::NUM_ACCOUNT_DATA_TYPES];
        uint32 m_tutorials[ACCOUNT_TUTORIALS_COUNT];
        TutorialDataState m_tutorialState;

        // Clustering system (TODO remove this)
    public:
        MasterPlayer* GetMasterPlayer() const { return m_masterPlayer; }
        PlayerPointer GetPlayerPointer() const
        {
            if (_player)
                return PlayerPointer(new PlayerWrapper<Player>(_player));
            if (GetMasterPlayer())
                return PlayerPointer(new PlayerWrapper<MasterPlayer>(GetMasterPlayer()));
            return PlayerPointer(nullptr);
        }
    protected:
        MasterPlayer*   m_masterPlayer;
        // End of clustering system
};
#endif
