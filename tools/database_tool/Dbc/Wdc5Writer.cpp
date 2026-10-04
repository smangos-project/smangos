#include "Wdc5Writer.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <numeric>

namespace
{
    void PutU16(std::vector<uint8_t>& buf, uint16_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 2); }
    void PutI16(std::vector<uint8_t>& buf, int16_t v) { PutU16(buf, static_cast<uint16_t>(v)); }
    void PutU32(std::vector<uint8_t>& buf, uint32_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 4); }
    void PutU64(std::vector<uint8_t>& buf, uint64_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 8); }
    void PutF32(std::vector<uint8_t>& buf, float v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 4); }
    void PatchU32(std::vector<uint8_t>& buf, size_t pos, uint32_t v) { std::memcpy(buf.data() + pos, &v, 4); }

    int FieldWidthBytes(DbcFieldMeta const& field)
    {
        if (field.type == DbcFieldType::Float)
            return 4;
        if (field.type == DbcFieldType::String || field.type == DbcFieldType::LocString)
            return 4; // relative string-table offset
        return field.bits ? field.bits / 8 : 4;
    }

    std::vector<uint32_t> FieldFlattenedStart(DbcTableMeta const& table)
    {
        std::vector<uint32_t> starts(table.fieldCount);
        uint32_t offset = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            starts[i] = offset;
            offset += table.fields[i].arrayLength;
        }
        return starts;
    }

    struct FieldLookup
    {
        int32_t idFieldIndex = -1;
        int32_t relationFieldIndex = -1;
        bool idNonInline = false;
        bool relationNonInline = false;
        std::vector<uint32_t> flattenedStart;
    };

    FieldLookup ComputeFieldLookup(DbcTableMeta const& table)
    {
        FieldLookup lookup;
        lookup.flattenedStart = FieldFlattenedStart(table);
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            if (table.fields[i].isId)
                lookup.idFieldIndex = static_cast<int32_t>(i);
            if (table.fields[i].isRelation)
                lookup.relationFieldIndex = static_cast<int32_t>(i);
        }
        lookup.idNonInline = lookup.idFieldIndex >= 0 && table.fields[lookup.idFieldIndex].isNonInline;
        lookup.relationNonInline = lookup.relationFieldIndex >= 0 && table.fields[lookup.relationFieldIndex].isNonInline;
        return lookup;
    }

    uint32_t RowId(Wdc5Row const& row, FieldLookup const& lookup)
    {
        return lookup.idFieldIndex >= 0
            ? static_cast<uint32_t>(row[lookup.flattenedStart[lookup.idFieldIndex]].intValue)
            : 0;
    }

    // Rows sorted by id ascending (dbc-formats.md).
    std::vector<uint32_t> SortedRowOrder(std::vector<Wdc5Row> const& rows, FieldLookup const& lookup)
    {
        std::vector<uint32_t> order(rows.size());
        std::iota(order.begin(), order.end(), 0u);
        std::sort(order.begin(), order.end(),
            [&](uint32_t a, uint32_t b) { return RowId(rows[a], lookup) < RowId(rows[b], lookup); });
        return order;
    }

    void AppendHeaderSchema(std::vector<uint8_t>& buf, std::string const& schema)
    {
        std::string truncated = schema.substr(0, 128);
        buf.insert(buf.end(), truncated.begin(), truncated.end());
        buf.insert(buf.end(), 128 - truncated.size(), 0);
    }

    // Writes `value` at `record[fieldOffset..)` using `width` bytes (regular, None-compressed
    // layout) -- the field's *effective* width, which for an int may be wider than its nominal
    // metadata width (see EffectiveWidthBytes); string values are appended to `stringTable` and
    // written as a relative offset.
    void WriteFieldValue(std::vector<uint8_t>& record, size_t fieldOffset, DbcFieldMeta const& field, int width,
                          Wdc5Value const& value, std::string& stringTable, size_t recordAreaSize,
                          size_t recordIndex, size_t recordSize)
    {
        if (field.type == DbcFieldType::Float)
        {
            float f = static_cast<float>(value.floatValue);
            std::memcpy(record.data() + fieldOffset, &f, 4);
        }
        else if (field.type == DbcFieldType::String || field.type == DbcFieldType::LocString)
        {
            size_t absoluteFieldPos = recordIndex * recordSize + fieldOffset;
            size_t absoluteStringPos = recordAreaSize + stringTable.size();
            uint32_t relOffset = value.stringValue.empty() ? 0 : static_cast<uint32_t>(absoluteStringPos - absoluteFieldPos);
            std::memcpy(record.data() + fieldOffset, &relOffset, 4);
            if (!value.stringValue.empty())
            {
                stringTable += value.stringValue;
                stringTable += '\0';
            }
        }
        else
        {
            uint64_t raw = static_cast<uint64_t>(value.intValue);
            std::memcpy(record.data() + fieldOffset, &raw, static_cast<size_t>(width));
        }
    }

    struct InRecordField
    {
        uint32_t fieldIndex;
        size_t byteOffset;
        int width; // per element
    };

    // A compressed source field (Immediate/Pallet/CommonData) is not bound to its nominal
    // declared width at all -- e.g. a Pallet index just selects an arbitrary stored uint32, so a
    // field WoWDBDefs calls "8 bits" can legitimately decode to a value that doesn't fit 8 bits.
    // Writing None-compression at the nominal width would silently truncate that; this widens
    // such a field (for every element, so the record layout stays uniform) to whatever power-of-
    // two width actually holds every value seen for it across `rows`.
    int EffectiveWidthBytes(DbcFieldMeta const& field, std::vector<Wdc5Row> const& rows, uint32_t flatColumn)
    {
        int width = FieldWidthBytes(field);
        if (field.type != DbcFieldType::Int)
            return width;

        for (Wdc5Row const& row : rows)
        {
            for (uint16_t a = 0; a < field.arrayLength; ++a)
            {
                int64_t v = row[flatColumn + a].intValue;
                while (width < 8)
                {
                    int bits = width * 8;
                    bool fits = field.isSigned
                        ? (v >= -(int64_t(1) << (bits - 1)) && v < (int64_t(1) << (bits - 1)))
                        : (v >= 0 && v < (int64_t(1) << bits));
                    if (fits)
                        break;
                    width *= 2;
                }
            }
        }
        return width;
    }

    std::vector<InRecordField> ComputeInRecordFields(DbcTableMeta const& table, FieldLookup const& lookup,
                                                       std::vector<Wdc5Row> const& rows, size_t& recordSize)
    {
        std::vector<InRecordField> fields;
        size_t offset = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            DbcFieldMeta const& field = table.fields[i];
            if (field.isNonInline)
                continue;
            int width = EffectiveWidthBytes(field, rows, lookup.flattenedStart[i]);
            fields.push_back({ i, offset, width });
            offset += static_cast<size_t>(width) * field.arrayLength;
        }
        recordSize = offset;
        return fields;
    }

    // The client misreads scalar fields stored without compression, so ints and floats are
    // written as Immediate (bit-packed); arrays and strings stay uncompressed.
    bool IsImmediateField(DbcFieldMeta const& field, int width)
    {
        bool numeric = field.type == DbcFieldType::Int || field.type == DbcFieldType::Float;
        return numeric && field.arrayLength == 1 && width <= 4;
    }

    Wdc5WriteResult WriteRegular(DbcTableMeta const& table, Wdc5FileInfo const& info, std::vector<Wdc5Row> const& rows)
    {
        FieldLookup lookup = ComputeFieldLookup(table);
        std::vector<uint32_t> order = SortedRowOrder(rows, lookup);

        size_t recordSize = 0;
        std::vector<InRecordField> inRecordFields = ComputeInRecordFields(table, lookup, rows, recordSize);

        std::vector<uint8_t> recordArea(recordSize * rows.size(), 0);
        std::string stringTable;
        size_t recordAreaSize = recordArea.size();

        for (size_t r = 0; r < order.size(); ++r)
        {
            Wdc5Row const& row = rows[order[r]];
            uint8_t* record = recordArea.data() + r * recordSize;
            std::vector<uint8_t> recordBuf(record, record + recordSize);
            for (InRecordField const& f : inRecordFields)
            {
                DbcFieldMeta const& field = table.fields[f.fieldIndex];
                for (uint16_t a = 0; a < field.arrayLength; ++a)
                {
                    size_t elementOffset = f.byteOffset + static_cast<size_t>(f.width) * a;
                    WriteFieldValue(recordBuf, elementOffset, field, f.width, row[lookup.flattenedStart[f.fieldIndex] + a],
                                     stringTable, recordAreaSize, r, recordSize);
                }
            }
            std::memcpy(record, recordBuf.data(), recordSize);
        }

        if (stringTable.empty())
            stringTable.assign(2, '\0'); // the client's files always carry a string block

        uint32_t minId = rows.empty() ? 0 : RowId(rows[order.front()], lookup);
        uint32_t maxId = rows.empty() ? 0 : RowId(rows[order.back()], lookup);

        int16_t indexField = lookup.idNonInline ? 0 : -1; // the client's files write 0 for a non-inline id
        if (!lookup.idNonInline && lookup.idFieldIndex >= 0)
            for (size_t i = 0; i < inRecordFields.size(); ++i)
                if (inRecordFields[i].fieldIndex == static_cast<uint32_t>(lookup.idFieldIndex))
                    indexField = static_cast<int16_t>(i);

        size_t packedStart = recordSize;
        for (InRecordField const& f : inRecordFields)
            if (IsImmediateField(table.fields[f.fieldIndex], f.width))
                packedStart = std::min(packedStart, f.byteOffset);

        std::vector<uint8_t> file;
        uint32_t fieldCount = static_cast<uint32_t>(inRecordFields.size());
        uint16_t flags = static_cast<uint16_t>(info.flags & ~uint16_t(0x1)); // never sparse on this path

        PutU32(file, 0x35434457u);
        PutU32(file, 5);
        AppendHeaderSchema(file, info.schema);
        PutU32(file, static_cast<uint32_t>(rows.size()));  // recordCount
        PutU32(file, fieldCount);
        PutU32(file, static_cast<uint32_t>(recordSize));
        PutU32(file, static_cast<uint32_t>(stringTable.size()));
        PutU32(file, info.tableHash);
        PutU32(file, info.layoutHash);
        PutU32(file, minId);
        PutU32(file, maxId);
        PutU32(file, info.locale);
        PutU16(file, flags);
        PutI16(file, indexField);
        PutU32(file, fieldCount);            // totalFieldCount
        PutU32(file, static_cast<uint32_t>(packedStart)); // packedDataOffset
        PutU32(file, lookup.relationNonInline ? 1u : 0u); // parentLookupCount
        PutU32(file, fieldCount * 24);        // columnMetaSize
        PutU32(file, 0);                      // commonDataSize
        PutU32(file, 0);                      // palletDataSize
        PutU32(file, 1);                      // sectionCount

        uint32_t idTableSize = lookup.idNonInline ? static_cast<uint32_t>(4 * rows.size()) : 0;
        uint32_t parentLookupDataSize = lookup.relationNonInline
            ? static_cast<uint32_t>(12 + 8 * rows.size())
            : 0;

        size_t sectionPos = file.size();
        PutU64(file, 0); // tactId
        PutU32(file, 0); // fileOffset, patched below
        PutU32(file, static_cast<uint32_t>(rows.size()));
        PutU32(file, static_cast<uint32_t>(stringTable.size()));
        PutU32(file, 0); // catalogDataOffset (unused, regular)
        PutU32(file, idTableSize);
        PutU32(file, parentLookupDataSize);
        PutU32(file, 0); // catalogDataCount
        PutU32(file, 0); // copyTableCount

        for (InRecordField const& f : inRecordFields)
        {
            DbcFieldMeta const& field = table.fields[f.fieldIndex];
            if (IsImmediateField(field, f.width))
            {
                PutI16(file, 32);
                PutU16(file, static_cast<uint16_t>(packedStart));
            }
            else
            {
                PutI16(file, static_cast<int16_t>(std::max(0, 32 - f.width * 8)));
                PutU16(file, static_cast<uint16_t>(f.byteOffset));
            }
        }

        for (InRecordField const& f : inRecordFields)
        {
            DbcFieldMeta const& field = table.fields[f.fieldIndex];
            PutU16(file, static_cast<uint16_t>(f.byteOffset * 8));
            PutU16(file, static_cast<uint16_t>(f.width * 8 * field.arrayLength));
            PutU32(file, 0); // additionalDataSize
            if (IsImmediateField(field, f.width))
            {
                PutU32(file, 1); // compressionType: Immediate
                PutU32(file, static_cast<uint32_t>((f.byteOffset - packedStart) * 8));
                PutU32(file, static_cast<uint32_t>(f.width * 8));
                PutU32(file, field.isSigned ? 1u : 0u);
            }
            else
            {
                PutU32(file, 0); // compressionType: None
                PutU32(file, 0);
                PutU32(file, 0);
                PutU32(file, 0);
            }
        }

        uint32_t fileOffset = static_cast<uint32_t>(file.size());
        PatchU32(file, sectionPos + 8, fileOffset);
        file.insert(file.end(), recordArea.begin(), recordArea.end());
        file.insert(file.end(), stringTable.begin(), stringTable.end());

        if (lookup.idNonInline)
            for (uint32_t index : order)
                PutU32(file, RowId(rows[index], lookup));

        if (lookup.relationNonInline)
        {
            std::vector<uint32_t> parents;
            for (uint32_t index : order)
                parents.push_back(static_cast<uint32_t>(
                    rows[index][lookup.flattenedStart[lookup.relationFieldIndex]].intValue));
            PutU32(file, static_cast<uint32_t>(rows.size())); // NumEntries
            PutU32(file, parents.empty() ? 0 : *std::min_element(parents.begin(), parents.end()));
            PutU32(file, parents.empty() ? 0 : *std::max_element(parents.begin(), parents.end()));
            for (uint32_t r = 0; r < parents.size(); ++r)
            {
                PutU32(file, parents[r]);
                PutU32(file, r);
            }
        }

        Wdc5WriteResult result;
        result.bytes = std::move(file);
        return result;
    }

    Wdc5WriteResult WriteSparse(DbcTableMeta const& table, Wdc5FileInfo const& info, std::vector<Wdc5Row> const& rows)
    {
        FieldLookup lookup = ComputeFieldLookup(table);
        if (lookup.relationNonInline)
        {
            Wdc5WriteResult result;
            result.error = "sparse layout with a non-inline relation field isn't supported";
            return result;
        }

        std::vector<uint32_t> order = SortedRowOrder(rows, lookup);

        std::vector<InRecordField> inRecordFields;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            if (!table.fields[i].isNonInline)
                inRecordFields.push_back({ i, 0, EffectiveWidthBytes(table.fields[i], rows, lookup.flattenedStart[i]) });
        }

        std::vector<std::vector<uint8_t>> records(rows.size());
        for (size_t r = 0; r < order.size(); ++r)
        {
            Wdc5Row const& row = rows[order[r]];
            std::vector<uint8_t>& record = records[r];
            for (InRecordField const& f : inRecordFields)
            {
                DbcFieldMeta const& field = table.fields[f.fieldIndex];
                for (uint16_t a = 0; a < field.arrayLength; ++a)
                {
                    Wdc5Value const& value = row[lookup.flattenedStart[f.fieldIndex] + a];
                    if (field.type == DbcFieldType::Float)
                    {
                        float fv = static_cast<float>(value.floatValue);
                        auto p = reinterpret_cast<uint8_t const*>(&fv);
                        record.insert(record.end(), p, p + 4);
                    }
                    else if (field.type == DbcFieldType::String || field.type == DbcFieldType::LocString)
                    {
                        record.insert(record.end(), value.stringValue.begin(), value.stringValue.end());
                        record.push_back(0);
                    }
                    else
                    {
                        uint64_t raw = static_cast<uint64_t>(value.intValue);
                        auto p = reinterpret_cast<uint8_t const*>(&raw);
                        record.insert(record.end(), p, p + f.width);
                    }
                }
            }
            record.push_back(0); // the client's records end in a padding byte
        }

        uint32_t minId = rows.empty() ? 0 : RowId(rows[order.front()], lookup);
        uint32_t maxId = rows.empty() ? 0 : RowId(rows[order.back()], lookup);
        uint32_t fieldCount = static_cast<uint32_t>(inRecordFields.size());
        uint32_t rowCount = static_cast<uint32_t>(rows.size());

        // Slot width of each field in the client's own sparse files: strings and floats take 32
        // bits, integers their real width; fields are packed back to back.
        std::vector<uint32_t> slotBits;
        uint32_t slotBytes = 0;
        for (InRecordField const& f : inRecordFields)
        {
            DbcFieldMeta const& field = table.fields[f.fieldIndex];
            bool const wide = field.type != DbcFieldType::Int;
            slotBits.push_back(wide ? 32 : f.width * 8);
            slotBytes += slotBits.back() / 8;
        }

        std::vector<uint8_t> file;
        PutU32(file, 0x35434457u);
        PutU32(file, 5);
        AppendHeaderSchema(file, info.schema);
        PutU32(file, rowCount);
        PutU32(file, fieldCount);
        PutU32(file, slotBytes + 1); // recordSize
        PutU32(file, 0);             // stringTableSize: strings are inline
        PutU32(file, info.tableHash);
        PutU32(file, info.layoutHash);
        PutU32(file, minId);
        PutU32(file, maxId);
        PutU32(file, info.locale);
        PutU16(file, info.flags); // already has the sparse bit set
        PutI16(file, 0);          // indexField
        PutU32(file, fieldCount);
        PutU32(file, slotBytes);  // packedDataOffset
        PutU32(file, 0);          // parentLookupCount
        PutU32(file, fieldCount * 24); // fieldStorageInfoSize
        PutU32(file, 0);
        PutU32(file, 0);
        PutU32(file, 1); // sectionCount

        size_t sectionPos = file.size();
        PutU64(file, 0); // tactId
        PutU32(file, 0); // fileOffset, patched below
        PutU32(file, rowCount);
        PutU32(file, 0); // stringTableSize
        PutU32(file, 0); // offsetRecordsEndOffset, patched below
        PutU32(file, rowCount * 4); // idListSize
        PutU32(file, 0); // parentLookupDataSize
        PutU32(file, rowCount); // offsetMapIdCount
        PutU32(file, 0); // copyTableCount

        uint32_t byteOffset = 0;
        for (size_t i = 0; i < inRecordFields.size(); ++i)
        {
            PutI16(file, static_cast<int16_t>(32 - slotBits[i]));
            PutU16(file, static_cast<uint16_t>(byteOffset));
            byteOffset += slotBits[i] / 8;
        }

        uint32_t bitOffset = 0;
        for (size_t i = 0; i < inRecordFields.size(); ++i)
        {
            PutU16(file, static_cast<uint16_t>(bitOffset));
            PutU16(file, static_cast<uint16_t>(slotBits[i]));
            for (int k = 0; k < 5; ++k)
                PutU32(file, 0); // additionalDataSize, storageType (none), three unused
            bitOffset += slotBits[i];
        }

        uint32_t recordsStart = static_cast<uint32_t>(file.size());
        for (std::vector<uint8_t> const& record : records)
            file.insert(file.end(), record.begin(), record.end());

        uint32_t recordsEnd = static_cast<uint32_t>(file.size());
        for (uint32_t index : order)
            PutU32(file, RowId(rows[index], lookup));

        size_t recordFileOffset = recordsStart;
        for (std::vector<uint8_t> const& record : records)
        {
            PutU32(file, static_cast<uint32_t>(recordFileOffset));
            PutU16(file, static_cast<uint16_t>(record.size()));
            recordFileOffset += record.size();
        }

        for (uint32_t index : order)
            PutU32(file, RowId(rows[index], lookup));

        PatchU32(file, sectionPos + 8, recordsStart);
        PatchU32(file, sectionPos + 8 + 12, recordsEnd);

        Wdc5WriteResult result;
        result.bytes = std::move(file);
        return result;
    }
}

Wdc5WriteResult WriteWdc5File(DbcTableMeta const& table, Wdc5FileInfo const& info, std::vector<Wdc5Row> const& rows)
{
    if (info.flags & 0x1)
        return WriteSparse(table, info, rows);
    return WriteRegular(table, info, rows);
}
