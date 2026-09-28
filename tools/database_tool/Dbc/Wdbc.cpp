#include "Wdbc.h"

#include <cstring>
#include <fstream>
#include <stdexcept>

namespace
{
    class Cursor
    {
    public:
        explicit Cursor(std::vector<uint8_t> const& data) : m_data(data), m_pos(0) {}

        template <typename T>
        T Read()
        {
            Require(sizeof(T));
            T value;
            std::memcpy(&value, m_data.data() + m_pos, sizeof(T));
            m_pos += sizeof(T);
            return value;
        }

        void ReadRaw(void* dest, size_t size)
        {
            Require(size);
            std::memcpy(dest, m_data.data() + m_pos, size);
            m_pos += size;
        }

    private:
        void Require(size_t size) const
        {
            if (m_pos + size > m_data.size())
                throw std::runtime_error("unexpected end of file");
        }

        std::vector<uint8_t> const& m_data;
        size_t m_pos;
    };

    int FieldWidthBytes(DbcFieldMeta const& field)
    {
        if (field.type == DbcFieldType::Float)
            return 4;
        if (field.type == DbcFieldType::String || field.type == DbcFieldType::LocString)
            return 4;
        return field.bits ? field.bits / 8 : 4;
    }

    // A locstring field occupies `layout.locStringSlots` row columns instead of one; every other
    // field keeps its plain array length.
    uint32_t FieldSlotCount(DbcFieldMeta const& field, WdbcLayout const& layout)
    {
        uint32_t slotsPerElement = field.type == DbcFieldType::LocString ? layout.locStringSlots : 1;
        return static_cast<uint32_t>(field.arrayLength) * slotsPerElement;
    }

    uint32_t TotalColumnCount(DbcTableMeta const& table, WdbcLayout const& layout)
    {
        uint32_t total = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
            total += FieldSlotCount(table.fields[i], layout);
        return total;
    }

    std::vector<uint32_t> FieldFlattenedStart(DbcTableMeta const& table, WdbcLayout const& layout)
    {
        std::vector<uint32_t> starts(table.fieldCount);
        uint32_t offset = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            starts[i] = offset;
            offset += FieldSlotCount(table.fields[i], layout);
        }
        return starts;
    }

    // Within a locstring field's slots, the last one is the flags mask once there is more than
    // one slot (1.12's 8 locale offsets + mask); otherwise every slot is a string offset.
    bool IsLocStringMaskSlot(DbcFieldMeta const& field, WdbcLayout const& layout, uint32_t slotInField)
    {
        return field.type == DbcFieldType::LocString && layout.locStringSlots > 1 &&
               (slotInField % layout.locStringSlots) == layout.locStringSlots - 1;
    }

    std::string ReadCString(uint8_t const* data, size_t size, size_t offset)
    {
        if (offset >= size)
            throw std::runtime_error("string offset out of range");
        size_t end = offset;
        while (end < size && data[end] != 0)
            ++end;
        if (end >= size)
            throw std::runtime_error("unterminated string");
        return std::string(reinterpret_cast<char const*>(data + offset), end - offset);
    }

    int64_t NarrowSigned(uint64_t raw, int bits)
    {
        switch (bits)
        {
            case 8: return static_cast<int64_t>(static_cast<int8_t>(static_cast<uint8_t>(raw)));
            case 16: return static_cast<int64_t>(static_cast<int16_t>(static_cast<uint16_t>(raw)));
            case 32: return static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(raw)));
            default: return static_cast<int64_t>(raw);
        }
    }
}

WdbcReadResult ReadWdbcFile(std::filesystem::path const& path, DbcTableMeta const& table, WdbcLayout const& layout)
{
    WdbcReadResult result;

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        result.error = "cannot open " + path.string();
        return result;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try
    {
        Cursor cursor(data);
        uint32_t signature = cursor.Read<uint32_t>();
        if (signature != 0x43424457u) // 'WDBC'
            throw std::runtime_error("not a WDBC file");

        uint32_t recordCount = cursor.Read<uint32_t>();
        uint32_t fieldCount = cursor.Read<uint32_t>();
        uint32_t recordSize = cursor.Read<uint32_t>();
        uint32_t stringBlockSize = cursor.Read<uint32_t>();

        uint32_t totalColumns = TotalColumnCount(table, layout);
        if (fieldCount != totalColumns)
            throw std::runtime_error("field count mismatch: file has " + std::to_string(fieldCount) +
                                      ", metadata expects " + std::to_string(totalColumns) + " fields");

        std::vector<uint32_t> flattenedStart = FieldFlattenedStart(table, layout);

        uint32_t expectedRecordSize = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
            expectedRecordSize +=
                static_cast<uint32_t>(FieldWidthBytes(table.fields[i])) * FieldSlotCount(table.fields[i], layout);
        if (recordSize != expectedRecordSize)
            throw std::runtime_error("record size mismatch: file has " + std::to_string(recordSize) +
                                      ", metadata expects " + std::to_string(expectedRecordSize));

        size_t recordAreaStart = 20; // header size
        size_t recordAreaSize = static_cast<size_t>(recordSize) * recordCount;
        size_t stringBlockStart = recordAreaStart + recordAreaSize;
        if (stringBlockStart + stringBlockSize > data.size())
            throw std::runtime_error("file is smaller than its declared record and string block size");

        uint8_t const* stringBlock = data.data() + stringBlockStart;

        for (uint32_t r = 0; r < recordCount; ++r)
        {
            uint8_t const* record = data.data() + recordAreaStart + static_cast<size_t>(r) * recordSize;
            size_t offset = 0;

            Wdc5Row row(totalColumns);
            for (uint32_t fieldIndex = 0; fieldIndex < table.fieldCount; ++fieldIndex)
            {
                DbcFieldMeta const& field = table.fields[fieldIndex];
                int width = FieldWidthBytes(field);
                uint32_t slotCount = FieldSlotCount(field, layout);

                for (uint32_t a = 0; a < slotCount; ++a)
                {
                    Wdc5Value value;
                    if (IsLocStringMaskSlot(field, layout, a))
                    {
                        uint32_t mask;
                        std::memcpy(&mask, record + offset, sizeof(uint32_t));
                        value.intValue = static_cast<int64_t>(mask);
                    }
                    else if (field.type == DbcFieldType::Float)
                    {
                        float f;
                        std::memcpy(&f, record + offset, sizeof(float));
                        value.floatValue = f;
                    }
                    else if (field.type == DbcFieldType::String || field.type == DbcFieldType::LocString)
                    {
                        uint32_t stringOffset;
                        std::memcpy(&stringOffset, record + offset, sizeof(uint32_t));
                        value.stringValue = stringOffset ? ReadCString(stringBlock, stringBlockSize, stringOffset) : std::string();
                    }
                    else
                    {
                        uint64_t raw = 0;
                        std::memcpy(&raw, record + offset, static_cast<size_t>(width));
                        value.intValue = field.isSigned ? NarrowSigned(raw, width * 8) : static_cast<int64_t>(raw);
                    }
                    row[flattenedStart[fieldIndex] + a] = value;
                    offset += static_cast<size_t>(width);
                }
            }
            result.rows.push_back(std::move(row));
        }
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
        result.rows.clear();
    }

    return result;
}

WdbcWriteResult WriteWdbcFile(DbcTableMeta const& table, std::vector<Wdc5Row> const& rows, WdbcLayout const& layout)
{
    WdbcWriteResult result;

    std::vector<uint32_t> flattenedStart = FieldFlattenedStart(table, layout);
    uint32_t totalColumns = TotalColumnCount(table, layout);

    uint32_t recordSize = 0;
    for (uint32_t i = 0; i < table.fieldCount; ++i)
        recordSize += static_cast<uint32_t>(FieldWidthBytes(table.fields[i])) * FieldSlotCount(table.fields[i], layout);

    std::vector<uint8_t> recordArea(static_cast<size_t>(recordSize) * rows.size(), 0);
    std::string stringBlock;
    stringBlock += '\0'; // offset 0 is reserved for "no string"

    for (size_t r = 0; r < rows.size(); ++r)
    {
        Wdc5Row const& row = rows[r];
        uint8_t* record = recordArea.data() + r * recordSize;
        size_t offset = 0;

        for (uint32_t fieldIndex = 0; fieldIndex < table.fieldCount; ++fieldIndex)
        {
            DbcFieldMeta const& field = table.fields[fieldIndex];
            int width = FieldWidthBytes(field);
            uint32_t slotCount = FieldSlotCount(field, layout);

            for (uint32_t a = 0; a < slotCount; ++a)
            {
                Wdc5Value const& value = row[flattenedStart[fieldIndex] + a];
                if (IsLocStringMaskSlot(field, layout, a))
                {
                    uint32_t mask = static_cast<uint32_t>(value.intValue);
                    std::memcpy(record + offset, &mask, sizeof(uint32_t));
                }
                else if (field.type == DbcFieldType::Float)
                {
                    float f = static_cast<float>(value.floatValue);
                    std::memcpy(record + offset, &f, sizeof(float));
                }
                else if (field.type == DbcFieldType::String || field.type == DbcFieldType::LocString)
                {
                    uint32_t stringOffset = 0;
                    if (!value.stringValue.empty())
                    {
                        stringOffset = static_cast<uint32_t>(stringBlock.size());
                        stringBlock += value.stringValue;
                        stringBlock += '\0';
                    }
                    std::memcpy(record + offset, &stringOffset, sizeof(uint32_t));
                }
                else
                {
                    uint64_t raw = static_cast<uint64_t>(value.intValue);
                    std::memcpy(record + offset, &raw, static_cast<size_t>(width));
                }
                offset += static_cast<size_t>(width);
            }
        }
    }

    std::vector<uint8_t>& file = result.bytes;
    auto putU32 = [&file](uint32_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); file.insert(file.end(), p, p + 4); };

    putU32(0x43424457u); // 'WDBC'
    putU32(static_cast<uint32_t>(rows.size()));
    putU32(totalColumns);
    putU32(recordSize);
    putU32(static_cast<uint32_t>(stringBlock.size()));
    file.insert(file.end(), recordArea.begin(), recordArea.end());
    file.insert(file.end(), stringBlock.begin(), stringBlock.end());

    return result;
}
