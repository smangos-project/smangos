#include "BnetRpcFraming.h"
#include "rpc_types.pb.h"

std::vector<uint8> BnetRpcFraming::EncodeFrame(bgs::protocol::Header const& header, std::string const& payload)
{
    std::string headerBytes;
    header.SerializeToString(&headerBytes);

    std::vector<uint8> frame;
    frame.reserve(2 + headerBytes.size() + payload.size());
    uint16 const headerLen = uint16(headerBytes.size());
    frame.push_back(uint8(headerLen >> 8));
    frame.push_back(uint8(headerLen));
    frame.insert(frame.end(), headerBytes.begin(), headerBytes.end());
    frame.insert(frame.end(), payload.begin(), payload.end());
    return frame;
}

uint16 BnetRpcFraming::DecodeHeaderLength(uint8 const bytes[2])
{
    return uint16((uint16(bytes[0]) << 8) | bytes[1]);
}

bool BnetRpcFraming::DecodeHeader(char const* headerBytes, size_t headerLen, bgs::protocol::Header& header)
{
    return header.ParseFromArray(headerBytes, int(headerLen));
}
