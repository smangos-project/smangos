#ifndef MANGOS_CLASSIC_GUIDMAPPING_H
#define MANGOS_CLASSIC_GUIDMAPPING_H

#include "Guid.h"
#include "ObjectGuid.h"
#include "nonstd/optional.hpp"

// The cast guid source an aura carries.
constexpr uint8 CLASSIC_CAST_SOURCE_AURA = 13;

// Maps between the server's 64-bit object guids and the 128-bit guids the modern protocol carries.
class ClassicGuidMapping
{
    public:
        // `realmId` is stamped into every realm-specific and map-specific guid. An empty ObjectGuid
        // maps to an empty ClassicGuid, as does one of a type with no classic counterpart. The wire's
        // entry field is 23 bits wide, so a larger entry loses its top bit, and the map id is always
        // 0 (the server guid carries none). HIGHGUID_TRANSPORT and HIGHGUID_MO_TRANSPORT both map
        // to the classic transport type.
        static ClassicGuid ToClassic(ObjectGuid const& guid, uint32 realmId);

        // nullopt for a guid type the server has no ObjectGuid for (party, guild, ...) or one whose
        // counter doesn't fit the server's guid layout. A classic transport comes back as
        // HIGHGUID_TRANSPORT.
        static nonstd::optional<ObjectGuid> FromClassic(ClassicGuid const& guid);

        // A cast guid: unique per cast (`counter`), keyed on the map and spell it belongs to. The
        // client pairs a cast's start, go and failure messages by it, and rejects an empty one.
        // `source` says what started the cast; an aura's own cast guid uses CLASSIC_CAST_SOURCE_AURA.
        static ClassicGuid MakeCast(uint32 realmId, uint32 mapId, uint32 spellId, uint64 counter, uint8 source = 0);
};

#endif
