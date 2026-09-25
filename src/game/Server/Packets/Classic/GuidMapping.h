#ifndef MANGOS_CLASSIC_GUIDMAPPING_H
#define MANGOS_CLASSIC_GUIDMAPPING_H

#include "Guid.h"
#include "ObjectGuid.h"
#include "nonstd/optional.hpp"

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
};

#endif
