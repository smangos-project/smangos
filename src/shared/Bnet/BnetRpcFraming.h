#ifndef SHARED_BNETRPCFRAMING_H
#define SHARED_BNETRPCFRAMING_H

#include "Common.h"
#include <string>
#include <vector>

namespace bgs { namespace protocol { class Header; } }

// Wire framing for Battle.net RPC, both directions: [u16 BE headerLength][Header protobuf][payload].
// A response carries service_id RESPONSE_SERVICE_ID, an unprompted push carries PUSH_SERVICE_ID;
// everything else about a frame's meaning belongs to the caller.
class BnetRpcFraming
{
    public:
        static constexpr uint32 ResponseServiceId = 0xFE;
        static constexpr uint32 PushServiceId = 0;

        static constexpr uint32 MaxHeaderLength = 4096;
        static constexpr uint32 MaxPayloadSize = 0x100000; // 1 MiB sanity bound

        static std::vector<uint8> EncodeFrame(bgs::protocol::Header const& header, std::string const& payload);

        static uint16 DecodeHeaderLength(uint8 const bytes[2]);

        // Parses `headerBytes` into `header`. Returns false on a malformed header; `header` is then
        // left in whatever state protobuf's ParseFromArray leaves it in.
        static bool DecodeHeader(char const* headerBytes, size_t headerLen, bgs::protocol::Header& header);
};

#endif
