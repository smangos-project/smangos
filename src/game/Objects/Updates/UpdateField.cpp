#include "UpdateField.h"
#include "ByteBuffer.h"

void WriteDynamicFieldUpdateMask(std::size_t size, std::vector<uint32> const& updateMask, ByteBuffer& data, int32 bitsForSize /*= 32*/)
{
    data.WriteBits(size, bitsForSize);
    if (size > 32)
    {
        for (std::size_t block = 0; block < size / 32; ++block)
            data.WriteBits(updateMask[block], 32);
    }
    else if (size == 32)
    {
        data.WriteBits(updateMask.back(), 32);
        return;
    }

    if (size % 32)
        data.WriteBits(updateMask.back(), size % 32);
}

void WriteCompleteDynamicFieldUpdateMask(std::size_t size, ByteBuffer& data, int32 bitsForSize /*= 32*/)
{
    data.WriteBits(size, bitsForSize);
    if (size > 32)
    {
        for (std::size_t block = 0; block < size / 32; ++block)
            data.WriteBits(0xFFFFFFFFu, 32);
    }
    else if (size == 32)
    {
        data.WriteBits(0xFFFFFFFFu, 32);
        return;
    }

    if (size % 32)
        data.WriteBits(0xFFFFFFFFu, size % 32);
}
