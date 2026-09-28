#include "Wdc5Reader.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace
{
    // Sequential/random reads over an in-memory copy of the file. WDC5 is little-endian and
    // this tool only ever runs on little-endian hosts, so no byte-swapping is needed.
    class Cursor
    {
    public:
        explicit Cursor(std::vector<uint8_t> const& data, size_t pos = 0) : m_data(data), m_pos(pos) {}

        size_t Position() const { return m_pos; }
        void Seek(size_t pos) { m_pos = pos; }

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

        void Skip(size_t size)
        {
            Require(size);
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

    struct RawHeader
    {
        uint32_t signature;
        uint32_t version;
        std::array<char, 128> schema;
        uint32_t recordCount;
        uint32_t fieldCount;
        uint32_t recordSize;
        uint32_t stringTableSize;
        uint32_t tableHash;
        uint32_t layoutHash;
        uint32_t minId;
        uint32_t maxId;
        uint32_t locale;
        uint16_t flags;
        int16_t indexField;
        uint32_t totalFieldCount;
        uint32_t packedDataOffset;
        uint32_t parentLookupCount;
        uint32_t columnMetaSize;
        uint32_t commonDataSize;
        uint32_t palletDataSize;
        uint32_t sectionCount;
    };

    struct RawSection
    {
        uint64_t tactId;
        uint32_t fileOffset;
        uint32_t recordCount;
        uint32_t stringTableSize;
        uint32_t catalogDataOffset;
        uint32_t idTableSize;
        uint32_t parentLookupDataSize;
        uint32_t catalogDataCount;
        uint32_t copyTableCount;
    };

    enum class ColumnCompression : uint32_t { None, Immediate, CommonData, Pallet, PalletArray, SignedImmediate };

    struct RawColumnMeta
    {
        uint16_t bitOffset;
        uint16_t bitSize;
        uint32_t additionalDataSize;
        ColumnCompression compressionType;
        uint32_t packed0; // immediate/pallet BitOffset, or commonData Value
        uint32_t packed1; // immediate/pallet BitWidth
        uint32_t packed2; // pallet ArraySize
    };

    struct RawCopy
    {
        uint32_t newRowId;
        uint32_t sourceRowId;
    };

    struct RawIndexEntry
    {
        uint32_t parentId;
        uint32_t recordIndex;
    };

    // A CommonData field's per-record value depends on the record's id, which for a non-inline
    // id field is only known once that section's id list has been read -- so decoding just
    // records where to patch the real value in later, once every row's id is final.
    struct PendingCommonData
    {
        size_t rowIndex;
        uint32_t columnIndex;
        uint32_t inlineFieldIndex;
        DbcFieldType fieldType;
    };

    RawHeader ReadHeader(Cursor& cursor)
    {
        RawHeader header{};
        header.signature = cursor.Read<uint32_t>();
        header.version = cursor.Read<uint32_t>();
        cursor.ReadRaw(header.schema.data(), header.schema.size());
        header.recordCount = cursor.Read<uint32_t>();
        header.fieldCount = cursor.Read<uint32_t>();
        header.recordSize = cursor.Read<uint32_t>();
        header.stringTableSize = cursor.Read<uint32_t>();
        header.tableHash = cursor.Read<uint32_t>();
        header.layoutHash = cursor.Read<uint32_t>();
        header.minId = cursor.Read<uint32_t>();
        header.maxId = cursor.Read<uint32_t>();
        header.locale = cursor.Read<uint32_t>();
        header.flags = cursor.Read<uint16_t>();
        header.indexField = cursor.Read<int16_t>();
        header.totalFieldCount = cursor.Read<uint32_t>();
        header.packedDataOffset = cursor.Read<uint32_t>();
        header.parentLookupCount = cursor.Read<uint32_t>();
        header.columnMetaSize = cursor.Read<uint32_t>();
        header.commonDataSize = cursor.Read<uint32_t>();
        header.palletDataSize = cursor.Read<uint32_t>();
        header.sectionCount = cursor.Read<uint32_t>();
        return header;
    }

    RawSection ReadSection(Cursor& cursor)
    {
        RawSection section{};
        section.tactId = cursor.Read<uint64_t>();
        section.fileOffset = cursor.Read<uint32_t>();
        section.recordCount = cursor.Read<uint32_t>();
        section.stringTableSize = cursor.Read<uint32_t>();
        section.catalogDataOffset = cursor.Read<uint32_t>();
        section.idTableSize = cursor.Read<uint32_t>();
        section.parentLookupDataSize = cursor.Read<uint32_t>();
        section.catalogDataCount = cursor.Read<uint32_t>();
        section.copyTableCount = cursor.Read<uint32_t>();
        return section;
    }

    RawColumnMeta ReadColumnMeta(Cursor& cursor)
    {
        RawColumnMeta meta{};
        meta.bitOffset = cursor.Read<uint16_t>();
        meta.bitSize = cursor.Read<uint16_t>();
        meta.additionalDataSize = cursor.Read<uint32_t>();
        meta.compressionType = static_cast<ColumnCompression>(cursor.Read<uint32_t>());
        meta.packed0 = cursor.Read<uint32_t>();
        meta.packed1 = cursor.Read<uint32_t>();
        meta.packed2 = cursor.Read<uint32_t>();
        return meta;
    }

    uint32_t InlineMetaFieldCount(DbcTableMeta const& table)
    {
        uint32_t count = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
            if (!table.fields[i].isNonInline)
                ++count;
        return count;
    }

    uint32_t TotalColumnCount(DbcTableMeta const& table)
    {
        uint32_t total = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
            total += table.fields[i].arrayLength;
        return total;
    }

    // Index (into the flattened per-record value vector) of a field's first element.
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

    bool HasLayoutHash(DbcTableMeta const& table, uint32_t layoutHash)
    {
        for (uint32_t i = 0; i < table.layoutHashCount; ++i)
            if (table.layoutHashes[i] == layoutHash)
                return true;
        return false;
    }

    // Reads up to 8 bytes starting at `byteOffset` (bounds-checked against `size`) and extracts
    // `bitWidth` bits starting at bit `bitOffsetInByte` of that little-endian window -- the same
    // extraction TrinityCore's RecordGetPackedValue performs, just bounds-safe instead of relying
    // on trailing pad bytes.
    uint64_t ReadPackedBits(uint8_t const* data, size_t size, size_t byteOffset, uint32_t bitOffsetInByte, uint32_t bitWidth)
    {
        uint64_t buffer = 0;
        size_t bytesNeeded = std::min<size_t>(8, (bitOffsetInByte + bitWidth + 7) / 8);
        size_t available = byteOffset < size ? std::min(bytesNeeded, size - byteOffset) : 0;
        if (available)
            std::memcpy(&buffer, data + byteOffset, available);

        uint64_t mask = bitWidth >= 64 ? ~uint64_t(0) : ((uint64_t(1) << bitWidth) - 1);
        return (buffer >> bitOffsetInByte) & mask;
    }

    // Sets exactly one of value.intValue/floatValue, matching the None-compression path: a Float
    // field's raw bits are reinterpreted, never left as a stray integer.
    void SetDecodedValue(Wdc5Value& value, DbcFieldType type, uint64_t raw)
    {
        if (type == DbcFieldType::Float)
        {
            uint32_t bits = static_cast<uint32_t>(raw);
            float f;
            std::memcpy(&f, &bits, sizeof(float));
            value.floatValue = f;
        }
        else
        {
            value.intValue = static_cast<int64_t>(raw);
        }
    }

    int64_t SignExtend(uint64_t value, uint32_t bitWidth)
    {
        if (bitWidth >= 64)
            return static_cast<int64_t>(value);
        uint64_t mask = uint64_t(1) << (bitWidth - 1);
        return static_cast<int64_t>((value ^ mask) - mask);
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

    // Everything needed to decode one inline field's value out of a regular-layout record.
    struct RegularFieldContext
    {
        DbcFieldMeta const* field = nullptr;
        RawColumnMeta columnMeta{};
        std::vector<uint32_t> const* palletValues = nullptr;      // Pallet
        std::vector<uint32_t> const* palletArrayValues = nullptr; // PalletArray
    };

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

    // Decodes one element (arrayIndex) of one inline field out of `record` (pointing at the
    // start of a fixed-size record within `buffer`, which also holds that record's own
    // section's trailing string table immediately after every record in the section).
    Wdc5Value DecodeRegularElement(RegularFieldContext const& ctx, uint8_t const* buffer, size_t bufferSize,
                                    uint8_t const* record, uint32_t arrayIndex, uint32_t packedDataOffset)
    {
        Wdc5Value value;
        DbcFieldMeta const& field = *ctx.field;
        // For None compression this is the file's own declared width, which is authoritative --
        // it can legitimately be wider than the field's nominal metadata width (e.g. a writer
        // that widened a field whose real values didn't fit that nominal width). Clamped to 8:
        // no integer column needs more, and `raw` below is a uint64_t -- an unclamped width from
        // a stray/oversized bitSize would memcpy past it.
        int elementBytes = field.type == DbcFieldType::Float ? 4
            : field.type == DbcFieldType::String || field.type == DbcFieldType::LocString ? 4
            : ctx.columnMeta.bitSize ? std::min<int>(ctx.columnMeta.bitSize / 8, 8)
            : (field.bits ? field.bits / 8 : 4);
        size_t recordOffset = static_cast<size_t>(record - buffer);

        switch (ctx.columnMeta.compressionType)
        {
            case ColumnCompression::None:
            {
                size_t fieldOffset = ctx.columnMeta.bitOffset / 8 + static_cast<size_t>(elementBytes) * arrayIndex;
                uint8_t const* p = record + fieldOffset;
                if (field.type == DbcFieldType::Float)
                {
                    float f;
                    std::memcpy(&f, p, sizeof(float));
                    value.floatValue = f;
                }
                else if (field.type == DbcFieldType::String || field.type == DbcFieldType::LocString)
                {
                    uint32_t relOffset;
                    std::memcpy(&relOffset, p, sizeof(uint32_t));
                    value.stringValue = relOffset ? ReadCString(buffer, bufferSize, recordOffset + fieldOffset + relOffset) : std::string();
                }
                else
                {
                    // Bounds-checked against the buffer too: a tiny file's last field can still
                    // have fewer bytes left than even a clamped elementBytes calls for (seen on
                    // a real file: OverrideSpellData.db2's last column, 46-byte section).
                    size_t absoluteOffset = recordOffset + fieldOffset;
                    size_t available = absoluteOffset < bufferSize ? bufferSize - absoluteOffset : 0;
                    size_t copyBytes = std::min({ static_cast<size_t>(elementBytes), sizeof(uint64_t), available });
                    uint64_t raw = 0;
                    std::memcpy(&raw, p, copyBytes);
                    value.intValue = field.isSigned ? NarrowSigned(raw, static_cast<int>(copyBytes) * 8)
                                                     : static_cast<int64_t>(raw);
                }
                break;
            }
            case ColumnCompression::Immediate:
            case ColumnCompression::SignedImmediate:
            {
                size_t byteOffset = recordOffset + ctx.columnMeta.packed0 / 8 + packedDataOffset;
                uint32_t bitOffsetInByte = ctx.columnMeta.packed0 & 7;
                uint64_t raw = ReadPackedBits(buffer, bufferSize, byteOffset, bitOffsetInByte, ctx.columnMeta.packed1);
                uint64_t decoded = ctx.columnMeta.compressionType == ColumnCompression::SignedImmediate
                    ? static_cast<uint64_t>(SignExtend(raw, ctx.columnMeta.packed1))
                    : raw;
                SetDecodedValue(value, field.type, decoded);
                break;
            }
            case ColumnCompression::CommonData:
                // The per-id override needs this record's id, which may not be known yet (a
                // non-inline id is only known once the whole section's id list is read). This
                // returns the column's default; ResolveCommonData patches in any override once
                // every row's id is final.
                SetDecodedValue(value, field.type, ctx.columnMeta.packed0);
                break;
            case ColumnCompression::Pallet:
            {
                size_t byteOffset = recordOffset + ctx.columnMeta.packed0 / 8 + packedDataOffset;
                uint32_t bitOffsetInByte = ctx.columnMeta.packed0 & 7;
                uint64_t index = ReadPackedBits(buffer, bufferSize, byteOffset, bitOffsetInByte, ctx.columnMeta.packed1);
                uint32_t raw = ctx.palletValues && index < ctx.palletValues->size() ? (*ctx.palletValues)[index] : 0;
                SetDecodedValue(value, field.type, raw);
                break;
            }
            case ColumnCompression::PalletArray:
            {
                size_t byteOffset = recordOffset + ctx.columnMeta.packed0 / 8 + packedDataOffset;
                uint32_t bitOffsetInByte = ctx.columnMeta.packed0 & 7;
                uint64_t index = ReadPackedBits(buffer, bufferSize, byteOffset, bitOffsetInByte, ctx.columnMeta.packed1);
                size_t flatIndex = static_cast<size_t>(index) * ctx.columnMeta.packed2 + arrayIndex;
                uint32_t raw = ctx.palletArrayValues && flatIndex < ctx.palletArrayValues->size() ? (*ctx.palletArrayValues)[flatIndex] : 0;
                SetDecodedValue(value, field.type, raw);
                break;
            }
        }

        return value;
    }
}

Wdc5ReadResult ReadWdc5File(std::filesystem::path const& path, DbcTableMeta const& table)
{
    Wdc5ReadResult result;

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
        RawHeader header = ReadHeader(cursor);

        if (header.signature != 0x35434457u) // 'WDC5'
            throw std::runtime_error("not a WDC5 file");
        if (header.version != 5)
            throw std::runtime_error("unsupported WDC5 version " + std::to_string(header.version));
        if (!HasLayoutHash(table, header.layoutHash))
            throw std::runtime_error("layout hash 0x" + [&] { std::ostringstream s; s << std::hex << header.layoutHash; return s.str(); }() +
                                      " is not one of this table's known layouts");

        int32_t idFieldIndex = -1, relationFieldIndex = -1;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            if (table.fields[i].isId)
                idFieldIndex = static_cast<int32_t>(i);
            if (table.fields[i].isRelation)
                relationFieldIndex = static_cast<int32_t>(i);
        }

        bool relationIsNonInline = relationFieldIndex >= 0 && table.fields[relationFieldIndex].isNonInline;

        // A non-inline id never has a file field slot. A non-inline relation sometimes still
        // does (its bytes are reserved space, patched from the parent lookup rather than read),
        // depending on the table -- so both possibilities are checked against what the file
        // actually declares (dbc-formats.md's own rule, "file field count == inline metadata
        // fields", covers the common case; this covers the rest).
        uint32_t withoutRelationSlot = InlineMetaFieldCount(table);
        uint32_t withRelationSlot = withoutRelationSlot + (relationIsNonInline ? 1 : 0);
        bool relationHasSlot;
        if (header.fieldCount == withoutRelationSlot)
            relationHasSlot = false;
        else if (relationIsNonInline && header.fieldCount == withRelationSlot)
            relationHasSlot = true;
        else
            throw std::runtime_error("field count mismatch: file has " + std::to_string(header.fieldCount) +
                                      ", metadata expects " + std::to_string(withoutRelationSlot) + " inline fields");

        result.info.tableHash = header.tableHash;
        result.info.layoutHash = header.layoutHash;
        result.info.locale = header.locale;
        result.info.flags = header.flags;
        {
            auto nul = std::find(header.schema.begin(), header.schema.end(), '\0');
            result.info.schema.assign(header.schema.begin(), nul);
        }

        bool sparse = (header.flags & 0x1) != 0;

        std::vector<RawSection> sections(header.sectionCount);
        for (uint32_t i = 0; i < header.sectionCount; ++i)
            sections[i] = ReadSection(cursor);

        // Plain field entries: only sparse mode needs these (UnusedBits gives each inline int
        // field's real byte width, which can differ from its nominal metadata width -- see
        // EffectiveWidthBytes in the writer); regular mode gets everything from columnMeta.
        std::vector<int16_t> fieldUnusedBits(header.fieldCount);
        for (uint32_t i = 0; i < header.fieldCount; ++i)
        {
            fieldUnusedBits[i] = cursor.Read<int16_t>();
            cursor.Skip(2); // Offset: unused by every known sparse reader, including this one
        }

        std::vector<RawColumnMeta> columnMeta;
        std::vector<std::vector<uint32_t>> palletValues(header.totalFieldCount);
        std::vector<std::vector<uint32_t>> palletArrayValues(header.totalFieldCount);
        std::vector<std::unordered_map<uint32_t, uint32_t>> commonValues(header.totalFieldCount);

        if (header.columnMetaSize)
        {
            columnMeta.resize(header.totalFieldCount);
            for (uint32_t i = 0; i < header.totalFieldCount; ++i)
                columnMeta[i] = ReadColumnMeta(cursor);

            for (uint32_t i = 0; i < header.totalFieldCount; ++i)
            {
                if (columnMeta[i].compressionType != ColumnCompression::Pallet)
                    continue;
                palletValues[i].resize(columnMeta[i].additionalDataSize / 4);
                for (uint32_t& v : palletValues[i])
                    v = cursor.Read<uint32_t>();
            }

            for (uint32_t i = 0; i < header.totalFieldCount; ++i)
            {
                if (columnMeta[i].compressionType != ColumnCompression::PalletArray)
                    continue;
                palletArrayValues[i].resize(columnMeta[i].additionalDataSize / 4);
                for (uint32_t& v : palletArrayValues[i])
                    v = cursor.Read<uint32_t>();
            }

            for (uint32_t i = 0; i < header.totalFieldCount; ++i)
            {
                if (columnMeta[i].compressionType != ColumnCompression::CommonData || !columnMeta[i].additionalDataSize)
                    continue;
                uint32_t count = columnMeta[i].additionalDataSize / 8;
                for (uint32_t r = 0; r < count; ++r)
                {
                    uint32_t recordId = cursor.Read<uint32_t>();
                    uint32_t value = cursor.Read<uint32_t>();
                    commonValues[i][recordId] = value;
                }
            }
        }

        // Per-section leading block: for every section with a nonzero TactId, a length-prefixed
        // list of "encrypted ids" this tool has no use for, encountered before any section's own
        // table data.
        for (RawSection const& section : sections)
        {
            if (section.tactId == 0)
                continue;
            uint32_t count = cursor.Read<uint32_t>();
            cursor.Skip(static_cast<size_t>(count) * 4);
        }

        std::vector<uint32_t> flattenedStart = FieldFlattenedStart(table);
        uint32_t totalColumns = TotalColumnCount(table);

        std::vector<uint32_t> idTable;      // global, concatenated across sections, non-inline id only
        std::vector<RawCopy> copyTable;     // global
        std::vector<std::vector<RawIndexEntry>> parentLookupBySection(header.sectionCount);

        std::vector<Wdc5Row> rowsById; // dense by row order; ids tracked alongside
        std::vector<uint32_t> rowIds;
        std::vector<PendingCommonData> pendingCommonData;
        rowsById.reserve(header.recordCount);
        rowIds.reserve(header.recordCount);

        // TrinityCore concatenates every known section's record data, then every known
        // section's string table, into one buffer: a string's relative offset can resolve
        // anywhere in that combined space, not just within its own section (real multi-section
        // files, e.g. UiTextureAtlasMember, rely on this).
        std::vector<uint8_t> combinedData;
        std::vector<size_t> sectionRecordStart(header.sectionCount, 0);
        if (!sparse)
        {
            size_t recordAreaSize = static_cast<size_t>(header.recordSize) * header.recordCount;
            combinedData.assign(recordAreaSize + header.stringTableSize, 0);

            size_t recordCursor = 0;
            size_t stringCursor = recordAreaSize;
            for (uint32_t s = 0; s < header.sectionCount; ++s)
            {
                RawSection const& sec = sections[s];
                sectionRecordStart[s] = recordCursor;
                size_t sectionRecordBytes = static_cast<size_t>(header.recordSize) * sec.recordCount;
                if (sec.tactId == 0)
                {
                    if (sec.fileOffset + sectionRecordBytes + sec.stringTableSize > data.size())
                        throw std::runtime_error("a section runs past end of file");
                    std::memcpy(combinedData.data() + recordCursor, data.data() + sec.fileOffset, sectionRecordBytes);
                    std::memcpy(combinedData.data() + stringCursor, data.data() + sec.fileOffset + sectionRecordBytes,
                                sec.stringTableSize);
                }
                recordCursor += sectionRecordBytes;
                stringCursor += sec.stringTableSize;
            }
        }

        for (uint32_t sectionIndex = 0; sectionIndex < header.sectionCount; ++sectionIndex)
        {
            RawSection const& section = sections[sectionIndex];
            bool known = section.tactId == 0;
            if (!known)
            {
                result.warnings.push_back("section " + std::to_string(sectionIndex) + " has TactId 0x" +
                    [&] { std::ostringstream s; s << std::hex << section.tactId; return s.str(); }() + "; skipped (no data)");
                continue;
            }

            if (!sparse)
            {
                size_t recordBytes = static_cast<size_t>(header.recordSize) * section.recordCount;
                // The id list/copy table/parent lookup that may follow are read sequentially via
                // `cursor`, right after this section's own record+string data, in the real file
                // (as opposed to the reassembled `combinedData`, which is decode-only).
                cursor.Seek(section.fileOffset + recordBytes + section.stringTableSize);

                for (uint32_t r = 0; r < section.recordCount; ++r)
                {
                    uint8_t const* record = combinedData.data() + sectionRecordStart[sectionIndex] +
                                             static_cast<size_t>(r) * header.recordSize;

                    Wdc5Row row(totalColumns);
                    for (uint32_t fieldIndex = 0, inlineIndex = 0; fieldIndex < table.fieldCount; ++fieldIndex)
                    {
                        DbcFieldMeta const& field = table.fields[fieldIndex];
                        bool hasSlot = !field.isNonInline || (relationHasSlot && static_cast<int32_t>(fieldIndex) == relationFieldIndex);
                        if (!hasSlot)
                            continue; // non-inline id, or a non-inline relation with no reserved slot; filled in below

                        if (field.isNonInline)
                        {
                            ++inlineIndex; // a reserved-but-unused slot: still consumes the alignment, never decoded here
                            continue;
                        }

                        RegularFieldContext ctx;
                        ctx.field = &field;
                        ctx.columnMeta = columnMeta.empty() ? RawColumnMeta{} : columnMeta[inlineIndex];
                        ctx.palletValues = palletValues.empty() ? nullptr : &palletValues[inlineIndex];
                        ctx.palletArrayValues = palletArrayValues.empty() ? nullptr : &palletArrayValues[inlineIndex];

                        for (uint32_t a = 0; a < field.arrayLength; ++a)
                        {
                            Wdc5Value value = DecodeRegularElement(ctx, combinedData.data(), combinedData.size(), record, a, header.packedDataOffset);
                            uint32_t columnIndex = flattenedStart[fieldIndex] + a;
                            if (ctx.columnMeta.compressionType == ColumnCompression::CommonData)
                                pendingCommonData.push_back({ rowsById.size(), columnIndex, inlineIndex, field.type });
                            row[columnIndex] = value;
                        }

                        ++inlineIndex;
                    }

                    uint32_t id;
                    if (idFieldIndex >= 0 && !table.fields[idFieldIndex].isNonInline)
                        id = static_cast<uint32_t>(row[flattenedStart[idFieldIndex]].intValue);
                    else
                        id = 0; // patched from the id list once it is read, below

                    rowsById.push_back(std::move(row));
                    rowIds.push_back(id);
                }
            }
            else
            {
                // Sparse: variable-length records read on demand via the section's own catalog,
                // fixed at CatalogDataOffset. Fields are laid out back to back in declaration
                // order, each at its metadata-declared width; strings are inline and
                // NUL-terminated.
                cursor.Seek(section.catalogDataOffset);
                std::vector<uint32_t> catalogIds(section.catalogDataCount);
                for (uint32_t& id : catalogIds)
                    id = cursor.Read<uint32_t>();

                std::vector<RawCopy> sectionCopies(section.copyTableCount);
                for (RawCopy& copy : sectionCopies)
                    copy = { cursor.Read<uint32_t>(), cursor.Read<uint32_t>() };
                copyTable.insert(copyTable.end(), sectionCopies.begin(), sectionCopies.end());

                struct CatalogEntry { uint32_t fileOffset; uint16_t recordSize; };
                std::vector<CatalogEntry> catalog(section.catalogDataCount);
                for (CatalogEntry& entry : catalog)
                {
                    entry.fileOffset = cursor.Read<uint32_t>();
                    entry.recordSize = cursor.Read<uint16_t>();
                }

                for (uint32_t r = 0; r < section.catalogDataCount; ++r)
                {
                    if (catalog[r].fileOffset + catalog[r].recordSize > data.size())
                        throw std::runtime_error("sparse record runs past end of file");
                    uint8_t const* record = data.data() + catalog[r].fileOffset;
                    size_t recordSize = catalog[r].recordSize;

                    Wdc5Row row(totalColumns);
                    size_t offset = 0;
                    for (uint32_t fieldIndex = 0, inlineIndex = 0; fieldIndex < table.fieldCount; ++fieldIndex)
                    {
                        DbcFieldMeta const& field = table.fields[fieldIndex];
                        if (field.isNonInline)
                            continue;

                        for (uint32_t a = 0; a < field.arrayLength; ++a)
                        {
                            Wdc5Value value;
                            if (field.type == DbcFieldType::Float)
                            {
                                float f;
                                std::memcpy(&f, record + offset, sizeof(float));
                                value.floatValue = f;
                                offset += sizeof(float);
                            }
                            else if (field.type == DbcFieldType::String || field.type == DbcFieldType::LocString)
                            {
                                value.stringValue = ReadCString(record, recordSize, offset);
                                offset += value.stringValue.size() + 1;
                            }
                            else
                            {
                                // The real per-element width -- from this field's own FieldEntry,
                                // not its nominal metadata width (see EffectiveWidthBytes).
                                int bytes = 4 - fieldUnusedBits[inlineIndex] / 8;
                                uint64_t raw = 0;
                                std::memcpy(&raw, record + offset, static_cast<size_t>(bytes));
                                value.intValue = field.isSigned ? NarrowSigned(raw, bytes * 8) : static_cast<int64_t>(raw);
                                offset += static_cast<size_t>(bytes);
                            }
                            row[flattenedStart[fieldIndex] + a] = value;
                        }

                        ++inlineIndex;
                    }

                    uint32_t id;
                    if (idFieldIndex >= 0 && !table.fields[idFieldIndex].isNonInline)
                        id = static_cast<uint32_t>(row[flattenedStart[idFieldIndex]].intValue);
                    else
                        id = catalogIds[r];

                    rowsById.push_back(std::move(row));
                    rowIds.push_back(id);
                }
            }

            if (idFieldIndex >= 0 && table.fields[idFieldIndex].isNonInline)
            {
                size_t oldSize = idTable.size();
                idTable.resize(oldSize + section.idTableSize / 4);
                for (size_t i = oldSize; i < idTable.size(); ++i)
                {
                    idTable[i] = cursor.Read<uint32_t>();
                    if (idTable[i] <= header.minId)
                        idTable[i] = header.minId + static_cast<uint32_t>(i);
                }
            }

            if (!sparse && section.copyTableCount)
            {
                size_t oldSize = copyTable.size();
                copyTable.resize(oldSize + section.copyTableCount);
                for (size_t i = oldSize; i < copyTable.size(); ++i)
                    copyTable[i] = { cursor.Read<uint32_t>(), cursor.Read<uint32_t>() };
            }

            if (header.parentLookupCount)
            {
                for (uint32_t j = 0; j < header.parentLookupCount; ++j)
                {
                    uint32_t numEntries = cursor.Read<uint32_t>();
                    cursor.Skip(8); // MinId, MaxId
                    if (!numEntries)
                        continue;
                    parentLookupBySection[sectionIndex].resize(numEntries);
                    for (RawIndexEntry& entry : parentLookupBySection[sectionIndex])
                        entry = { cursor.Read<uint32_t>(), cursor.Read<uint32_t>() };
                }
            }
        }

        // Patch in the non-inline id, now that the concatenated id list (if any) is complete.
        if (idFieldIndex >= 0 && table.fields[idFieldIndex].isNonInline)
        {
            for (size_t i = 0; i < rowsById.size() && i < idTable.size(); ++i)
                rowIds[i] = idTable[i];
        }

        // Resolve deferred CommonData lookups now that every row's id is final.
        for (PendingCommonData const& pending : pendingCommonData)
        {
            uint32_t id = rowIds[pending.rowIndex];
            auto it = commonValues[pending.inlineFieldIndex].find(id);
            if (it != commonValues[pending.inlineFieldIndex].end())
                SetDecodedValue(rowsById[pending.rowIndex][pending.columnIndex], pending.fieldType, it->second);
        }

        // Patch in the non-inline relation from the parent lookup, per section, using the
        // section-local record index recorded alongside each entry.
        if (relationFieldIndex >= 0 && table.fields[relationFieldIndex].isNonInline)
        {
            uint32_t globalOffset = 0;
            for (uint32_t sectionIndex = 0; sectionIndex < header.sectionCount; ++sectionIndex)
            {
                for (RawIndexEntry const& entry : parentLookupBySection[sectionIndex])
                {
                    uint32_t globalIndex = globalOffset + entry.recordIndex;
                    if (globalIndex < rowsById.size())
                        rowsById[globalIndex][flattenedStart[relationFieldIndex]].intValue = entry.parentId;
                }
                globalOffset += sections[sectionIndex].tactId == 0
                    ? (sparse ? sections[sectionIndex].catalogDataCount : sections[sectionIndex].recordCount)
                    : 0;
            }
        }

        for (size_t i = 0; i < rowsById.size(); ++i)
        {
            if (idFieldIndex >= 0 && table.fields[idFieldIndex].isNonInline)
                rowsById[i][flattenedStart[idFieldIndex]].intValue = rowIds[i];
            result.rows.push_back(std::move(rowsById[i]));
        }

        // Copy table: duplicate a source row under a new id.
        for (RawCopy const& copy : copyTable)
        {
            auto it = std::find(rowIds.begin(), rowIds.end(), copy.sourceRowId);
            if (it == rowIds.end())
                continue;
            size_t sourceIndex = static_cast<size_t>(it - rowIds.begin());
            Wdc5Row copied = result.rows[sourceIndex];
            if (idFieldIndex >= 0)
                copied[flattenedStart[idFieldIndex]].intValue = static_cast<int64_t>(copy.newRowId);
            result.rows.push_back(std::move(copied));
        }
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
        result.rows.clear();
    }

    return result;
}
