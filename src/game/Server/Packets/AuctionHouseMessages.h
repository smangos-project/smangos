#ifndef MANGOS_PACKETS_AUCTIONHOUSEMESSAGES_H
#define MANGOS_PACKETS_AUCTIONHOUSEMESSAGES_H

// The modern auction house wire, light like Packets/MailMessages.h. The 1.15 client never gained
// the bucket/commodity browsing redesign (item classes grouped by key, favorites, batched
// commodity purchase) that shipped for retail auction houses -- it still lists individual postings
// one at a time, so none of that is implemented here.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/ItemMessages.h"
#include <string>
#include <vector>

// --- Client packets ---

struct AuctionHelloRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_HELLO_REQUEST;

    ClassicGuid AuctioneerGuid;

    void Write(ByteBuffer& data) const { AuctioneerGuid.Write(data); }
    bool Read(ByteBuffer& data) { return AuctioneerGuid.Read(data); }
};

struct AuctionSellItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_SELL_ITEM;

    ClassicGuid AuctioneerGuid;
    ClassicGuid ItemGuid;
    uint64 Bid = 0;
    uint64 Buyout = 0;
    uint32 Etime = 0;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        ItemGuid.Write(data);
        data << Bid << Buyout << Etime;
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (!ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint64) + sizeof(uint32))
            return false;
        data >> Bid >> Buyout >> Etime;
        return true;
    }
};

struct AuctionRemoveItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_REMOVE_ITEM;

    ClassicGuid AuctioneerGuid;
    uint32 AuctionId = 0;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << AuctionId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> AuctionId;
        return true;
    }
};

struct AuctionListOwnedItems
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_LIST_OWNED_ITEMS;

    ClassicGuid AuctioneerGuid;
    uint32 ListFrom = 0;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << ListFrom;
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> ListFrom;
        return true;
    }
};

struct AuctionPlaceBid
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_PLACE_BID;

    ClassicGuid AuctioneerGuid;
    uint32 AuctionId = 0;
    uint64 Price = 0;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << AuctionId << Price;
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(uint64))
            return false;
        data >> AuctionId >> Price;
        return true;
    }
};

// The refresh-id count is a bits<7> bit-packed value on the wire, not a plain uint32 -- reading it
// as one misreads the first byte of the (usually absent) refresh-id list as a huge count and
// desyncs trying to read that many nonexistent uint32s.
struct AuctionListBiddedItems
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_LIST_BIDDED_ITEMS;

    ClassicGuid AuctioneerGuid;
    uint32 Offset = 0;
    std::vector<uint32> AuctionIdsToRefresh;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << Offset;
        data.WriteBits(uint32(AuctionIdsToRefresh.size()), 7);
        data.FlushBits();
        for (uint32 id : AuctionIdsToRefresh)
            data << id;
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Offset;

        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const count = data.ReadBits(7);
        data.ResetBitPos();

        AuctionIdsToRefresh.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            if (data.size() - data.rpos() < sizeof(uint32))
                return false;
            uint32 id;
            data >> id;
            AuctionIdsToRefresh.push_back(id);
        }
        return true;
    }
};

// This engine's filtering (AuctionHouseObject::BuildListAuctionItems) understands only one flat
// class/subclass pair, not the modern client's per-class array of subclass filters -- only the
// first class filter's class, and its first subclass filter's subclass, are kept; any further
// filters are parsed (to stay stream-aligned) and discarded, same for known-pet filters and sort
// criteria (no pet-auction or server-side-sort system here).
struct AuctionListItems
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_LIST_ITEMS;

    static constexpr uint32 NoFilter = 0xFFFFFFFF;

    ClassicGuid AuctioneerGuid;
    uint32 ListFrom = 0;
    std::string SearchedName;
    uint8 LevelMin = 0;
    uint8 LevelMax = 0;
    uint32 AuctionSlotId = NoFilter;
    uint32 AuctionMainCategory = NoFilter;
    uint32 AuctionSubCategory = NoFilter;
    uint32 Quality = NoFilter;
    uint8 Usable = 0;

    void Write(ByteBuffer& data) const
    {
        data << ListFrom;
        AuctioneerGuid.Write(data);
        data << LevelMin << LevelMax << Quality;
        data << uint8(0); // SortCount
        data << uint32(0); // KnownPetsCount
        data << uint8(0); // MaxPetLevel

        data.WriteBits(uint32(SearchedName.length()), 8);
        data.FlushBits();
        data.WriteString(SearchedName);

        bool const hasFilter = AuctionMainCategory != NoFilter;
        data.WriteBits(hasFilter ? 1 : 0, 3); // ClassFiltersCount
        data.WriteBit(Usable != 0);
        data.WriteBit(false); // ExactMatch
        data.FlushBits();

        if (hasFilter)
        {
            data.ResetBitPos();
            data << int32(AuctionMainCategory);
            data.ResetBitPos();
            bool const hasSubFilter = AuctionSubCategory != NoFilter;
            data.WriteBits(hasSubFilter ? 1 : 0, 5);
            data.FlushBits();
            if (hasSubFilter)
            {
                data << int32(AuctionSubCategory);
                data << uint64(0); // InvTypeMask
            }
        }

        data << uint32(0); // sort criteria sub-block length
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> ListFrom;
        if (!AuctioneerGuid.Read(data))
            return false;

        if (data.size() - data.rpos() < 2 * sizeof(uint8) + sizeof(uint32))
            return false;
        data >> LevelMin >> LevelMax >> Quality;

        if (data.size() - data.rpos() < sizeof(uint8) + sizeof(uint32) + sizeof(uint8))
            return false;
        uint8 sortCount; data >> sortCount;
        uint32 knownPetsCount; data >> knownPetsCount;
        uint8 maxPetLevel; data >> maxPetLevel;
        if (data.size() - data.rpos() < knownPetsCount)
            return false;
        for (uint32 i = 0; i < knownPetsCount; ++i)
        {
            uint8 petSpecies;
            data >> petSpecies;
        }

        data.ResetBitPos();
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLen = data.ReadBits(8);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLen)
            return false;
        SearchedName = data.ReadString(nameLen);

        data.ResetBitPos();
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const classFiltersCount = data.ReadBits(3);
        bool const onlyUsable = data.ReadBit();
        data.ReadBit(); // ExactMatch -- no exact-vs-partial name match distinction here
        data.ResetBitPos();
        Usable = onlyUsable ? 1 : 0;

        AuctionMainCategory = NoFilter;
        AuctionSubCategory = NoFilter;
        AuctionSlotId = NoFilter;

        for (uint32 i = 0; i < classFiltersCount; ++i)
        {
            data.ResetBitPos();
            if (data.size() - data.rpos() < sizeof(int32))
                return false;
            int32 itemClass; data >> itemClass;
            data.ResetBitPos();
            if (data.size() - data.rpos() < 1)
                return false;
            uint32 const subClassFiltersCount = data.ReadBits(5);
            data.ResetBitPos();

            if (i == 0)
                AuctionMainCategory = uint32(itemClass);

            for (uint32 j = 0; j < subClassFiltersCount; ++j)
            {
                if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint64))
                    return false;
                int32 itemSubclass; data >> itemSubclass;
                uint64 invTypeMask; data >> invTypeMask;
                if (i == 0 && j == 0)
                    AuctionSubCategory = uint32(itemSubclass);
            }
        }

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 sortBlockSize; data >> sortBlockSize;
        if (data.size() - data.rpos() < sortBlockSize)
            return false;
        data.read_skip(sortBlockSize);
        return true;
    }
};

// Vanilla/TC 3.3.5 send this query empty-bodied ("give me my pending sales") -- this engine
// resolves auctions immediately, has no pending-sales mailbox, and never replies to it.
struct AuctionListPendingSales
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_LIST_PENDING_SALES;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// --- Server packets ---

struct AuctionHelloResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_HELLO_RESPONSE;

    ClassicGuid AuctioneerGuid;
    uint32 HouseId = 0;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << HouseId;
        data.WriteBit(true); // OpenForBusiness -- this engine has no "AH closed" concept
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> HouseId;
        data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Named after TrinityCore's own AuctionCommandResult, not this engine's SendAuctionCommandResult()
// method -- that would otherwise collide with the wire struct's name now that light packets carry
// no namespace to disambiguate with. Unconditional fixed body (confirmed against TrinityCore's own
// AuctionCommandResult::Write()): fields not meaningful for a given ErrorCode/Action combination
// are simply sent as zero/empty; the client only reads what it needs for that ErrorCode.
struct AuctionCommandResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_COMMAND_RESULT;

    uint32 AuctionId = 0;
    uint32 Action = 0;
    uint32 ErrorCode = 0;
    uint32 BagResult = 0;    // used by AUCTION_ERR_INVENTORY
    ClassicGuid NewBidderGuid; // used by AUCTION_ERR_HIGHER_BID
    uint64 NewBid = 0;       // used by AUCTION_ERR_HIGHER_BID
    uint64 OutBid = 0;       // used by AUCTION_OK + AUCTION_BID_PLACED, and AUCTION_ERR_HIGHER_BID

    void Write(ByteBuffer& data) const
    {
        data << AuctionId << Action << ErrorCode << BagResult;
        NewBidderGuid.Write(data);
        data << NewBid << OutBid;
        data << uint32(0); // DesiredDelay -- no artificial-delay/anti-sniping system here
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4 * sizeof(uint32))
            return false;
        data >> AuctionId >> Action >> ErrorCode >> BagResult;
        if (!NewBidderGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint64) + sizeof(uint32))
            return false;
        data >> NewBid >> OutBid;
        uint32 desiredDelay; data >> desiredDelay;
        return true;
    }
};

// The auction posting shape shared by every list result and closed/won/outbid notification. This
// engine tracks none of the commodity-era fields (gems, unit price, bucket key, a creator distinct
// from the owner), so those are always absent; the embedded item is the minimal, no-bonus-data
// shape used everywhere else in this port (vendor, quest and mail items).
struct AuctionItem
{
    ClassicItemInstance Item;
    int32 Count = 0;
    int32 Charges = 0;
    uint32 PermanentEnchantId = 0; // 0 = none
    int32 Flags = 196608;          // no per-auction flag system here; this is every real posting's value
    int32 AuctionId = 0;
    ClassicGuid Owner;
    int32 DurationLeftMs = 0;
    uint64 MinBid = 0;
    uint64 MinIncrement = 0; // only sent while there is a bid
    uint64 BuyoutPrice = 0;  // only sent when the posting has a buyout
    ClassicGuid ItemGuid;
    ClassicGuid OwnerAccountGuid;
    uint32 EndTime = 0;
    ClassicGuid Bidder;      // only sent once there is a bidder
    uint64 BidAmount = 0;    // only sent once there is a bidder

    void Write(ByteBuffer& data) const
    {
        bool const hasBidder = !Bidder.IsEmpty();
        bool const hasMinIncrement = MinIncrement != 0;
        bool const hasBuyout = BuyoutPrice != 0;

        data.WriteBit(true); // Item != null
        data.WriteBits(PermanentEnchantId ? 1 : 0, 4); // Enchantments.Count
        data.WriteBits(0, 2); // Gems.Count
        data.WriteBit(true);  // MinBid.HasValue
        data.WriteBit(hasMinIncrement);
        data.WriteBit(hasBuyout);
        data.WriteBit(false); // UnitPrice.HasValue -- no commodity system
        data.WriteBit(false); // CensorServerSideInfo
        data.WriteBit(false); // CensorBidInfo
        data.WriteBit(false); // AuctionBucketKey != null
        data.WriteBit(false); // Creator != default
        data.WriteBit(hasBidder);
        data.WriteBit(hasBidder); // BidAmount.HasValue tracks whether there is a bidder too
        data.FlushBits();

        Item.Write(data);
        data << Count << Charges << Flags << AuctionId;
        Owner.Write(data);
        data << DurationLeftMs;
        data << uint8(0); // DeleteReason

        if (PermanentEnchantId)
        {
            data << PermanentEnchantId;
            data << uint32(0); // Expiration -- no separate enchant-expiry tracking for auctioned items
            data << int32(0);  // Charges
            data << uint8(0);  // Slot -- PERM_ENCHANTMENT_SLOT, the only one auctioned items ever carry
        }

        data << MinBid;
        if (hasMinIncrement)
            data << MinIncrement;
        if (hasBuyout)
            data << BuyoutPrice;

        // CensorServerSideInfo == false
        ItemGuid.Write(data);
        OwnerAccountGuid.Write(data);
        data << EndTime;

        // Creator != default == false

        // CensorBidInfo == false
        if (hasBidder)
        {
            Bidder.Write(data);
            data << BidAmount;
        }

        // Gems empty, AuctionBucketKey null -- nothing more to write
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3) // 18 bits
            return false;
        bool const hasItem = data.ReadBit();
        uint32 const enchantCount = data.ReadBits(4);
        uint32 const gemCount = data.ReadBits(2);
        bool const hasMinBid = data.ReadBit();
        bool const hasMinIncrement = data.ReadBit();
        bool const hasBuyout = data.ReadBit();
        bool const hasUnitPrice = data.ReadBit();
        bool const censorServerSideInfo = data.ReadBit();
        bool const censorBidInfo = data.ReadBit();
        bool const hasBucketKey = data.ReadBit();
        bool const hasCreator = data.ReadBit();
        bool const hasBidder = data.ReadBit();
        bool const hasBidAmount = data.ReadBit();
        data.ResetBitPos();

        if (!hasItem || gemCount != 0 || hasUnitPrice || hasBucketKey || hasCreator)
            return false; // this engine never sends or expects any of these

        if (!Item.Read(data))
            return false;

        if (data.size() - data.rpos() < 4 * sizeof(int32))
            return false;
        data >> Count >> Charges >> Flags >> AuctionId;
        if (!Owner.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint8))
            return false;
        data >> DurationLeftMs;
        uint8 deleteReason; data >> deleteReason;

        PermanentEnchantId = 0;
        if (enchantCount)
        {
            if (enchantCount != 1)
                return false; // this engine only ever sends the one permanent-enchant slot
            if (data.size() - data.rpos() < 2 * sizeof(uint32) + sizeof(int32) + sizeof(uint8))
                return false;
            uint32 expiration; int32 charges; uint8 slot;
            data >> PermanentEnchantId >> expiration >> charges >> slot;
        }

        if (!hasMinBid)
            return false; // this engine always sends one
        if (data.size() - data.rpos() < sizeof(uint64))
            return false;
        data >> MinBid;
        if (hasMinIncrement)
        {
            if (data.size() - data.rpos() < sizeof(uint64))
                return false;
            data >> MinIncrement;
        }
        if (hasBuyout)
        {
            if (data.size() - data.rpos() < sizeof(uint64))
                return false;
            data >> BuyoutPrice;
        }

        if (censorServerSideInfo)
            return false; // this engine never censors, so never expects it either
        if (!ItemGuid.Read(data))
            return false;
        if (!OwnerAccountGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> EndTime;

        if (censorBidInfo)
            return false;
        if (hasBidder != hasBidAmount)
            return false; // this engine always sends both together or neither
        if (hasBidder)
        {
            if (!Bidder.Read(data))
                return false;
            if (data.size() - data.rpos() < sizeof(uint64))
                return false;
            data >> BidAmount;
        }
        return true;
    }
};

// No bucket/commodity envelope (ListType, BucketKey, HasMoreResults, TotalCount) -- the 1.15
// client's plain item-list query never asks for one. OnlyUsable is only sent once, and only when
// the result isn't empty, matching this engine's own "usable" filter round trip.
struct AuctionListItemsResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_LIST_ITEMS_RESULT;

    std::vector<AuctionItem> Items;
    int32 TotalCount = 0;
    bool OnlyUsable = false;

    void Write(ByteBuffer& data) const
    {
        data << int32(Items.size()) << TotalCount << uint32(300); // DesiredDelay
        if (!Items.empty())
            data << uint8(OnlyUsable ? 1 : 0);
        for (AuctionItem const& item : Items)
            item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32) + sizeof(uint32))
            return false;
        int32 count; uint32 desiredDelay;
        data >> count >> TotalCount >> desiredDelay;
        if (count > 0)
        {
            if (data.size() - data.rpos() < sizeof(uint8))
                return false;
            uint8 onlyUsable; data >> onlyUsable;
            OnlyUsable = onlyUsable != 0;
        }
        Items.clear();
        for (int32 i = 0; i < count; ++i)
        {
            AuctionItem item;
            if (!item.Read(data))
                return false;
            Items.push_back(item);
        }
        return true;
    }
};

struct AuctionListBiddedItemsResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_LIST_BIDDED_ITEMS_RESULT;

    std::vector<AuctionItem> Items;
    int32 TotalCount = 0;

    void Write(ByteBuffer& data) const
    {
        data << int32(Items.size()) << TotalCount << uint32(300); // DesiredDelay
        for (AuctionItem const& item : Items)
            item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32) + sizeof(uint32))
            return false;
        int32 count; uint32 desiredDelay;
        data >> count >> TotalCount >> desiredDelay;
        Items.clear();
        for (int32 i = 0; i < count; ++i)
        {
            AuctionItem item;
            if (!item.Read(data))
                return false;
            Items.push_back(item);
        }
        return true;
    }
};

struct AuctionListOwnedItemsResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_LIST_OWNED_ITEMS_RESULT;

    std::vector<AuctionItem> Items;
    int32 TotalCount = 0;

    void Write(ByteBuffer& data) const
    {
        data << int32(Items.size()) << TotalCount << uint32(300); // DesiredDelay
        for (AuctionItem const& item : Items)
            item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32) + sizeof(uint32))
            return false;
        int32 count; uint32 desiredDelay;
        data >> count >> TotalCount >> desiredDelay;
        Items.clear();
        for (int32 i = 0; i < count; ++i)
        {
            AuctionItem item;
            if (!item.Read(data))
                return false;
            Items.push_back(item);
        }
        return true;
    }
};

// The auction is still open, but a new bid just came in -- the modern client's distinct
// "someone bid on your item" owner notice. This engine's vanilla behavior folded this into the
// same closed-auction message the sold/expired cases used; the modern wire keeps it separate.
struct AuctionOwnerBidNotification
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_OWNER_BID_NOTIFICATION;

    int32 AuctionId = 0;
    uint64 BidAmount = 0;
    ClassicItemInstance Item;
    uint64 MinIncrement = 0;
    ClassicGuid Bidder;

    void Write(ByteBuffer& data) const
    {
        data << AuctionId << BidAmount;
        Item.Write(data);
        data << MinIncrement;
        Bidder.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint64))
            return false;
        data >> AuctionId >> BidAmount;
        if (!Item.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint64))
            return false;
        data >> MinIncrement;
        return Bidder.Read(data);
    }
};

// Covers both this engine's "sold"/"not sold" owner cases via the Sold bit -- no separate packet
// for the unsold/expired case, matching WorldSession::SendAuctionOwnerNotification's own bool.
struct AuctionClosedNotification
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_CLOSED_NOTIFICATION;

    int32 AuctionId = 0;
    uint64 BidAmount = 0;
    ClassicItemInstance Item;
    bool Sold = false;

    void Write(ByteBuffer& data) const
    {
        data << AuctionId << BidAmount;
        Item.Write(data);
        data << float(3600.0f); // ProceedsMailDelay
        data.WriteBit(Sold);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint64))
            return false;
        data >> AuctionId >> BidAmount;
        if (!Item.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(float) + 1)
            return false;
        float proceedsMailDelay; data >> proceedsMailDelay;
        Sold = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct AuctionWonNotification
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_WON_NOTIFICATION;

    int32 AuctionId = 0;
    ClassicGuid BidderGuid;
    ClassicItemInstance Item;

    void Write(ByteBuffer& data) const
    {
        data << int32(2); // Command -- AUCTION_BID_PLACED
        data << AuctionId;
        BidderGuid.Write(data);
        Item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        int32 command; data >> command >> AuctionId;
        if (!BidderGuid.Read(data))
            return false;
        return Item.Read(data);
    }
};

// Also used, as the closest available signal, for the "owner cancelled the auction, your bid is
// refunded" case -- there is no exact modern equivalent for that (it isn't really an outbid), but
// this still gets the bidder's attention and carries the right refund amount.
struct AuctionOutbidNotification
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_OUTBID_NOTIFICATION;

    int32 AuctionId = 0;
    ClassicGuid BidderGuid;
    ClassicItemInstance Item;
    uint64 BidAmount = 0;
    uint64 MinIncrement = 0;

    void Write(ByteBuffer& data) const
    {
        data << int32(2); // Command -- AUCTION_BID_PLACED
        data << AuctionId;
        BidderGuid.Write(data);
        Item.Write(data);
        data << BidAmount << MinIncrement;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        int32 command; data >> command >> AuctionId;
        if (!BidderGuid.Read(data))
            return false;
        if (!Item.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint64))
            return false;
        data >> BidAmount >> MinIncrement;
        return true;
    }
};

#endif
