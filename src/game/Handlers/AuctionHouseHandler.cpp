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

#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "AuctionHouseMgr.h"
#include "TransactionLog.h"
#include "Mail.h"
#include "Util.h"
#include "Chat.h"
#include "Anticheat.h"
#include "Packets/Classic/GuidMapping.h"

// please DO NOT use iterator++, because it is slower than ++iterator!!!
// post-incrementation is always slower than pre-incrementation !

namespace
{
ClassicItemInstance MinimalAuctionItemInstance(AuctionEntry const& auction)
{
    ClassicItemInstance item;
    if (Item* pItem = sAuctionMgr.GetAItem(auction.itemGuidLow))
        item.ItemId = int32(pItem->GetEntry());
    else
        item.ItemId = int32(auction.itemTemplate);
    return item;
}
}

// void called when player click on auctioneer npc
void WorldSession::HandleAuctionHelloOpcode(AuctionHelloRequest const& packet)
{
    nonstd::optional<ObjectGuid> const auctioneerGuid = ClassicGuidMapping::FromClassic(packet.AuctioneerGuid);
    Creature* unit = auctioneerGuid ? GetPlayer()->GetNPCIfCanInteractWith(*auctioneerGuid, UNIT_NPC_FLAG_AUCTIONEER) : nullptr;
    if (!unit)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: HandleAuctionHelloOpcode - auctioneer not found or you can't interact with it.");
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    SendAuctionHello(unit);
}

// this void causes that auction window is opened
void WorldSession::SendAuctionHello(Unit* unit)
{
    // always return pointer
    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntry(unit);

    AuctionHelloResponse response;
    response.AuctioneerGuid = ClassicGuidMapping::ToClassic(unit->GetObjectGuid(), realmID);
    response.HouseId = ahEntry->houseId;
    SendClassic(response);
}

// call this method when player bids, creates, or deletes auction
void WorldSession::SendAuctionCommandResult(AuctionEntry* auc, AuctionAction Action, AuctionError ErrorCode, InventoryResult invError)
{
    AuctionCommandResult packet;
    packet.AuctionId = auc ? auc->Id : 0;
    packet.Action = Action;
    packet.ErrorCode = ErrorCode;

    switch (ErrorCode)
    {
        case AUCTION_OK:
            if (Action == AUCTION_BID_PLACED)
                packet.OutBid = auc->GetAuctionOutBid();
            break;
        case AUCTION_ERR_INVENTORY:
            packet.BagResult = invError;
            break;
        case AUCTION_ERR_HIGHER_BID:
            packet.NewBidderGuid = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, auc->bidder), realmID);
            packet.NewBid = auc->bid;
            packet.OutBid = auc->GetAuctionOutBid();
            break;
        default:
            break;
    }

    SendClassic(packet);
}

// this function sends notification, if bidder is online -- won ? AuctionWonNotification : AuctionOutbidNotification
void WorldSession::SendAuctionBidderNotification(AuctionEntry* auction, bool won)
{
    if (won)
    {
        AuctionWonNotification notification;
        notification.AuctionId = int32(auction->Id);
        notification.BidderGuid = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, auction->bidder), realmID);
        notification.Item = MinimalAuctionItemInstance(*auction);
        SendClassic(notification);
    }
    else
    {
        AuctionOutbidNotification notification;
        notification.AuctionId = int32(auction->Id);
        notification.BidderGuid = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, auction->bidder), realmID);
        notification.Item = MinimalAuctionItemInstance(*auction);
        notification.BidAmount = auction->bid;
        notification.MinIncrement = auction->GetAuctionOutBid();
        SendClassic(notification);
    }
}

// the auction is still open, but a new bid just came in -- distinct from the sold/expired case
// SendAuctionOwnerNotification below covers
void WorldSession::SendAuctionOwnerBidNotification(AuctionEntry* auction)
{
    AuctionOwnerBidNotification notification;
    notification.AuctionId = int32(auction->Id);
    notification.BidAmount = auction->bid;
    notification.Item = MinimalAuctionItemInstance(*auction);
    notification.MinIncrement = auction->GetAuctionOutBid();
    notification.Bidder = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, auction->bidder), realmID);
    SendClassic(notification);
}

// this void causes on client to display: "Your auction sold"
void WorldSession::SendAuctionOwnerNotification(AuctionEntry* auction, bool sold)
{
    AuctionClosedNotification notification;
    notification.AuctionId = int32(auction->Id);
    notification.BidAmount = auction->bid;
    notification.Item = MinimalAuctionItemInstance(*auction);
    notification.Sold = sold;
    SendClassic(notification);
}

// shows ERR_AUCTION_REMOVED_S -- no exact modern equivalent (this is "owner cancelled, your bid is
// refunded", not really an outbid), AuctionOutbidNotification is the closest available signal that
// still gets the bidder's attention -- see AuctionOutbidNotification's class comment.
void WorldSession::SendAuctionRemovedNotification(AuctionEntry* auction)
{
    AuctionOutbidNotification packet;
    packet.AuctionId = int32(auction->Id);
    packet.BidderGuid = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, auction->bidder), realmID);
    packet.Item = MinimalAuctionItemInstance(*auction);
    packet.BidAmount = auction->bid;
    packet.MinIncrement = auction->GetAuctionOutBid();
    SendClassic(packet);
}

// this function sends mail to old bidder
void WorldSession::SendAuctionOutbiddedMail(AuctionEntry* auction)
{
    ObjectGuid oldBidder_guid = ObjectGuid(HIGHGUID_PLAYER, auction->bidder);
    Player* oldBidder = sObjectMgr.GetPlayer(oldBidder_guid);

    uint32 oldBidder_accId = 0;
    if (!oldBidder)
        oldBidder_accId = sObjectMgr.GetPlayerAccountIdByGUID(oldBidder_guid);

    // old bidder exist
    if (oldBidder || oldBidder_accId)
    {
        std::ostringstream msgAuctionOutbiddedSubject;
        msgAuctionOutbiddedSubject << auction->itemTemplate << ":0:" << AUCTION_OUTBIDDED;

        if (oldBidder)
            oldBidder->GetSession()->SendAuctionBidderNotification(auction, false);

        MailDraft(msgAuctionOutbiddedSubject.str())
        .SetMoney(auction->bid)
        .SendMailTo(MailReceiver(oldBidder, oldBidder_guid), auction, MAIL_CHECK_MASK_COPIED);
    }
}

// this function sends mail, when auction is cancelled to old bidder
void WorldSession::SendAuctionCancelledToBidderMail(AuctionEntry* auction)
{
    ObjectGuid bidder_guid = ObjectGuid(HIGHGUID_PLAYER, auction->bidder);
    Player* bidder = sObjectMgr.GetPlayer(bidder_guid);

    uint32 bidder_accId = 0;
    if (!bidder)
        bidder_accId = sObjectMgr.GetPlayerAccountIdByGUID(bidder_guid);

    // bidder exist
    if (bidder || bidder_accId)
    {
        std::ostringstream msgAuctionCancelledSubject;
        msgAuctionCancelledSubject << auction->itemTemplate << ":0:" << AUCTION_CANCELLED_TO_BIDDER;

        if (bidder)
            bidder->GetSession()->SendAuctionRemovedNotification(auction);

        MailDraft(msgAuctionCancelledSubject.str())
        .SetMoney(auction->bid)
        .SendMailTo(MailReceiver(bidder, bidder_guid), auction, MAIL_CHECK_MASK_COPIED);
    }
}

AuctionHouseEntry const* WorldSession::GetCheckedAuctionHouseForAuctioneer(ObjectGuid guid)
{
    Unit* auctioneer = nullptr;

    // GM case
    if (guid == GetPlayer()->GetObjectGuid())
    {
        // command case will return only if player have real access to command
        // using special access modes (1,-1) done at mode set in command, so not need recheck
        if (GetPlayer()->GetAuctionAccessMode() == 0 && !ChatHandler(GetPlayer()).FindCommand("auction"))
        {
            sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "%s attempt open auction in cheating way.", guid.GetString().c_str());
            return nullptr;
        }

        auctioneer = GetPlayer();
    }
    // auctioneer case
    else
    {
        auctioneer = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
        if (!auctioneer)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Auctioneeer %s accessed in cheating way.", guid.GetString().c_str());
            return nullptr;
        }
    }

    // always return pointer
    return AuctionHouseMgr::GetAuctionHouseEntry(auctioneer);
}

// this void creates new auction and adds auction to some auctionhouse
void WorldSession::HandleAuctionSellItem(AuctionSellItem const& packet)
{
    if (!packet.Bid || !packet.Etime)
        return;                                             // check for cheaters

    // Client limit
    if (packet.Bid > 2000000000 || packet.Buyout > 2000000000)
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_NOT_ENOUGH_MONEY);
        ProcessAnticheatAction("GoldDupe", "Putting too high auction price", CHEAT_ACTION_LOG);
        return;
    }
    if (packet.Buyout && packet.Bid > packet.Buyout)
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_HIGHER_BID);
        ProcessAnticheatAction("GoldDupe", "bid > buyout", CHEAT_ACTION_LOG);
        return;
    }

    if (!sWorld.getConfig(CONFIG_BOOL_GM_ALLOW_TRADES) && GetSecurity() > SEC_PLAYER)
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_RESTRICTED_ACCOUNT);
        return;
    }

    if (HasTrialRestrictions())
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_RESTRICTED_ACCOUNT);
        return;
    }

    Player* pl = GetPlayer();

    nonstd::optional<ObjectGuid> const auctioneerGuid = ClassicGuidMapping::FromClassic(packet.AuctioneerGuid);
    AuctionHouseEntry const* auctionHouseEntry = auctioneerGuid ? GetCheckedAuctionHouseForAuctioneer(*auctioneerGuid) : nullptr;
    if (!auctionHouseEntry)
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_DATABASE);
        return;
    }

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    uint32 limit = sWorld.getConfig(CONFIG_UINT32_ACCOUNT_CONCURRENT_AUCTION_LIMIT);
    if (!!limit && auctionHouse->GetAccountAuctionCount(GetAccountId()) >= limit)
    {
        pl->SendSysMessage("You have reached the limit of active auctions on your account.");
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_DATABASE);
        return;
    }

    // client send time in minutes, convert to common used sec time
    uint32 etime = packet.Etime * MINUTE;

    // client understand only 3 auction time
    switch (etime)
    {
        case 1*MIN_AUCTION_TIME:
            break;
        case 4*MIN_AUCTION_TIME:
            break;
        case 12*MIN_AUCTION_TIME:
            break;
        default:
            SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_DATABASE);
            return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    nonstd::optional<ObjectGuid> const itemGuid = ClassicGuidMapping::FromClassic(packet.ItemGuid);
    if (!itemGuid)
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_ITEM_NOT_FOUND);
        return;
    }

    Item *it = pl->GetItemByGuid(*itemGuid);

    // do not allow to sell already auctioned items
    if (sAuctionMgr.GetAItem(itemGuid->GetCounter()))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "AuctionError, %s is sending %s, but item is already in another auction", pl->GetGuidStr().c_str(), itemGuid->GetString().c_str());
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    // prevent sending bag with items (cheat: can be placed in bag after adding equipped empty bag to auction)
    if (!it)
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    // prevent selling item in bank slot
    if (_player->IsBankPos(it->GetPos()))
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (!it->CanBeTraded())
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if ((it->GetProto()->Flags & ITEM_FLAG_CONJURED) || it->GetUInt32Value(ITEM_FIELD_DURATION))
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    // check money for deposit
    uint32 deposit = AuctionHouseMgr::GetAuctionDeposit(auctionHouseEntry, etime, it);

    if (pl->GetMoney() < deposit)
    {
        SendAuctionCommandResult(nullptr, AUCTION_STARTED, AUCTION_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    uint32 const bid = uint32(packet.Bid);
    uint32 const buyout = uint32(packet.Buyout);

    if (GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_BOOL_GM_LOG_TRADE))
    {
        sLog.Player(GetAccountId(), LOG_GM, LOG_LVL_BASIC,
            "GM %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
            GetPlayerName(), GetAccountId(), it->GetProto()->Name1, it->GetEntry(), it->GetCount());
    }

    pl->ModifyMoney(-int32(deposit));

    uint32 auction_time = uint32(etime * sWorld.getConfig(CONFIG_FLOAT_RATE_AUCTION_TIME));

    AuctionEntry* AH = new AuctionEntry;
    AH->Id = sObjectMgr.GenerateAuctionID();
    AH->itemGuidLow = it->GetObjectGuid().GetCounter();
    AH->itemTemplate = it->GetEntry();
    AH->owner = pl->GetGUIDLow();
    AH->ownerAccount = pl->GetSession()->GetAccountId();
    AH->startbid = bid;
    AH->bidder = 0;
    AH->bid = 0;
    AH->buyout = buyout;
    AH->lockedIpAddress = GetRemoteAddress();
    AH->depositTime = time(nullptr);
    AH->expireTime = time(nullptr) + auction_time;
    AH->deposit = deposit;
    AH->auctionHouseEntry = auctionHouseEntry;

    sLog.Player(this, LOG_MONEY_TRADES, LOG_LVL_MINIMAL, "[AuctionHouse]: Player %s listing %s (%u) at auctioneer. Initial bid: %u, buyout: %u, duration: %u, auctionhouse: %u",
                pl->GetShortDescription().c_str(), it->GetGuidStr().c_str(), it->GetEntry(),
                bid, buyout, auction_time, AH->GetHouseId());

    // Log this transaction
    PlayerTransactionData data;
    data.type = "PlaceAuction";
    data.parts[0].lowGuid = AH->owner;
    data.parts[0].itemsEntries[0] = AH->itemTemplate;
    data.parts[0].itemsCount[0] = it->GetCount();
    data.parts[0].itemsGuid[0] = it->GetGUIDLow();
    data.parts[0].money = bid;
    data.parts[1].lowGuid = auctioneerGuid->GetCounter();
    data.parts[1].money = buyout;
    sWorld.LogTransaction(data);

    auctionHouse->AddAuction(AH);

    sAuctionMgr.AddAItem(it);
    pl->MoveItemFromInventory(it->GetBagSlot(), it->GetSlot(), true);

    CharacterDatabase.BeginTransaction(pl->GetGUIDLow());
    it->DeleteFromInventoryDB();
    it->SaveToDB();                                         // recursive and not have transaction guard into self, not in inventiory and can be save standalone
    AH->SaveToDB();
    pl->SaveInventoryAndGoldToDB();
    CharacterDatabase.CommitTransaction();

    SendAuctionCommandResult(AH, AUCTION_STARTED, AUCTION_OK);
}

// this function is called when client bids or buys out auction
void WorldSession::HandleAuctionPlaceBid(AuctionPlaceBid const& packet)
{
    if (!sWorld.getConfig(CONFIG_BOOL_GM_ALLOW_TRADES) && GetSecurity() > SEC_PLAYER)
    {
        SendAuctionCommandResult(nullptr, AUCTION_BID_PLACED, AUCTION_ERR_RESTRICTED_ACCOUNT);
        return;
    }

    if (HasTrialRestrictions())
    {
        SendAuctionCommandResult(nullptr, AUCTION_BID_PLACED, AUCTION_ERR_RESTRICTED_ACCOUNT);
        return;
    }

    if (!packet.AuctionId || !packet.Price || packet.Price > 2000000000)
    {
        SendAuctionCommandResult(nullptr, AUCTION_BID_PLACED, AUCTION_ERR_ITEM_NOT_FOUND);
        return;
    }
    uint32 const price = uint32(packet.Price);

    nonstd::optional<ObjectGuid> const auctioneerGuid = ClassicGuidMapping::FromClassic(packet.AuctioneerGuid);
    AuctionHouseEntry const* auctionHouseEntry = auctioneerGuid ? GetCheckedAuctionHouseForAuctioneer(*auctioneerGuid) : nullptr;
    if (!auctionHouseEntry)
    {
        SendAuctionCommandResult(nullptr, AUCTION_BID_PLACED, AUCTION_ERR_ITEM_NOT_FOUND);
        return;
    }

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    AuctionEntry* auction = auctionHouse->GetAuction(packet.AuctionId);
    Player* pl = GetPlayer();

    if (!auction)
    {
        // item not found; auction may have expired, or been bought out
        SendAuctionCommandResult(nullptr, AUCTION_BID_PLACED, AUCTION_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (auction->owner == pl->GetGUIDLow())
    {
        // you cannot bid your own auction:
        SendAuctionCommandResult(nullptr, AUCTION_BID_PLACED, AUCTION_ERR_BID_OWN);
        return;
    }

    ObjectGuid ownerGuid = ObjectGuid(HIGHGUID_PLAYER, auction->owner);

    // impossible have online own another character (use this for speedup check in case online owner)
    Player* auction_owner = sObjectMgr.GetPlayer(ownerGuid);
    if (!auction_owner && sObjectMgr.GetPlayerAccountIdByGUID(ownerGuid) == pl->GetSession()->GetAccountId())
    {
        // you cannot bid your another character auction:
        SendAuctionCommandResult(nullptr, AUCTION_BID_PLACED, AUCTION_ERR_BID_OWN);
        return;
    }

    // cheating
    if (price < auction->startbid)
    {
        SendAuctionCommandResult(nullptr, AUCTION_BID_PLACED, AUCTION_ERR_BID_INCREMENT);
        return;
    }

    // cheating or client lags
    if (price <= auction->bid)
    {
        // client test but possible in result lags
        SendAuctionCommandResult(auction, AUCTION_BID_PLACED, AUCTION_ERR_HIGHER_BID);
        return;
    }

    // price too low for next bid if not buyout
    if ((price < auction->buyout || auction->buyout == 0) &&
         price < auction->bid + auction->GetAuctionOutBid())
    {
        // client test but possible in result lags
        SendAuctionCommandResult(auction, AUCTION_BID_PLACED, AUCTION_ERR_BID_INCREMENT);
        return;
    }

    if (price > pl->GetMoney())
    {
        // you don't have enough money!, client tests!
        return;
    }

    if ((price < auction->buyout) || (auction->buyout == 0))// bid
    {
        if (pl->GetGUIDLow() == auction->bidder)
            pl->LogModifyMoney(-int32(price - auction->bid), "AuctionBid", ObjectGuid(HIGHGUID_PLAYER, auction->owner), auction->itemTemplate);
        else
        {
            pl->LogModifyMoney(-int32(price), "AuctionBid", ObjectGuid(HIGHGUID_PLAYER, auction->owner), auction->itemTemplate);
            if (auction->bidder)                            // return money to old bidder if present
                SendAuctionOutbiddedMail(auction);
        }

        auction->bidder = pl->GetGUIDLow();
        auction->bid = price;

        if (auction_owner)
            auction_owner->GetSession()->SendAuctionOwnerBidNotification(auction);

        // after this update we should save player's money ...
        CharacterDatabase.PExecute("UPDATE `auction` SET `buyer_guid` = '%u', `last_bid` = '%u' WHERE `id` = '%u'", auction->bidder, auction->bid, auction->Id);

        SendAuctionCommandResult(auction, AUCTION_BID_PLACED, AUCTION_OK);
    }
    else                                                    // buyout
    {
        if (pl->GetGUIDLow() == auction->bidder)
            pl->LogModifyMoney(-int32(auction->buyout - auction->bid), "AuctionBuyout", ObjectGuid(HIGHGUID_PLAYER, auction->owner), auction->itemTemplate);
        else
        {
            pl->LogModifyMoney(-int32(auction->buyout), "AuctionBuyout", ObjectGuid(HIGHGUID_PLAYER, auction->owner), auction->itemTemplate);
            if (auction->bidder)                            // return money to old bidder if present
                SendAuctionOutbiddedMail(auction);
        }

        auction->bidder = pl->GetGUIDLow();
        auction->bid = auction->buyout;

        PlayerTransactionData data;
        data.type = "Buyout";
        data.parts[0].lowGuid = auction->owner;
        data.parts[0].itemsEntries[0] = auction->itemTemplate;
        Item* item = sAuctionMgr.GetAItem(auction->itemGuidLow);
        data.parts[0].itemsCount[0] = item ? item->GetCount() : 0;
        data.parts[0].itemsGuid[0] = auction->itemGuidLow;
        data.parts[1].lowGuid = auction->bidder;
        data.parts[1].money = auction->bid;
        sWorld.LogTransaction(data);

        sAuctionMgr.SendAuctionSuccessfulMail(auction);
        sAuctionMgr.SendAuctionWonMail(auction);

        SendAuctionCommandResult(auction, AUCTION_BID_PLACED, AUCTION_OK);

        sAuctionMgr.RemoveAItem(auction->itemGuidLow);
        auctionHouse->RemoveAuction(auction);
        auction->DeleteFromDB();

        delete auction;
    }
    CharacterDatabase.BeginTransaction(pl->GetGUIDLow());
    pl->SaveInventoryAndGoldToDB();
    CharacterDatabase.CommitTransaction();
}

// this void is called when auction_owner cancels his auction
void WorldSession::HandleAuctionRemoveItem(AuctionRemoveItem const& packet)
{
    nonstd::optional<ObjectGuid> const auctioneerGuid = ClassicGuidMapping::FromClassic(packet.AuctioneerGuid);
    AuctionHouseEntry const* auctionHouseEntry = auctioneerGuid ? GetCheckedAuctionHouseForAuctioneer(*auctioneerGuid) : nullptr;
    if (!auctionHouseEntry)
        return;

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);
    AuctionEntry* auction = auctionHouse->GetAuction(packet.AuctionId);
    Player* pl = GetPlayer();

    if (auction && auction->owner == pl->GetGUIDLow())
    {
        Item *pItem = sAuctionMgr.GetAItem(auction->itemGuidLow);
        if (pItem)
        {
            if (auction->bidder > 0)                        // If we have a bidder, we have to send him the money he paid
            {
                uint32 auctionCut = auction->GetAuctionCut();
                if (pl->GetMoney() < auctionCut)            // player doesn't have enough money, maybe message needed
                    return;

                SendAuctionCancelledToBidderMail(auction);
                pl->ModifyMoney(-int32(auctionCut));
            }
            // Return the item by mail
            std::ostringstream msgAuctionCanceledOwner;
            msgAuctionCanceledOwner << auction->itemTemplate << ":0:" << AUCTION_CANCELED;

            // item will deleted or added to received mail list
            MailDraft(msgAuctionCanceledOwner.str())
            .AddItem(pItem)
            .SendMailTo(pl, auction, MAIL_CHECK_MASK_COPIED);
        }
        else
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Auction id: %u has nonexistent item (item guid : %u)!!!", auction->Id, auction->itemGuidLow);
            SendAuctionCommandResult(nullptr, AUCTION_REMOVED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
            return;
        }
    }
    else
    {
        SendAuctionCommandResult(nullptr, AUCTION_REMOVED, AUCTION_ERR_DATABASE);
        // this code isn't possible ... maybe there should be ASSERT
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "CHEATER : %u, he tried to cancel auction (id: %u) of another player, or auction is nullptr", pl->GetGUIDLow(), packet.AuctionId);
        return;
    }

    // inform player, that auction is removed
    SendAuctionCommandResult(auction, AUCTION_REMOVED, AUCTION_OK);
    // Now remove the auction
    CharacterDatabase.BeginTransaction(pl->GetGUIDLow());
    auction->DeleteFromDB();
    pl->SaveInventoryAndGoldToDB();
    CharacterDatabase.CommitTransaction();
    sAuctionMgr.RemoveAItem(auction->itemGuidLow);
    auctionHouse->RemoveAuction(auction);
    delete auction;
}

class AuctionHouseClientQueryTask : public AuctionHouseClientQuery
{
public:
    AuctionHouseClientQueryTask(AuctionClientQueryType type) : _queryType(type)
    {
    }
    void operator ()()
    {
        if (WorldSession* sess = sWorld.FindSession(accountId))
        {
            sess->SetReceivedAHListRequest(_queryType, false);

            Player* player = sess->GetPlayer();
            if (!player || !player->IsInWorld())
                return;

            std::vector<AuctionItem> items;
            uint32 count = 0;
            uint32 totalcount = 0;

            switch (_queryType)
            {
                case AUCTION_QUERY_LIST:
                {
                    auctionHouse->BuildListAuctionItems(items, player, *this, count, totalcount);
                    AuctionListItemsResult result;
                    result.Items = std::move(items);
                    result.TotalCount = int32(totalcount);
                    result.OnlyUsable = usable != 0;
                    sess->SendClassic(result);
                    break;
                }
                case AUCTION_QUERY_LIST_BIDDER:
                {
                    for (const auto& outbiddedAuctionId : outbiddedAuctionIds)
                    {
                        AuctionEntry* auction = auctionHouse->GetAuction(outbiddedAuctionId);
                        if (auction)
                        {
                            ++totalcount;

                            if (count < 50 && totalcount > listfrom)
                            {
                                AuctionItem item;
                                if (auction->BuildAuctionInfo(item))
                                {
                                    items.push_back(item);
                                    ++count;
                                }
                            }
                        }
                    }

                    auctionHouse->BuildListBidderItems(items, player, listfrom, count, totalcount);
                    AuctionListBiddedItemsResult result;
                    result.Items = std::move(items);
                    result.TotalCount = int32(totalcount);
                    sess->SendClassic(result);
                    break;
                }
                case AUCTION_QUERY_LIST_OWNER:
                {
                    auctionHouse->BuildListOwnerItems(items, player, listfrom, count, totalcount);
                    AuctionListOwnedItemsResult result;
                    result.Items = std::move(items);
                    result.TotalCount = int32(totalcount);
                    sess->SendClassic(result);
                    break;
                }
                default:
                {
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "[AsyncAuctionQuery] Invalid query type %u", _queryType);
                    return;
                }
            }
        }
    }
    AuctionHouseObject* auctionHouse;
    AuctionClientQueryType _queryType;
};

// called when player lists his bids
void WorldSession::HandleAuctionListBidderItems(AuctionListBidderItems const& packet)
{
    if (ReceivedAHListRequest(AUCTION_QUERY_LIST_BIDDER))
        return; // Only one AH request at a time is allowed

    nonstd::optional<ObjectGuid> const auctioneerGuid = ClassicGuidMapping::FromClassic(packet.AuctioneerGuid);
    AuctionHouseEntry const* auctionHouseEntry = auctioneerGuid ? GetCheckedAuctionHouseForAuctioneer(*auctioneerGuid) : nullptr;
    if (!auctionHouseEntry)
        return;

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseClientQueryTask task(AUCTION_QUERY_LIST_BIDDER);
    task.auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);
    task.accountId = GetAccountId();
    task.listfrom = packet.Offset;
    task.outbiddedAuctionIds = packet.AuctionIdsToRefresh;
    SetReceivedAHListRequest(AUCTION_QUERY_LIST_BIDDER, true);
    sWorld.AddAsyncTask(std::move(task));
}

// this void sends player info about his auctions
void WorldSession::HandleAuctionListOwnerItems(AuctionListOwnerItems const& packet)
{
    if (ReceivedAHListRequest(AUCTION_QUERY_LIST_OWNER))
        return;

    nonstd::optional<ObjectGuid> const auctioneerGuid = ClassicGuidMapping::FromClassic(packet.AuctioneerGuid);
    AuctionHouseEntry const* auctionHouseEntry = auctioneerGuid ? GetCheckedAuctionHouseForAuctioneer(*auctioneerGuid) : nullptr;
    if (!auctionHouseEntry)
        return;

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseClientQueryTask task(AUCTION_QUERY_LIST_OWNER);
    task.auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);
    task.accountId = GetAccountId();
    task.listfrom = packet.ListFrom;
    SetReceivedAHListRequest(AUCTION_QUERY_LIST_OWNER, true);
    sWorld.AddAsyncTask({std::move(task)});
}

void WorldSession::HandleAuctionListItems(AuctionListItems const& packet)
{
    if (ReceivedAHListRequest(AUCTION_QUERY_LIST))
        return;

    AuctionHouseClientQueryTask task(AUCTION_QUERY_LIST);
    task.accountId = GetAccountId();

    task.listfrom = packet.ListFrom;
    task.levelmin = packet.LevelMin;
    task.levelmax = packet.LevelMax;
    task.classFilters = packet.ClassFilters;
    task.quality = packet.Quality;
    task.usable = packet.OnlyUsable ? 1 : 0;

    nonstd::optional<ObjectGuid> const auctioneerGuid = ClassicGuidMapping::FromClassic(packet.AuctioneerGuid);
    AuctionHouseEntry const* auctionHouseEntry = auctioneerGuid ? GetCheckedAuctionHouseForAuctioneer(*auctioneerGuid) : nullptr;
    if (!auctionHouseEntry)
        return;

    // always return pointer
    task.auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // converting string that we try to find to lower case
    if (!Utf8toWStr(packet.SearchedName, task.wsearchedname))
        return;

    wstrToLower(task.wsearchedname);
    SetReceivedAHListRequest(AUCTION_QUERY_LIST, true);
    sWorld.AddAsyncTask(std::move(task));
}

void WorldSession::HandleAuctionListPendingSales(AuctionListPendingSales const& /*packet*/)
{
    // This engine resolves auctions immediately and has no pending-sales mailbox, so an empty
    // reply would be correct anyway -- but CLASSIC_SMSG_AUCTION_LIST_PENDING_SALES_RESULT does not
    // exist on the modern wire at all, so sending nothing is the only wire-legal answer.
}
