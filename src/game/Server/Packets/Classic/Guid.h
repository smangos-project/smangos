#ifndef MANGOS_CLASSIC_GUID_H
#define MANGOS_CLASSIC_GUID_H

#include "ByteBuffer.h"

// Packed 128-bit guid, the wire format every modern packet uses for an object guid: a two-byte
// (lowMask, highMask) header, then only the non-zero bytes each mask marks. Low/High are the raw
// 128-bit value, named as the wire names them.
struct ClassicGuid
{
    uint64 Low = 0;
    uint64 High = 0;

    bool IsEmpty() const { return Low == 0 && High == 0; }

    void Write(ByteBuffer& data) const
    {
        uint8 lowBytes[8], highBytes[8];
        uint8 lowMask = 0, highMask = 0;
        uint8 loSize = 0, hiSize = 0;
        for (uint8 i = 0; i < 8; ++i)
        {
            uint8 b = uint8(Low >> (i * 8));
            if (b)
            {
                lowMask |= uint8(1 << i);
                lowBytes[loSize++] = b;
            }
        }
        for (uint8 i = 0; i < 8; ++i)
        {
            uint8 b = uint8(High >> (i * 8));
            if (b)
            {
                highMask |= uint8(1 << i);
                highBytes[hiSize++] = b;
            }
        }

        data << lowMask << highMask;
        for (uint8 i = 0; i < loSize; ++i)
            data << lowBytes[i];
        for (uint8 i = 0; i < hiSize; ++i)
            data << highBytes[i];
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint8 lowMask, highMask;
        data >> lowMask >> highMask;

        Low = 0;
        High = 0;
        for (uint8 i = 0; i < 8; ++i)
            if (lowMask & uint8(1 << i))
            {
                if (data.size() - data.rpos() < 1)
                    return false;
                uint8 b;
                data >> b;
                Low |= uint64(b) << (i * 8);
            }
        for (uint8 i = 0; i < 8; ++i)
            if (highMask & uint8(1 << i))
            {
                if (data.size() - data.rpos() < 1)
                    return false;
                uint8 b;
                data >> b;
                High |= uint64(b) << (i * 8);
            }
        return true;
    }
};

#endif
