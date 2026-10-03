#include "GuidMapping.h"

namespace
{
enum ClassicGuidType : uint8
{
    CLASSIC_GUID_PLAYER         = 2,
    CLASSIC_GUID_ITEM           = 3,
    CLASSIC_GUID_TRANSPORT      = 6,
    CLASSIC_GUID_CREATURE       = 8,
    CLASSIC_GUID_PET            = 10,
    CLASSIC_GUID_GAMEOBJECT     = 11,
    CLASSIC_GUID_DYNAMICOBJECT  = 12,
    CLASSIC_GUID_CORPSE         = 14,
    CLASSIC_GUID_CHAT_CHANNEL   = 26,
    CLASSIC_GUID_PARTY          = 27,
    CLASSIC_GUID_GUILD          = 28,
    CLASSIC_GUID_WOW_ACCOUNT    = 29,
    CLASSIC_GUID_BNET_ACCOUNT   = 30,
    CLASSIC_GUID_CAST           = 47,
};

// high = type << 58 | realm << 42 | map << 29 | entry << 6 | subtype, low = counter (40 bits).
// Transports have no realm, map or low part: high = type << 58 | counter << 38 | entry.
constexpr uint64 TYPE_SHIFT = 58;
constexpr uint64 REALM_SHIFT = 42;
constexpr uint64 ENTRY_SHIFT = 6;
constexpr uint64 TRANSPORT_COUNTER_SHIFT = 38;
constexpr uint64 MAP_SHIFT = 29;
constexpr uint64 MAP_MASK = 0x1FFF;
constexpr uint64 REALM_MASK = 0x1FFF;
constexpr uint64 ENTRY_MASK = 0x7FFFFF;
constexpr uint64 SUBTYPE_MASK = 0x3F;
constexpr uint64 TRANSPORT_COUNTER_MASK = 0xFFFFF;
constexpr uint64 COUNTER_MASK = 0xFFFFFFFFFFULL;

bool ToClassicType(HighGuid high, uint8& type)
{
    switch (high)
    {
        case HIGHGUID_PLAYER:        type = CLASSIC_GUID_PLAYER; return true;
        case HIGHGUID_ITEM:          type = CLASSIC_GUID_ITEM; return true;
        case HIGHGUID_UNIT:          type = CLASSIC_GUID_CREATURE; return true;
        case HIGHGUID_PET:           type = CLASSIC_GUID_PET; return true;
        case HIGHGUID_GAMEOBJECT:    type = CLASSIC_GUID_GAMEOBJECT; return true;
        case HIGHGUID_DYNAMICOBJECT: type = CLASSIC_GUID_DYNAMICOBJECT; return true;
        case HIGHGUID_CORPSE:        type = CLASSIC_GUID_CORPSE; return true;
        case HIGHGUID_TRANSPORT:
        case HIGHGUID_MO_TRANSPORT:  type = CLASSIC_GUID_TRANSPORT; return true;
        default:                     return false;
    }
}

bool FromClassicType(uint8 type, HighGuid& high)
{
    switch (type)
    {
        case CLASSIC_GUID_PLAYER:        high = HIGHGUID_PLAYER; return true;
        case CLASSIC_GUID_ITEM:          high = HIGHGUID_ITEM; return true;
        case CLASSIC_GUID_CREATURE:      high = HIGHGUID_UNIT; return true;
        case CLASSIC_GUID_PET:           high = HIGHGUID_PET; return true;
        case CLASSIC_GUID_GAMEOBJECT:    high = HIGHGUID_GAMEOBJECT; return true;
        case CLASSIC_GUID_DYNAMICOBJECT: high = HIGHGUID_DYNAMICOBJECT; return true;
        case CLASSIC_GUID_CORPSE:        high = HIGHGUID_CORPSE; return true;
        case CLASSIC_GUID_TRANSPORT:     high = HIGHGUID_TRANSPORT; return true;
        default:                         return false;
    }
}
}

ClassicGuid ClassicGuidMapping::ToClassic(ObjectGuid const& guid, uint32 realmId)
{
    ClassicGuid out;
    uint8 type;
    if (guid.IsEmpty() || !ToClassicType(guid.GetHigh(), type))
        return out;

    if (type == CLASSIC_GUID_TRANSPORT)
    {
        out.High = (uint64(type) << TYPE_SHIFT)
            | ((uint64(guid.GetCounter()) & TRANSPORT_COUNTER_MASK) << TRANSPORT_COUNTER_SHIFT)
            | (uint64(guid.GetEntry()) & ENTRY_MASK);
        return out;
    }

    out.Low = guid.GetCounter();
    out.High = (uint64(type) << TYPE_SHIFT) | ((uint64(realmId) & REALM_MASK) << REALM_SHIFT);
    if (type != CLASSIC_GUID_PLAYER && type != CLASSIC_GUID_ITEM)
        out.High |= (uint64(guid.GetEntry()) & ENTRY_MASK) << ENTRY_SHIFT;
    return out;
}

nonstd::optional<ObjectGuid> ClassicGuidMapping::FromClassic(ClassicGuid const& guid)
{
    if (guid.IsEmpty())
        return ObjectGuid();

    uint8 const type = uint8((guid.High >> TYPE_SHIFT) & 0x3F);
    HighGuid high;
    if (!FromClassicType(type, high))
        return nonstd::nullopt;

    if (type == CLASSIC_GUID_TRANSPORT)
    {
        return ObjectGuid(high, uint32(guid.High & ENTRY_MASK),
            uint32((guid.High >> TRANSPORT_COUNTER_SHIFT) & TRANSPORT_COUNTER_MASK));
    }

    uint64 const counter = guid.Low & COUNTER_MASK;
    if (counter > ObjectGuid::GetMaxCounter(high))
        return nonstd::nullopt;

    if (type == CLASSIC_GUID_CREATURE || type == CLASSIC_GUID_PET || type == CLASSIC_GUID_GAMEOBJECT)
        return ObjectGuid(high, uint32((guid.High >> ENTRY_SHIFT) & ENTRY_MASK), uint32(counter));
    return ObjectGuid(high, uint32(counter));
}

ClassicGuid ClassicGuidMapping::MakeCast(uint32 realmId, uint32 mapId, uint32 spellId, uint64 counter, uint8 source)
{
    ClassicGuid out;
    out.Low = counter & COUNTER_MASK;
    out.High = (uint64(CLASSIC_GUID_CAST) << TYPE_SHIFT)
        | ((uint64(realmId) & REALM_MASK) << REALM_SHIFT)
        | ((uint64(mapId) & MAP_MASK) << MAP_SHIFT)
        | ((uint64(spellId) & ENTRY_MASK) << ENTRY_SHIFT)
        | (uint64(source) & SUBTYPE_MASK);
    return out;
}

ClassicGuid ClassicGuidMapping::MakeAccount(uint32 accountId)
{
    return ClassicGuid{ accountId, uint64(CLASSIC_GUID_WOW_ACCOUNT) << TYPE_SHIFT };
}

uint32 ClassicGuidMapping::VirtualRealmAddress(uint32 realmId)
{
    return (uint32(1) << 24) | (uint32(1) << 16) | uint16(realmId);
}

ClassicGuid ClassicGuidMapping::MakeBnetAccount(uint32 accountId)
{
    return ClassicGuid{ accountId, uint64(CLASSIC_GUID_BNET_ACCOUNT) << TYPE_SHIFT };
}

ClassicGuid ClassicGuidMapping::MakeParty(uint32 groupId)
{
    return ClassicGuid{ groupId, uint64(CLASSIC_GUID_PARTY) << TYPE_SHIFT };
}

ClassicGuid ClassicGuidMapping::MakeGuild(uint32 guildId)
{
    return ClassicGuid{ guildId, uint64(CLASSIC_GUID_GUILD) << TYPE_SHIFT };
}

nonstd::optional<uint32> ClassicGuidMapping::GuildIdFromClassic(ClassicGuid const& guid)
{
    if (uint8((guid.High >> TYPE_SHIFT) & 0x3F) != CLASSIC_GUID_GUILD)
        return nonstd::nullopt;
    return uint32(guid.Low & COUNTER_MASK);
}

ClassicGuid ClassicGuidMapping::MakeChannel(uint32 realmId, uint32 mapId, uint32 zoneId, uint32 channelId)
{
    ClassicGuid out;
    out.Low = channelId;
    out.High = (uint64(CLASSIC_GUID_CHAT_CHANNEL) << TYPE_SHIFT)
        | ((uint64(realmId) & REALM_MASK) << REALM_SHIFT)
        | ((uint64(mapId) & MAP_MASK) << MAP_SHIFT)
        | ((uint64(zoneId) & ENTRY_MASK) << ENTRY_SHIFT);
    return out;
}
