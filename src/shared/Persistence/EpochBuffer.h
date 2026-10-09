#ifndef MANGOS_PERSISTENCE_EPOCHBUFFER_H
#define MANGOS_PERSISTENCE_EPOCHBUFFER_H

#include "Persistence/TableRegistry.h"

#include <chrono>
#include <cstring>
#include <string_view>
#include <vector>

namespace Persistence
{
    // All rows of one table that belong to one owner. An empty image means the owner has no rows.
    struct ImageRef
    {
        uint16 table;
        uint64 owner;
        uint32 begin;   // byte range in EpochBuffer::bytes
        uint32 end;
        uint32 rows;
    };

    // Rows captured by one thread. Row encoding: u32 payload length, then the values in descriptor
    // order. Nullable columns carry a 1-byte present flag. Int is a zigzag varint, Real 8 bytes,
    // Text/Blob a varint length and the bytes.
    struct EpochBuffer
    {
        std::vector<uint8> bytes;
        std::vector<ImageRef> images;

        void Clear() { bytes.clear(); images.clear(); }
    };

    // One capture pass, handed to the writer as a whole.
    struct Epoch
    {
        uint64 id = 0;
        uint64 captureUs = 0;
        std::chrono::steady_clock::time_point captured;
        std::vector<EpochBuffer> shards;
    };

    class RowWriter
    {
        public:
            RowWriter(TableRegistry const& registry, EpochBuffer& buffer) : m_registry(registry), m_buffer(buffer) {}

            void BeginImage(uint16 table, uint64 owner);
            void EndImage();

            RowWriter& BeginRow();
            RowWriter& Int(int64 value);
            RowWriter& Real(double value);
            RowWriter& Text(std::string_view value);
            RowWriter& Blob(void const* data, size_t size);
            RowWriter& Null();
            void EndRow();

        private:
            void NextColumn(ColType type);
            void PutVarint(uint64 value);
            void PutBytes(void const* data, size_t size);

            TableRegistry const& m_registry;
            EpochBuffer& m_buffer;
            TableDesc const* m_table = nullptr;
            size_t m_image = 0;
            size_t m_rowStart = 0;
            uint32 m_col = 0;
    };

    struct Value
    {
        ColType type;
        bool null;
        int64 i;
        double r;
        std::string_view bytes;
    };

    // Decodes the values of one row payload (without its length prefix).
    class RowReader
    {
        public:
            RowReader(TableDesc const& table, uint8 const* data, size_t size) : m_table(table), m_data(data), m_size(size) {}

            bool Next(Value& value);
            size_t Offset() const { return m_pos; }

        private:
            uint64 GetVarint();

            TableDesc const& m_table;
            uint8 const* m_data;
            size_t m_size;
            size_t m_pos = 0;
            uint32 m_col = 0;
    };

    // Calls f(payload, size) for every row of image.
    template <class F>
    void ForEachRow(EpochBuffer const& buffer, ImageRef const& image, F&& f)
    {
        for (uint32 pos = image.begin; pos < image.end;)
        {
            uint32 size;
            std::memcpy(&size, buffer.bytes.data() + pos, sizeof(size));
            pos += sizeof(size);
            f(buffer.bytes.data() + pos, size_t(size));
            pos += size;
        }
    }

    inline uint64 Fnv1a(void const* data, size_t size, uint64 hash = 14695981039346656037ull)
    {
        for (size_t i = 0; i < size; ++i)
            hash = (hash ^ static_cast<uint8 const*>(data)[i]) * 1099511628211ull;
        return hash;
    }
}

#endif
