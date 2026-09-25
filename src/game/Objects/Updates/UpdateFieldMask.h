#ifndef UPDATEFIELDMASK_H
#define UPDATEFIELDMASK_H

#include "Platform/Define.h"
#include <algorithm>
#include "Errors.h"
#include <array>
#include <cstring>

inline constexpr std::size_t GetUpdateFieldBlockIndex(std::size_t bit) { return bit / 32u; }
inline constexpr uint32 GetUpdateFieldBlockFlag(std::size_t bit) { return 1u << (bit % 32u); }

// A fixed-size bitmask over Bits update-field slots, grouped into 32-bit blocks with a second
// level "which blocks are non-zero" mask, so a mostly-unchanged struct can skip most of its
// blocks on the wire. Named UpdateFieldMask, not UpdateMask, to avoid the unrelated vanilla
// ::UpdateMask (Objects/UpdateMask.h).
template<uint32 Bits>
class UpdateFieldMask
{
    public:
        static constexpr uint32 BlockCount = (Bits + 31) / 32;
        static constexpr uint32 BlocksMaskCount = (BlockCount + 31) / 32;

        constexpr UpdateFieldMask() : _blocksMask(), _blocks()
        {
        }

        constexpr UpdateFieldMask(std::array<uint32, BlockCount> const& init)
        {
            _blocksMask.back() = 0; // only the last blocksMask entry is not written in full below
            for (uint32 block = 0; block < BlockCount; ++block)
            {
                if ((_blocks[block] = init[block]) != 0)
                    _blocksMask[GetUpdateFieldBlockIndex(block)] |= GetUpdateFieldBlockFlag(block);
                else
                    _blocksMask[GetUpdateFieldBlockIndex(block)] &= ~GetUpdateFieldBlockFlag(block);
            }
        }

        constexpr uint32 GetBlocksMask(uint32 index) const
        {
            return _blocksMask[index];
        }

        constexpr uint32 GetBlock(uint32 index) const
        {
            return _blocks[index];
        }

        constexpr bool operator[](uint32 index) const
        {
            return (_blocks[GetUpdateFieldBlockIndex(index)] & GetUpdateFieldBlockFlag(index)) != 0;
        }

        constexpr bool IsAnySet() const
        {
            return std::ranges::any_of(_blocksMask, [](uint32 blockMask)
            {
                return blockMask != 0;
            });
        }

        constexpr void Reset(uint32 index)
        {
            std::size_t blockIndex = GetUpdateFieldBlockIndex(index);
            if (!(_blocks[blockIndex] &= ~GetUpdateFieldBlockFlag(index)))
                _blocksMask[GetUpdateFieldBlockIndex(blockIndex)] &= ~GetUpdateFieldBlockFlag(blockIndex);
        }

        constexpr void ResetAll()
        {
            _blocksMask = { };
            _blocks = { };
        }

        constexpr void Set(uint32 index)
        {
            std::size_t blockIndex = GetUpdateFieldBlockIndex(index);
            MANGOS_ASSERT(blockIndex < _blocks.size() && "UpdateFieldMask::Set OOB");
            _blocks[blockIndex] |= GetUpdateFieldBlockFlag(index);
            _blocksMask[GetUpdateFieldBlockIndex(blockIndex)] |= GetUpdateFieldBlockFlag(blockIndex);
        }

        constexpr void SetAll()
        {
            // _blocksMask holds one bit per block (BlockCount of them); _blocks holds one bit per
            // field (Bits of them). Each array's last word only has that many bits valid.
            std::memset(_blocksMask.data(), 0xFF, _blocksMask.size() * sizeof(typename decltype(_blocksMask)::value_type));
            if constexpr (BlockCount % 32)
            {
                constexpr uint32 unused = 32 - (BlockCount % 32);
                _blocksMask.back() &= (0xFFFFFFFF >> unused);
            }
            std::memset(_blocks.data(), 0xFF, _blocks.size() * sizeof(typename decltype(_blocks)::value_type));
            if constexpr (Bits % 32)
            {
                constexpr uint32 unused = 32 - (Bits % 32);
                _blocks.back() &= (0xFFFFFFFF >> unused);
            }
        }

        constexpr UpdateFieldMask& operator&=(UpdateFieldMask const& right)
        {
            for (uint32 i = 0; i < BlocksMaskCount; ++i)
                _blocksMask[i] &= right._blocksMask[i];

            for (uint32 i = 0; i < BlockCount; ++i)
                if (!(_blocks[i] &= right._blocks[i]))
                    _blocksMask[GetUpdateFieldBlockIndex(i)] &= ~GetUpdateFieldBlockFlag(i);

            return *this;
        }

        constexpr UpdateFieldMask& operator|=(UpdateFieldMask const& right)
        {
            for (std::size_t i = 0; i < BlocksMaskCount; ++i)
                _blocksMask[i] |= right._blocksMask[i];

            for (std::size_t i = 0; i < BlockCount; ++i)
                _blocks[i] |= right._blocks[i];

            return *this;
        }

    private:
        std::array<uint32, BlocksMaskCount> _blocksMask;
        std::array<uint32, BlockCount> _blocks;
};

template<uint32 Bits>
UpdateFieldMask<Bits> operator&(UpdateFieldMask<Bits> const& left, UpdateFieldMask<Bits> const& right)
{
    UpdateFieldMask<Bits> result = left;
    result &= right;
    return result;
}

template<uint32 Bits>
UpdateFieldMask<Bits> operator|(UpdateFieldMask<Bits> const& left, UpdateFieldMask<Bits> const& right)
{
    UpdateFieldMask<Bits> result = left;
    result |= right;
    return result;
}

#endif // UPDATEFIELDMASK_H
