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

// Addon taint info the client attaches to actions started from addon or /run code.
inline bool SkipAuctionTaintedBy(ByteBuffer& data)
{
    data.ResetBitPos();
    if (data.size() - data.rpos() < 3)
        return false;
    uint32 const nameLength = data.ReadBits(10);
    uint32 const versionLength = data.ReadBits(10);
    data.ReadBit(); // Loaded
    data.ReadBit(); // Disabled
    data.ResetBitPos();
    if (data.size() - data.rpos() < nameLength + versionLength)
        return false;
    data.read_skip(nameLength);
    data.read_skip(versionLength);
    return true;
}

struct AuctionHelloRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_HELLO_REQUEST;

    ClassicGuid AuctioneerGuid;

    void Write(ByteBuffer& data) const { AuctioneerGuid.Write(data); }
    bool Read(ByteBuffer& data) { return AuctioneerGuid.Read(data); }
};

struct AuctionItemForSale
{
    ClassicGuid ItemGuid;
    uint32 UseCount = 0;
};

// Runtime is in minutes. Each item entry is a bag item and how many of its stack go up for sale.
struct AuctionSellItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_SELL_ITEM;

    ClassicGuid AuctioneerGuid;
    uint64 Bid = 0;
    uint64 Buyout = 0;
    uint32 Etime = 0;
    std::vector<AuctionItemForSale> Items;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << Bid << Buyout << Etime;
        data.WriteBit(false); // TaintedBy
        data.WriteBits(uint32(Items.size()), 5);
        data.FlushBits();
        for (AuctionItemForSale const& item : Items)
        {
            item.ItemGuid.Write(data);
            data << item.UseCount;
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint64) + sizeof(uint32) + 1)
            return false;
        data >> Bid >> Buyout >> Etime;
        bool const tainted = data.ReadBit();
        uint32 const count = data.ReadBits(5);
        if (tainted && !SkipAuctionTaintedBy(data))
            return false;
        data.ResetBitPos();

        Items.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            AuctionItemForSale item;
            if (!item.ItemGuid.Read(data))
                return false;
            if (data.size() - data.rpos() < sizeof(uint32))
                return false;
            data >> item.UseCount;
            Items.push_back(item);
        }
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
        data.WriteBit(false); // TaintedBy
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> AuctionId;
        return !data.ReadBit() || SkipAuctionTaintedBy(data);
    }
};

struct AuctionListOwnerItems
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_LIST_OWNER_ITEMS;

    ClassicGuid AuctioneerGuid;
    uint32 ListFrom = 0;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << ListFrom;
        data << uint8(0);
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> ListFrom;
        data.rpos(data.size()); // trailing bit block, always zero
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
        data.WriteBit(false); // TaintedBy
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(uint64) + 1)
            return false;
        data >> AuctionId >> Price;
        return !data.ReadBit() || SkipAuctionTaintedBy(data);
    }
};

// The refresh-id count is a bits<7> bit-packed value on the wire, not a plain uint32 -- reading it
// as one misreads the first byte of the (usually absent) refresh-id list as a huge count and
// desyncs trying to read that many nonexistent uint32s.
struct AuctionListBidderItems
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_LIST_BIDDER_ITEMS;

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

struct AuctionListFilterSubClass
{
    int32 ItemSubclass = 0;
    uint64 InvTypeMask = 0;
};

struct AuctionListFilterClass
{
    int32 ItemClass = 0;
    std::vector<AuctionListFilterSubClass> SubClassFilters;
};

// Sort criteria are skipped; results are always returned in the engine's own order.
struct AuctionListItems
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUCTION_LIST_ITEMS;

    ClassicGuid AuctioneerGuid;
    uint32 ListFrom = 0;
    uint8 LevelMin = 0;
    uint8 LevelMax = 0;
    uint32 Quality = 0xFFFFFFFF;
    std::string SearchedName;
    std::vector<AuctionListFilterClass> ClassFilters;
    bool OnlyUsable = false;
    bool ExactMatch = false;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << ListFrom << LevelMin << LevelMax << Quality;
        data << uint8(0);  // SortCount
        data << uint32(0); // KnownPets
        data << uint8(0);  // MaxPetLevel

        data.WriteBit(false); // TaintedBy
        data.WriteSizedString(SearchedName, 8);

        data.WriteBits(uint32(ClassFilters.size()), 3);
        data.WriteBit(OnlyUsable);
        data.WriteBit(ExactMatch);
        data.FlushBits();
        for (AuctionListFilterClass const& filter : ClassFilters)
        {
            data << filter.ItemClass;
            data.WriteBits(uint32(filter.SubClassFilters.size()), 5);
            data.FlushBits();
            for (AuctionListFilterSubClass const& sub : filter.SubClassFilters)
                data << sub.InvTypeMask << sub.ItemSubclass;
        }

        data << uint32(0); // sort data size
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32) + 2 * sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8))
            return false;
        data >> ListFrom >> LevelMin >> LevelMax >> Quality;
        uint8 sortCount; data >> sortCount;
        uint32 knownPetsCount; data >> knownPetsCount;
        uint8 maxPetLevel; data >> maxPetLevel;
        if (data.size() - data.rpos() < knownPetsCount + 2)
            return false;
        data.read_skip(knownPetsCount);

        data.ResetBitPos();
        bool const tainted = data.ReadBit();
        uint32 const nameLen = data.ReadBits(8);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLen + 1)
            return false;
        SearchedName = data.ReadString(nameLen);

        uint32 const classFiltersCount = data.ReadBits(3);
        OnlyUsable = data.ReadBit();
        ExactMatch = data.ReadBit();
        if (tainted && !SkipAuctionTaintedBy(data))
            return false;
        data.ResetBitPos();

        ClassFilters.resize(classFiltersCount);
        for (AuctionListFilterClass& filter : ClassFilters)
        {
            if (data.size() - data.rpos() < sizeof(int32) + 1)
                return false;
            data >> filter.ItemClass;
            uint32 const subCount = data.ReadBits(5);
            data.ResetBitPos();
            if (data.size() - data.rpos() < subCount * (sizeof(uint64) + sizeof(int32)))
                return false;
            filter.SubClassFilters.resize(subCount);
            for (AuctionListFilterSubClass& sub : filter.SubClassFilters)
                data >> sub.InvTypeMask >> sub.ItemSubclass;
        }

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 sortDataSize; data >> sortDataSize;
        if (data.size() - data.rpos() < sortDataSize)
            return false;
        data.read_skip(sortDataSize);
        return true;
    }
};

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
    uint32 PurchaseDeliveryDelay = 0;
    uint32 CancelDeliveryDelay = 0;
    uint32 HouseId = 0;

    void Write(ByteBuffer& data) const
    {
        AuctioneerGuid.Write(data);
        data << PurchaseDeliveryDelay << CancelDeliveryDelay << HouseId;
        data.WriteBit(true); // OpenForBusiness -- this engine has no "AH closed" concept
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!AuctioneerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32) + 1)
            return false;
        data >> PurchaseDeliveryDelay >> CancelDeliveryDelay >> HouseId;
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

// Body layout is TrinityCore's AuctionListItemsResult; not yet verified against the client with a non-empty list.
struct AuctionListItemsResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_LIST_RESULT;

    std::vector<AuctionItem> Items;
    int32 TotalCount = 0;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Items.size()) << uint32(TotalCount) << uint32(300); // DesiredDelay
        for (AuctionItem const& item : Items)
            item.Write(data);
        data.WriteBits(0, 2); // ListType
        data.WriteBit(false); // HasMoreResults
        data.FlushBits();
        data.WriteBits(0, 20); // BucketKey: ItemID
        data.WriteBit(false);  // BattlePetSpeciesID
        data.WriteBits(0, 11); // ItemLevel
        data.WriteBit(false);  // ItemSuffix
        data.FlushBits();
        data << uint32(TotalCount);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        uint32 count; uint32 desiredDelay;
        data >> count >> TotalCount >> desiredDelay;
        Items.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            AuctionItem item;
            if (!item.Read(data))
                return false;
            Items.push_back(item);
        }
        if (data.size() - data.rpos() < 6 + sizeof(uint32))
            return false;
        data.read_skip(6);
        data >> TotalCount;
        return true;
    }
};

struct AuctionListBidderItemsResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_LIST_BIDDER_ITEMS_RESULT;

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

struct AuctionListOwnerItemsResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_LIST_OWNER_ITEMS_RESULT;

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

// Mail-list shaped (count, total, mails). Auctions resolve immediately here, so it is always empty.
struct AuctionListPendingSalesResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUCTION_LIST_PENDING_SALES_RESULT;

    void Write(ByteBuffer& data) const
    {
        data << uint32(0) << int32(0);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(int32))
            return false;
        uint32 count; int32 total;
        data >> count >> total;
        return count == 0;
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
