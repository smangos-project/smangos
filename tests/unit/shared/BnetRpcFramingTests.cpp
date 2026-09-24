#include "Bnet/BnetRpcFraming.h"
#include "rpc_types.pb.h"

#include <catch_amalgamated.hpp>

TEST_CASE("BnetRpcFraming round-trips a header and payload", "[bnet]")
{
    bgs::protocol::Header out;
    out.set_service_id(0);
    out.set_service_hash(0x0DECFC01u);
    out.set_method_id(1);
    out.set_token(42);

    std::string const payload = "hello";
    out.set_size(uint32(payload.size()));

    std::vector<uint8> const frame = BnetRpcFraming::EncodeFrame(out, payload);
    REQUIRE(frame.size() > 2);

    uint8 const lengthBytes[2] = { frame[0], frame[1] };
    uint16 const headerLen = BnetRpcFraming::DecodeHeaderLength(lengthBytes);
    REQUIRE(size_t(2 + headerLen) <= frame.size());

    bgs::protocol::Header in;
    REQUIRE(BnetRpcFraming::DecodeHeader(reinterpret_cast<char const*>(frame.data() + 2), headerLen, in));
    CHECK(in.service_id() == out.service_id());
    CHECK(in.service_hash() == out.service_hash());
    CHECK(in.method_id() == out.method_id());
    CHECK(in.token() == out.token());
    CHECK(in.size() == out.size());

    std::string const decodedPayload(frame.begin() + 2 + headerLen, frame.end());
    CHECK(decodedPayload == payload);
}

TEST_CASE("BnetRpcFraming rejects a malformed header", "[bnet]")
{
    std::vector<uint8> const garbage = { 0xFF, 0xFF, 0xFF };

    bgs::protocol::Header header;
    CHECK_FALSE(BnetRpcFraming::DecodeHeader(reinterpret_cast<char const*>(garbage.data()), garbage.size(), header));
}

TEST_CASE("BnetRpcFraming decodes a big-endian header length", "[bnet]")
{
    uint8 const bytes[2] = { 0x01, 0x02 };
    CHECK(BnetRpcFraming::DecodeHeaderLength(bytes) == 0x0102);
}
