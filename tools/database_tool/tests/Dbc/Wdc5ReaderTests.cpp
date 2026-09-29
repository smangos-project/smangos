#include "Dbc/Wdc5Reader.h"

#include <catch_amalgamated.hpp>

#include <cstring>
#include <filesystem>
#include <fstream>

namespace
{
    struct TempFile
    {
        std::filesystem::path path;

        explicit TempFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_wdc5_" + name + ".db2"))
        {
        }

        ~TempFile() { std::filesystem::remove(path); }

        void Write(std::vector<uint8_t> const& bytes)
        {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            out.write(reinterpret_cast<char const*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        }
    };

    void PutU16(std::vector<uint8_t>& buf, uint16_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 2); }
    void PutI16(std::vector<uint8_t>& buf, int16_t v) { PutU16(buf, static_cast<uint16_t>(v)); }
    void PutU32(std::vector<uint8_t>& buf, uint32_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 4); }
    void PutU64(std::vector<uint8_t>& buf, uint64_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 8); }
    void PutF32(std::vector<uint8_t>& buf, float v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 4); }
    void PutPad(std::vector<uint8_t>& buf, size_t n) { buf.insert(buf.end(), n, 0); }
    void PutCString(std::vector<uint8_t>& buf, std::string const& s) { buf.insert(buf.end(), s.begin(), s.end()); buf.push_back(0); }

    void PatchU32(std::vector<uint8_t>& buf, size_t pos, uint32_t v)
    {
        std::memcpy(buf.data() + pos, &v, 4);
    }

    // Overwrites (rather than appends) 4 bytes at `pos` in an already-sized buffer.
    void PutU32At(std::vector<uint8_t>& buf, size_t pos, uint32_t v) { PatchU32(buf, pos, v); }

    // ORs `value`'s low `bitWidth` bits into `record` starting at byte `byteOffset`, bit
    // `bitOffsetInByte` -- the write-side mirror of Wdc5Reader's ReadPackedBits.
    void OrPackedBits(std::vector<uint8_t>& record, size_t byteOffset, uint32_t bitOffsetInByte, uint32_t bitWidth, uint64_t value)
    {
        uint64_t mask = bitWidth >= 64 ? ~uint64_t(0) : ((uint64_t(1) << bitWidth) - 1);
        uint64_t bits = (value & mask) << bitOffsetInByte;
        for (size_t i = 0; i < 8 && byteOffset + i < record.size(); ++i)
            record[byteOffset + i] |= static_cast<uint8_t>((bits >> (i * 8)) & 0xFF);
    }

    struct HeaderSpec
    {
        uint32_t recordCount = 0;
        uint32_t fieldCount = 0;
        uint32_t recordSize = 0;
        uint32_t stringTableSize = 0;
        uint32_t tableHash = 0x1234;
        uint32_t layoutHash = 0xABCD;
        uint32_t minId = 0;
        uint32_t maxId = 1000;
        uint32_t locale = 1;
        uint16_t flags = 0;
        int16_t indexField = 0;
        uint32_t totalFieldCount = 0;
        uint32_t packedDataOffset = 0;
        uint32_t parentLookupCount = 0;
        uint32_t columnMetaSize = 0;
        uint32_t commonDataSize = 0;
        uint32_t palletDataSize = 0;
        uint32_t sectionCount = 1;
    };

    void PutHeader(std::vector<uint8_t>& buf, HeaderSpec const& h)
    {
        PutU32(buf, 0x35434457u); // 'WDC5'
        PutU32(buf, 5);
        std::string schema = "TestSchema";
        buf.insert(buf.end(), schema.begin(), schema.end());
        PutPad(buf, 128 - schema.size());
        PutU32(buf, h.recordCount);
        PutU32(buf, h.fieldCount);
        PutU32(buf, h.recordSize);
        PutU32(buf, h.stringTableSize);
        PutU32(buf, h.tableHash);
        PutU32(buf, h.layoutHash);
        PutU32(buf, h.minId);
        PutU32(buf, h.maxId);
        PutU32(buf, h.locale);
        PutU16(buf, h.flags);
        PutI16(buf, h.indexField);
        PutU32(buf, h.totalFieldCount);
        PutU32(buf, h.packedDataOffset);
        PutU32(buf, h.parentLookupCount);
        PutU32(buf, h.columnMetaSize);
        PutU32(buf, h.commonDataSize);
        PutU32(buf, h.palletDataSize);
        PutU32(buf, h.sectionCount);
    }

    void PutSection(std::vector<uint8_t>& buf, uint64_t tactId, uint32_t fileOffset, uint32_t recordCount,
                     uint32_t stringTableSize, uint32_t catalogDataOffset, uint32_t idTableSize,
                     uint32_t parentLookupDataSize, uint32_t catalogDataCount, uint32_t copyTableCount)
    {
        PutU64(buf, tactId);
        PutU32(buf, fileOffset);
        PutU32(buf, recordCount);
        PutU32(buf, stringTableSize);
        PutU32(buf, catalogDataOffset);
        PutU32(buf, idTableSize);
        PutU32(buf, parentLookupDataSize);
        PutU32(buf, catalogDataCount);
        PutU32(buf, copyTableCount);
    }

    enum class Compression : uint32_t { None, Immediate, CommonData, Pallet, PalletArray, SignedImmediate };

    void PutColumnMeta(std::vector<uint8_t>& buf, uint16_t bitOffset, uint16_t bitSize, uint32_t additionalDataSize,
                        Compression compression, uint32_t packed0, uint32_t packed1, uint32_t packed2)
    {
        PutU16(buf, bitOffset);
        PutU16(buf, bitSize);
        PutU32(buf, additionalDataSize);
        PutU32(buf, static_cast<uint32_t>(compression));
        PutU32(buf, packed0);
        PutU32(buf, packed1);
        PutU32(buf, packed2);
    }

    DbcFieldMeta MakeField(char const* name, DbcFieldType type, uint8_t bits, bool isSigned, uint16_t arrayLength,
                            bool isId = false, bool isRelation = false, bool isNonInline = false,
                            char const* foreignTable = nullptr, char const* foreignColumn = nullptr)
    {
        return { name, type, bits, isSigned, arrayLength, isId, isRelation, isNonInline, foreignTable, foreignColumn };
    }

    Wdc5ReadResult ReadFromBytes(TempFile& file, std::vector<uint8_t> const& bytes, DbcTableMeta const& table)
    {
        file.Write(bytes);
        return ReadWdc5File(file.path, table);
    }
}

TEST_CASE("ReadWdc5File decodes None-compressed scalar and array fields", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Delta", DbcFieldType::Int, 16, true, 1),
        MakeField("Weight", DbcFieldType::Float, 0, true, 1),
        MakeField("Name", DbcFieldType::String, 0, true, 1),
        MakeField("Arr", DbcFieldType::Int, 16, false, 2),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 5 };

    // None region layout: ID(4)@0, Delta(2)@4, Weight(4)@6, Name(4)@10, Arr(2*2)@14 -> record size 18
    uint32_t recordSize = 18;
    HeaderSpec h;
    h.recordCount = 1;
    h.fieldCount = 5;
    h.totalFieldCount = 5;
    h.recordSize = recordSize;
    h.layoutHash = 0xABCD;
    h.columnMetaSize = 5 * 24;

    std::vector<uint8_t> record;
    PutU32(record, 7);              // ID
    PutI16(record, -5);             // Delta
    PutF32(record, 3.5f);           // Weight
    PutU32(record, 8);              // Name relative offset: fieldOffset(10) + 8 = string at record+18 (start of string table)
    PutU16(record, 111);            // Arr[0]
    PutU16(record, 222);            // Arr[1]

    std::string stringTable = "Hello";
    stringTable += '\0';
    h.stringTableSize = static_cast<uint32_t>(stringTable.size());

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t sectionPos = file.size();
    PutSection(file, 0, 0, h.recordCount, h.stringTableSize, 0, 0, 0, 0, 0);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4); // field entries, unused
    PutColumnMeta(file, 0, 32, 0, Compression::None, 0, 0, 0);
    PutColumnMeta(file, 32, 16, 0, Compression::None, 0, 0, 0);
    PutColumnMeta(file, 48, 32, 0, Compression::None, 0, 0, 0);
    PutColumnMeta(file, 80, 32, 0, Compression::None, 0, 0, 0);
    PutColumnMeta(file, 112, 16, 0, Compression::None, 0, 0, 0);

    uint32_t fileOffset = static_cast<uint32_t>(file.size());
    PatchU32(file, sectionPos + 8, fileOffset); // section.fileOffset (after TactId u64)
    file.insert(file.end(), record.begin(), record.end());
    file.insert(file.end(), stringTable.begin(), stringTable.end());

    TempFile temp("none");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    CHECK(result.rows[0][0].intValue == 7);
    CHECK(result.rows[0][1].intValue == -5);
    CHECK(result.rows[0][2].floatValue == Catch::Approx(3.5));
    CHECK(result.rows[0][3].stringValue == "Hello");
    CHECK(result.rows[0][4].intValue == 111);
    CHECK(result.rows[0][5].intValue == 222);
    CHECK(result.info.tableHash == h.tableHash);
    CHECK(result.info.layoutHash == 0xABCD);
    CHECK(result.info.schema == "TestSchema");
}

TEST_CASE("ReadWdc5File decodes Immediate, SignedImmediate, Pallet, PalletArray and CommonData", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Imm", DbcFieldType::Int, 32, false, 1),
        MakeField("SImm", DbcFieldType::Int, 32, true, 1),
        MakeField("Pal", DbcFieldType::Int, 32, false, 1),
        MakeField("PalArr", DbcFieldType::Int, 32, false, 2),
        MakeField("Common", DbcFieldType::Int, 32, false, 1),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 6 };

    uint32_t noneSize = 4; // just ID
    uint32_t packedDataOffset = noneSize;
    uint32_t packedBytes = 4; // 4 bits (Imm) + 4 bits (SImm) + 4 bits (Pal) + 4 bits (PalArr) = 16 bits -> 2 bytes; round up for safety
    uint32_t recordSize = packedDataOffset + packedBytes;

    HeaderSpec h;
    h.recordCount = 2;
    h.fieldCount = 6;
    h.totalFieldCount = 6;
    h.recordSize = recordSize;
    h.layoutHash = 0xABCD;
    h.packedDataOffset = packedDataOffset;
    h.columnMetaSize = 6 * 24;
    h.commonDataSize = 8; // one override entry
    h.palletDataSize = 3 * 4 + 4 * 4; // 3 pallet values + 4 pallet-array values

    std::vector<uint8_t> recordA(recordSize, 0), recordB(recordSize, 0);
    PutU32At(recordA, 0, 10); // ID
    PutU32At(recordB, 0, 20);

    // Imm: 4 bits at bit 0, Pal/etc share the same byte range packed tightly.
    OrPackedBits(recordA, packedDataOffset, 0, 4, 9);   // Imm = 9
    OrPackedBits(recordA, packedDataOffset, 4, 4, static_cast<uint64_t>(-3) & 0xF); // SImm = -3 (4-bit two's complement)
    OrPackedBits(recordA, packedDataOffset + 1, 0, 4, 1); // Pal index 1
    OrPackedBits(recordA, packedDataOffset + 1, 4, 4, 0); // PalArr index 0

    OrPackedBits(recordB, packedDataOffset, 0, 4, 3);   // Imm = 3
    OrPackedBits(recordB, packedDataOffset, 4, 4, 2);   // SImm = 2
    OrPackedBits(recordB, packedDataOffset + 1, 0, 4, 2); // Pal index 2
    OrPackedBits(recordB, packedDataOffset + 1, 4, 4, 1); // PalArr index 1

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t sectionPos = file.size();
    PutSection(file, 0, 0, h.recordCount, 0, 0, 0, 0, 0, 0);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);

    PutColumnMeta(file, 0, 32, 0, Compression::None, 0, 0, 0);                 // ID
    PutColumnMeta(file, 0, 4, 0, Compression::Immediate, 0, 4, 0);             // Imm: bit 0, width 4
    PutColumnMeta(file, 0, 4, 0, Compression::SignedImmediate, 4, 4, 0);       // SImm: bit 4, width 4
    PutColumnMeta(file, 0, 4, 3 * 4, Compression::Pallet, 8, 4, 0);            // Pal: bit 8, width 4
    PutColumnMeta(file, 0, 4, 4 * 4, Compression::PalletArray, 12, 4, 2);      // PalArr: bit 12, width 4, ArraySize 2
    PutColumnMeta(file, 0, 0, 8, Compression::CommonData, 42, 0, 0);           // Common: default 42

    for (uint32_t v : { 111u, 222u, 333u })
        PutU32(file, v); // pallet values
    for (uint32_t v : { 10u, 11u, 20u, 21u })
        PutU32(file, v); // pallet array values (index*2 + arrayIndex)
    PutU32(file, 10); PutU32(file, 999); // CommonData override: recordId=10 -> 999

    uint32_t fileOffset = static_cast<uint32_t>(file.size());
    PatchU32(file, sectionPos + 8, fileOffset);
    file.insert(file.end(), recordA.begin(), recordA.end());
    file.insert(file.end(), recordB.begin(), recordB.end());

    TempFile temp("compression");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 2);

    CHECK(result.rows[0][0].intValue == 10);
    CHECK(result.rows[0][1].intValue == 9);
    CHECK(result.rows[0][2].intValue == -3);
    CHECK(result.rows[0][3].intValue == 222);
    CHECK(result.rows[0][4].intValue == 10);
    CHECK(result.rows[0][5].intValue == 11);
    CHECK(result.rows[0][6].intValue == 999); // override for id 10

    CHECK(result.rows[1][0].intValue == 20);
    CHECK(result.rows[1][1].intValue == 3);
    CHECK(result.rows[1][2].intValue == 2);
    CHECK(result.rows[1][3].intValue == 333);
    CHECK(result.rows[1][4].intValue == 20);
    CHECK(result.rows[1][5].intValue == 21);
    CHECK(result.rows[1][6].intValue == 42); // default, no override for id 20
}

TEST_CASE("ReadWdc5File narrows pallet and common values to the field width", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Byte", DbcFieldType::Int, 8, true, 1),
        MakeField("Short", DbcFieldType::Int, 16, false, 1),
        MakeField("Common", DbcFieldType::Int, 8, false, 1),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 4 };

    uint32_t packedDataOffset = 4;
    uint32_t recordSize = packedDataOffset + 4;

    HeaderSpec h;
    h.recordCount = 1;
    h.fieldCount = 4;
    h.totalFieldCount = 4;
    h.recordSize = recordSize;
    h.packedDataOffset = packedDataOffset;
    h.columnMetaSize = 4 * 24;
    h.palletDataSize = 2 * 4;

    std::vector<uint8_t> record(recordSize, 0);
    PutU32At(record, 0, 10);
    OrPackedBits(record, packedDataOffset, 0, 1, 0); // Byte pallet index 0
    OrPackedBits(record, packedDataOffset, 1, 1, 0); // Short pallet index 0

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t sectionPos = file.size();
    PutSection(file, 0, 0, h.recordCount, 0, 0, 0, 0, 0, 0);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);

    PutColumnMeta(file, 0, 32, 0, Compression::None, 0, 0, 0);
    PutColumnMeta(file, 0, 1, 4, Compression::Pallet, 0, 1, 0);
    PutColumnMeta(file, 0, 1, 4, Compression::Pallet, 1, 1, 0);
    PutColumnMeta(file, 0, 0, 0, Compression::CommonData, 0x1205, 0, 0);

    PutU32(file, 0x12A0FFu); // Byte: -1 with neighbouring bytes above it
    PutU32(file, 0x42200005u); // Short: 5

    uint32_t fileOffset = static_cast<uint32_t>(file.size());
    PatchU32(file, sectionPos + 8, fileOffset);
    file.insert(file.end(), record.begin(), record.end());

    TempFile temp("narrow");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    CHECK(result.rows[0][1].intValue == -1);
    CHECK(result.rows[0][2].intValue == 5);
    CHECK(result.rows[0][3].intValue == 5);
}

TEST_CASE("ReadWdc5File resolves a non-inline id list and the copy table", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true, false, /*isNonInline*/ true),
        MakeField("Value", DbcFieldType::Int, 32, false, 1),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 2 };

    uint32_t recordSize = 4; // just Value inline
    HeaderSpec h;
    h.recordCount = 1;
    h.fieldCount = 1; // ID is non-inline: not part of the file's inline field count
    h.totalFieldCount = 1;
    h.recordSize = recordSize;
    h.layoutHash = 0xABCD;
    h.columnMetaSize = 1 * 24;

    std::vector<uint8_t> record;
    PutU32(record, 555); // Value

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t sectionPos = file.size();
    PutSection(file, 0, 0, h.recordCount, 0, 0, /*idTableSize*/ 4, 0, 0, /*copyTableCount*/ 1);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);
    PutColumnMeta(file, 0, 32, 0, Compression::None, 0, 0, 0);

    uint32_t fileOffset = static_cast<uint32_t>(file.size());
    PatchU32(file, sectionPos + 8, fileOffset);
    file.insert(file.end(), record.begin(), record.end());
    PutU32(file, 42); // id list: row 0 has id 42
    PutU32(file, 43); PutU32(file, 42); // copy table: new id 43 copies source id 42

    TempFile temp("idlist_copy");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0][0].intValue == 42);
    CHECK(result.rows[0][1].intValue == 555);
    CHECK(result.rows[1][0].intValue == 43);
    CHECK(result.rows[1][1].intValue == 555);
}

TEST_CASE("ReadWdc5File resolves a non-inline relation from the parent lookup", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Value", DbcFieldType::Int, 32, false, 1),
        MakeField("ParentID", DbcFieldType::Int, 32, false, 1, false, /*isRelation*/ true, /*isNonInline*/ true),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 3 };

    uint32_t recordSize = 8; // ID + Value inline
    HeaderSpec h;
    h.recordCount = 2;
    h.fieldCount = 2; // ParentID is non-inline
    h.totalFieldCount = 2;
    h.recordSize = recordSize;
    h.layoutHash = 0xABCD;
    h.parentLookupCount = 1;
    h.columnMetaSize = 2 * 24;

    std::vector<uint8_t> recordA, recordB;
    PutU32(recordA, 1); PutU32(recordA, 100);
    PutU32(recordB, 2); PutU32(recordB, 200);

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t sectionPos = file.size();
    PutSection(file, 0, 0, h.recordCount, 0, 0, 0, 0, 0, 0);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);
    PutColumnMeta(file, 0, 32, 0, Compression::None, 0, 0, 0);
    PutColumnMeta(file, 32, 32, 0, Compression::None, 0, 0, 0);

    uint32_t fileOffset = static_cast<uint32_t>(file.size());
    PatchU32(file, sectionPos + 8, fileOffset);
    file.insert(file.end(), recordA.begin(), recordA.end());
    file.insert(file.end(), recordB.begin(), recordB.end());

    // Parent lookup: one entry, only record index 1 (recordB) has a parent.
    PutU32(file, 1); PutU32(file, 0); PutU32(file, 1); // NumEntries, MinId, MaxId
    PutU32(file, 999); PutU32(file, 1); // ParentId=999, RecordIndex=1

    TempFile temp("parent_lookup");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0][2].intValue == 0);   // no parent lookup entry for record 0
    CHECK(result.rows[1][2].intValue == 999); // patched from the parent lookup
}

TEST_CASE("ReadWdc5File decodes a sparse (variable-length) file", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Value", DbcFieldType::Int, 16, false, 1),
        MakeField("Name", DbcFieldType::String, 0, true, 1),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 3 };

    HeaderSpec h;
    h.recordCount = 2;
    h.fieldCount = 3;
    h.totalFieldCount = 3;
    h.recordSize = 0; // unused for sparse
    h.layoutHash = 0xABCD;
    h.flags = 0x1; // sparse
    h.maxId = 99;

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t sectionPos = file.size();
    // catalogDataOffset and fileOffset are patched in below once known.
    PutSection(file, 0, 0, h.recordCount, 0, 0, 0, 0, /*catalogDataCount*/ 2, 0);
    // Field entries: UnusedBits = 32 - the field's real width, the one FieldEntry member sparse
    // decode actually consults (Offset is unused by every known reader, including this one).
    PutI16(file, 0);  PutU16(file, 0); // ID: 32 bits
    PutI16(file, 16); PutU16(file, 0); // Value: 16 bits
    PutI16(file, 0);  PutU16(file, 0); // Name: irrelevant for strings

    // No columnMetaSize -> sparse never uses columnMeta.

    // Sparse records, variable length: ID(4)+Value(2)+Name(NUL-terminated).
    std::vector<uint8_t> recordA, recordB;
    PutU32(recordA, 7); PutU16(recordA, 11); PutCString(recordA, "Ax");
    PutU32(recordB, 8); PutU16(recordB, 22); PutCString(recordB, "Bravo");

    uint32_t recordsStart = static_cast<uint32_t>(file.size());
    file.insert(file.end(), recordA.begin(), recordA.end());
    uint32_t recordBOffset = static_cast<uint32_t>(file.size());
    file.insert(file.end(), recordB.begin(), recordB.end());

    uint32_t catalogDataOffset = static_cast<uint32_t>(file.size());
    PutU32(file, 70); PutU32(file, 80); // catalog ids
    // no copy table
    PutU32(file, recordsStart); PutU16(file, static_cast<uint16_t>(recordA.size()));
    PutU32(file, recordBOffset); PutU16(file, static_cast<uint16_t>(recordB.size()));

    PatchU32(file, sectionPos + 8, recordsStart);       // section.fileOffset (unused by the reader for sparse, but set anyway)
    PatchU32(file, sectionPos + 8 + 12, catalogDataOffset); // section.catalogDataOffset

    TempFile temp("sparse");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0][0].intValue == 7);
    CHECK(result.rows[0][1].intValue == 11);
    CHECK(result.rows[0][2].stringValue == "Ax");
    CHECK(result.rows[1][0].intValue == 8);
    CHECK(result.rows[1][1].intValue == 22);
    CHECK(result.rows[1][2].stringValue == "Bravo");
}

TEST_CASE("ReadWdc5File skips an encrypted section and warns", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = { MakeField("ID", DbcFieldType::Int, 32, false, 1, true) };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 1 };

    HeaderSpec h;
    h.recordCount = 2; // 1 encrypted + 1 normal
    h.fieldCount = 1;
    h.totalFieldCount = 1;
    h.recordSize = 4;
    h.layoutHash = 0xABCD;
    h.columnMetaSize = 24;
    h.sectionCount = 2;

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t section0Pos = file.size();
    PutSection(file, 0xDEAD, 0, 1, 0, 0, 0, 0, 0, 0); // encrypted section, 1 record
    size_t section1Pos = file.size();
    PutSection(file, 0, 0, 1, 0, 0, 0, 0, 0, 0); // normal section, 1 record
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);
    PutColumnMeta(file, 0, 32, 0, Compression::None, 0, 0, 0);

    // Encrypted-id-count block for section 0 only (nonzero TactId).
    PutU32(file, 1); PutU32(file, 12345);

    uint32_t normalOffset = static_cast<uint32_t>(file.size());
    PutU32(file, 77); // the one decodable record

    PatchU32(file, section1Pos + 8, normalOffset);

    TempFile temp("encrypted");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    CHECK(result.rows[0][0].intValue == 77);
    REQUIRE(result.warnings.size() == 1);
    CHECK(result.warnings[0].find("skipped") != std::string::npos);
}

TEST_CASE("ReadWdc5File imports an empty table with zero sections", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = { MakeField("ID", DbcFieldType::Int, 32, false, 1, true) };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 1 };

    HeaderSpec h;
    h.recordCount = 0;
    h.fieldCount = 1;
    h.totalFieldCount = 1;
    h.recordSize = 4;
    h.layoutHash = 0xABCD;
    h.sectionCount = 0;

    std::vector<uint8_t> file;
    PutHeader(file, h);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);

    TempFile temp("zero_section");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    CHECK(result.rows.empty());
}

TEST_CASE("ReadWdc5File rejects an unknown layout hash", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = { MakeField("ID", DbcFieldType::Int, 32, false, 1, true) };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 1 };

    HeaderSpec h;
    h.fieldCount = 1;
    h.totalFieldCount = 1;
    h.layoutHash = 0x11111111; // not in table.layoutHashes
    h.sectionCount = 0;

    std::vector<uint8_t> file;
    PutHeader(file, h);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);

    TempFile temp("bad_layout");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    CHECK(result.error.find("layout") != std::string::npos);
    CHECK(result.rows.empty());
}

TEST_CASE("ReadWdc5File rejects a field count mismatch", "[Wdc5Reader]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, true),
        MakeField("Value", DbcFieldType::Int, 32, false, 1),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 2 };

    HeaderSpec h;
    h.fieldCount = 1; // metadata expects 2 inline fields
    h.totalFieldCount = 1;
    h.layoutHash = 0xABCD;
    h.sectionCount = 0;

    std::vector<uint8_t> file;
    PutHeader(file, h);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);

    TempFile temp("bad_fieldcount");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    CHECK(result.error.find("field count") != std::string::npos);
}

TEST_CASE("ReadWdc5File clamps a None column's declared width to 8 bytes", "[Wdc5Reader]")
{
    // A stray/oversized columnMeta.bitSize (here 512 bits = 64 bytes, larger than any real
    // integer column) must not blow up the elementBytes used both for the memcpy width and for
    // striding across array elements.
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Oversized", DbcFieldType::Int, 64, false, 2),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 2 };

    uint32_t recordSize = 4 + 8 + 8; // ID + two clamped-to-8-byte elements
    HeaderSpec h;
    h.recordCount = 1;
    h.fieldCount = 2;
    h.totalFieldCount = 2;
    h.recordSize = recordSize;
    h.layoutHash = 0xABCD;
    h.columnMetaSize = 2 * 24;

    std::vector<uint8_t> record;
    PutU32(record, 99);                    // ID
    PutU64(record, 0x0102030405060708ULL); // Oversized[0]
    PutU64(record, 0x1112131415161718ULL); // Oversized[1]

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t sectionPos = file.size();
    PutSection(file, 0, 0, h.recordCount, 0, 0, 0, 0, 0, 0);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);
    PutColumnMeta(file, 0, 32, 0, Compression::None, 0, 0, 0);
    PutColumnMeta(file, 32, 512, 0, Compression::None, 0, 0, 0); // bitSize way past any real width

    uint32_t fileOffset = static_cast<uint32_t>(file.size());
    PatchU32(file, sectionPos + 8, fileOffset);
    file.insert(file.end(), record.begin(), record.end());

    TempFile temp("oversized_width");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    CHECK(result.rows[0][0].intValue == 99);
    CHECK(result.rows[0][1].intValue == static_cast<int64_t>(0x0102030405060708ULL));
    CHECK(result.rows[0][2].intValue == static_cast<int64_t>(0x1112131415161718ULL));
}

TEST_CASE("ReadWdc5File bounds-checks a None field against a truncated buffer", "[Wdc5Reader]")
{
    // Seen on a real file (OverrideSpellData.db2): the last column of the last record in a
    // small section can have fewer bytes left in the buffer than even a clamped elementBytes
    // calls for. The reader must read only what's actually there instead of running past it.
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Trailing", DbcFieldType::Int, 64, false, 1),
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 2 };

    // Declares an 8-byte-wide Trailing column but the file only actually has 1 byte for it.
    uint32_t recordSize = 5;
    HeaderSpec h;
    h.recordCount = 1;
    h.fieldCount = 2;
    h.totalFieldCount = 2;
    h.recordSize = recordSize;
    h.layoutHash = 0xABCD;
    h.columnMetaSize = 2 * 24;

    std::vector<uint8_t> record;
    PutU32(record, 55);            // ID
    record.push_back(0xEE);        // Trailing: only 1 byte actually present

    std::vector<uint8_t> file;
    PutHeader(file, h);
    size_t sectionPos = file.size();
    PutSection(file, 0, 0, h.recordCount, 0, 0, 0, 0, 0, 0);
    PutPad(file, static_cast<size_t>(h.fieldCount) * 4);
    PutColumnMeta(file, 0, 32, 0, Compression::None, 0, 0, 0);
    PutColumnMeta(file, 32, 64, 0, Compression::None, 0, 0, 0);

    uint32_t fileOffset = static_cast<uint32_t>(file.size());
    PatchU32(file, sectionPos + 8, fileOffset);
    file.insert(file.end(), record.begin(), record.end());

    TempFile temp("truncated_buffer");
    Wdc5ReadResult result = ReadFromBytes(temp, file, table);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    CHECK(result.rows[0][0].intValue == 55);
    CHECK(result.rows[0][1].intValue == 0xEE);
}
