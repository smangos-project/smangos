#include "Persistence/EpochBuffer.h"

#include "Errors.h"

namespace Persistence
{
    void RowWriter::BeginImage(uint16 table, uint64 owner)
    {
        MANGOS_DEBUG_ASSERT(!m_table && table < m_registry.Size());
        m_table = &m_registry.Get(table);
        m_image = m_buffer.images.size();
        uint32 const pos = uint32(m_buffer.bytes.size());
        m_buffer.images.push_back({ table, owner, pos, pos, 0 });
    }

    void RowWriter::EndImage()
    {
        MANGOS_DEBUG_ASSERT(m_table);
        m_buffer.images[m_image].end = uint32(m_buffer.bytes.size());
        m_table = nullptr;
    }

    RowWriter& RowWriter::BeginRow()
    {
        MANGOS_DEBUG_ASSERT(m_table);
        m_rowStart = m_buffer.bytes.size();
        m_buffer.bytes.resize(m_rowStart + sizeof(uint32));
        m_col = 0;
        return *this;
    }

    void RowWriter::EndRow()
    {
        MANGOS_DEBUG_ASSERT(m_col == m_table->cols.size());
        uint32 const size = uint32(m_buffer.bytes.size() - m_rowStart - sizeof(uint32));
        std::memcpy(m_buffer.bytes.data() + m_rowStart, &size, sizeof(size));
        ++m_buffer.images[m_image].rows;
    }

    void RowWriter::NextColumn(ColType type)
    {
        MANGOS_DEBUG_ASSERT(m_col < m_table->cols.size() && m_table->cols[m_col].type == type);
        if (m_table->cols[m_col++].nullable)
            m_buffer.bytes.push_back(1);
    }

    void RowWriter::PutVarint(uint64 value)
    {
        while (value >= 0x80)
        {
            m_buffer.bytes.push_back(uint8(value | 0x80));
            value >>= 7;
        }
        m_buffer.bytes.push_back(uint8(value));
    }

    void RowWriter::PutBytes(void const* data, size_t size)
    {
        uint8 const* bytes = static_cast<uint8 const*>(data);
        m_buffer.bytes.insert(m_buffer.bytes.end(), bytes, bytes + size);
    }

    RowWriter& RowWriter::Int(int64 value)
    {
        NextColumn(ColType::Int);
        PutVarint((uint64(value) << 1) ^ uint64(value >> 63));
        return *this;
    }

    RowWriter& RowWriter::Real(double value)
    {
        NextColumn(ColType::Real);
        PutBytes(&value, sizeof(value));
        return *this;
    }

    RowWriter& RowWriter::Text(std::string_view value)
    {
        NextColumn(ColType::Text);
        PutVarint(value.size());
        PutBytes(value.data(), value.size());
        return *this;
    }

    RowWriter& RowWriter::Blob(void const* data, size_t size)
    {
        NextColumn(ColType::Blob);
        PutVarint(size);
        PutBytes(data, size);
        return *this;
    }

    RowWriter& RowWriter::Null()
    {
        MANGOS_DEBUG_ASSERT(m_col < m_table->cols.size() && m_table->cols[m_col].nullable);
        ++m_col;
        m_buffer.bytes.push_back(0);
        return *this;
    }

    uint64 RowReader::GetVarint()
    {
        uint64 value = 0;
        for (uint32 shift = 0; m_pos < m_size; shift += 7)
        {
            uint8 const b = m_data[m_pos++];
            value |= uint64(b & 0x7F) << shift;
            if (!(b & 0x80))
                break;
        }
        return value;
    }

    bool RowReader::Next(Value& value)
    {
        if (m_col >= m_table.cols.size())
            return false;
        ColDesc const& col = m_table.cols[m_col++];
        value.type = col.type;
        value.null = col.nullable && m_data[m_pos++] == 0;
        if (value.null)
            return true;

        switch (col.type)
        {
            case ColType::Int:
            {
                uint64 const raw = GetVarint();
                value.i = int64(raw >> 1) ^ -int64(raw & 1);
                break;
            }
            case ColType::Real:
                std::memcpy(&value.r, m_data + m_pos, sizeof(value.r));
                m_pos += sizeof(value.r);
                break;
            case ColType::Text:
            case ColType::Blob:
            {
                size_t const size = size_t(GetVarint());
                value.bytes = std::string_view(reinterpret_cast<char const*>(m_data + m_pos), size);
                m_pos += size;
                break;
            }
        }
        return true;
    }
}
