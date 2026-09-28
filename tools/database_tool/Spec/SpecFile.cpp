#include "SpecFile.h"

#include <fstream>
#include <stdexcept>

namespace
{
    std::vector<std::string> SplitTabs(std::string const& line)
    {
        std::vector<std::string> fields;
        std::string current;
        for (char c : line)
        {
            if (c == '\t')
            {
                fields.push_back(current);
                current.clear();
            }
            else
            {
                current += c;
            }
        }
        fields.push_back(current);
        return fields;
    }

    // A tab or newline in a hand-written field would corrupt the column count on the next
    // read, so collapse either to a space rather than writing it out raw.
    std::string SanitizeField(std::string field)
    {
        for (char& c : field)
            if (c == '\t' || c == '\n' || c == '\r')
                c = ' ';
        return field;
    }

    std::vector<std::vector<std::string>> ReadDataRows(std::filesystem::path const& path, std::vector<std::string> const& header, size_t columnCount)
    {
        std::vector<std::vector<std::string>> rows;
        if (!std::filesystem::exists(path))
            return rows;

        std::ifstream in(path);
        std::string line;
        if (!std::getline(in, line))
            return rows;
        if (SplitTabs(line) != header)
            throw std::runtime_error(path.string() + ": unexpected header");

        while (std::getline(in, line))
        {
            if (line.empty())
                continue;
            std::vector<std::string> fields = SplitTabs(line);
            if (fields.size() != columnCount)
                throw std::runtime_error(path.string() + ": row has " + std::to_string(fields.size()) +
                                          " fields, expected " + std::to_string(columnCount));
            rows.push_back(std::move(fields));
        }
        return rows;
    }

    void WriteRows(std::filesystem::path const& path, std::vector<std::string> const& header, std::vector<std::vector<std::string>> const& rows)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path, std::ios::trunc);

        auto writeLine = [&](std::vector<std::string> const& fields)
        {
            for (size_t i = 0; i < fields.size(); ++i)
                out << (i ? "\t" : "") << fields[i];
            out << "\n";
        };

        writeLine(header);
        for (std::vector<std::string> const& row : rows)
            writeLine(row);
    }
}

std::vector<SpecColumnRow> ReadColumnsTsv(std::filesystem::path const& path)
{
    static std::vector<std::string> const header = { "source", "table", "column", "disposition", "target", "rule", "reason", "reviewed" };
    std::vector<SpecColumnRow> rows;
    for (std::vector<std::string> const& fields : ReadDataRows(path, header, header.size()))
        rows.push_back({ fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], fields[7] });
    return rows;
}

void WriteColumnsTsv(std::filesystem::path const& path, std::vector<SpecColumnRow> const& rows)
{
    static std::vector<std::string> const header = { "source", "table", "column", "disposition", "target", "rule", "reason", "reviewed" };
    std::vector<std::vector<std::string>> data;
    data.reserve(rows.size());
    for (SpecColumnRow const& row : rows)
        data.push_back({ row.source, row.table, row.column, SanitizeField(row.disposition), SanitizeField(row.target),
                          SanitizeField(row.rule), SanitizeField(row.reason), SanitizeField(row.reviewed) });
    WriteRows(path, header, data);
}

std::vector<SpecDbcColumnRow> ReadDbcColumnsTsv(std::filesystem::path const& path)
{
    static std::vector<std::string> const header = { "table", "column", "mangosd_reads", "vanilla_value" };
    std::vector<SpecDbcColumnRow> rows;
    for (std::vector<std::string> const& fields : ReadDataRows(path, header, header.size()))
        rows.push_back({ fields[0], fields[1], fields[2], fields[3] });
    return rows;
}

void WriteDbcColumnsTsv(std::filesystem::path const& path, std::vector<SpecDbcColumnRow> const& rows)
{
    static std::vector<std::string> const header = { "table", "column", "mangosd_reads", "vanilla_value" };
    std::vector<std::vector<std::string>> data;
    data.reserve(rows.size());
    for (SpecDbcColumnRow const& row : rows)
        data.push_back({ row.table, row.column, SanitizeField(row.mangosdReads), SanitizeField(row.vanillaValue) });
    WriteRows(path, header, data);
}

std::vector<SpecEvidenceRow> ReadEvidenceTsv(std::filesystem::path const& path)
{
    static std::vector<std::string> const header = { "source", "table", "column", "type", "readers", "wire_hint",
        "counterpart", "counterpart_rule", "match_rate", "shape" };
    std::vector<SpecEvidenceRow> rows;
    for (std::vector<std::string> const& fields : ReadDataRows(path, header, header.size()))
        rows.push_back({ fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], fields[7], fields[8], fields[9] });
    return rows;
}

void WriteEvidenceTsv(std::filesystem::path const& path, std::vector<SpecEvidenceRow> const& rows)
{
    static std::vector<std::string> const header = { "source", "table", "column", "type", "readers", "wire_hint",
        "counterpart", "counterpart_rule", "match_rate", "shape" };
    std::vector<std::vector<std::string>> data;
    data.reserve(rows.size());
    for (SpecEvidenceRow const& row : rows)
        data.push_back({ row.source, row.table, row.column, SanitizeField(row.type), SanitizeField(row.readers),
                          SanitizeField(row.wireHint), SanitizeField(row.counterpart), SanitizeField(row.counterpartRule),
                          SanitizeField(row.matchRate), SanitizeField(row.shape) });
    WriteRows(path, header, data);
}

std::vector<SpecPairRow> ReadPairsTsv(std::filesystem::path const& path)
{
    static std::vector<std::string> const header = { "source", "table", "cdbc_table", "join_kind", "source_key", "cdbc_key", "notes" };
    std::vector<SpecPairRow> rows;
    for (std::vector<std::string> const& fields : ReadDataRows(path, header, header.size()))
        rows.push_back({ fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6] });
    return rows;
}

void WritePairsTsv(std::filesystem::path const& path, std::vector<SpecPairRow> const& rows)
{
    static std::vector<std::string> const header = { "source", "table", "cdbc_table", "join_kind", "source_key", "cdbc_key", "notes" };
    std::vector<std::vector<std::string>> data;
    data.reserve(rows.size());
    for (SpecPairRow const& row : rows)
        data.push_back({ row.source, row.table, row.cdbcTable, row.joinKind, row.sourceKey, row.cdbcKey, SanitizeField(row.notes) });
    WriteRows(path, header, data);
}

std::vector<SpecGapRow> ReadGapsTsv(std::filesystem::path const& path)
{
    static std::vector<std::string> const header = { "kind", "table", "column", "detail" };
    std::vector<SpecGapRow> rows;
    for (std::vector<std::string> const& fields : ReadDataRows(path, header, header.size()))
        rows.push_back({ fields[0], fields[1], fields[2], fields[3] });
    return rows;
}

void WriteGapsTsv(std::filesystem::path const& path, std::vector<SpecGapRow> const& rows)
{
    static std::vector<std::string> const header = { "kind", "table", "column", "detail" };
    std::vector<std::vector<std::string>> data;
    data.reserve(rows.size());
    for (SpecGapRow const& row : rows)
        data.push_back({ row.kind, row.table, row.column, SanitizeField(row.detail) });
    WriteRows(path, header, data);
}

std::vector<SpecRuleRow> ReadRulesTsv(std::filesystem::path const& path)
{
    static std::vector<std::string> const header = { "name", "kind", "expr", "inverse", "applies_to" };
    std::vector<SpecRuleRow> rows;
    for (std::vector<std::string> const& fields : ReadDataRows(path, header, header.size()))
        rows.push_back({ fields[0], fields[1], fields[2], fields[3], fields[4] });
    return rows;
}

void WriteRulesTsv(std::filesystem::path const& path, std::vector<SpecRuleRow> const& rows)
{
    static std::vector<std::string> const header = { "name", "kind", "expr", "inverse", "applies_to" };
    std::vector<std::vector<std::string>> data;
    data.reserve(rows.size());
    for (SpecRuleRow const& row : rows)
        data.push_back({ row.name, row.kind, row.expr, row.inverse, SanitizeField(row.appliesTo) });
    WriteRows(path, header, data);
}
