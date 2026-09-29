#include "SpecRender.h"

#include "Spec/SpecFile.h"

#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <tuple>

static std::string Cell(std::string const& value)
{
    std::string escaped;
    for (char c : value)
    {
        if (c == '|')
            escaped += "\\|";
        else
            escaped += c;
    }
    return escaped.empty() ? " " : escaped;
}

static std::string TableKey(SpecColumnRow const& row)
{
    return row.source + "_" + row.table;
}

static void WriteFile(std::filesystem::path const& path, std::string const& text)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
}

static std::string CountLine(std::map<std::string, uint32_t> const& counts)
{
    std::string line;
    for (auto const& [name, count] : counts)
        line += (line.empty() ? "" : ", ") + name + " " + std::to_string(count);
    return line;
}

SpecRenderResult RenderSpec(std::filesystem::path const& specDir, std::filesystem::path const& outDir)
{
    SpecRenderResult result;

    std::vector<SpecColumnRow> const columns = ReadColumnsTsv(specDir / "columns.tsv");
    std::vector<SpecDbcColumnRow> const dbcColumns = ReadDbcColumnsTsv(specDir / "dbc-columns.tsv");
    std::vector<SpecEvidenceRow> const evidence = ReadEvidenceTsv(specDir / "evidence.tsv");
    if (columns.empty())
    {
        result.error = (specDir / "columns.tsv").string() + " has no rows (run spec init)";
        return result;
    }

    std::map<std::tuple<std::string, std::string, std::string>, std::string> matchRates;
    for (SpecEvidenceRow const& row : evidence)
    {
        if (!row.matchRate.empty())
            matchRates[{ row.source, row.table, row.column }] = row.counterpart + " (" + row.matchRate + ")";
    }

    for (char const* folder : { "dbc", "server", "gone" })
        std::filesystem::remove_all(outDir / folder);

    // Group rows by source table and by dbc_ target table.
    std::map<std::string, std::vector<SpecColumnRow const*>> byTable;
    std::map<std::string, std::vector<SpecColumnRow const*>> byTarget;
    std::map<std::string, std::map<std::string, uint32_t>> dispositionsBySource;
    uint32_t reviewed = 0;
    for (SpecColumnRow const& row : columns)
    {
        byTable[TableKey(row)].push_back(&row);
        ++dispositionsBySource[row.source][row.disposition.empty() ? "unset" : row.disposition];
        if (row.reviewed == "yes")
            ++reviewed;
        if (row.disposition == "dbc")
            byTarget[row.target.substr(0, row.target.find('.'))].push_back(&row);
    }

    // dbc/<table>.md
    std::map<std::string, std::vector<SpecDbcColumnRow const*>> dbcByTable;
    for (SpecDbcColumnRow const& row : dbcColumns)
        dbcByTable[row.table].push_back(&row);

    std::vector<std::string> dbcPages;
    for (auto const& [target, rows] : byTarget)
    {
        std::ostringstream page;
        page << "# " << target << "\n\n"
             << "| column | source | rule | counterpart (match rate) | reason |\n|---|---|---|---|---|\n";
        std::set<std::string> filled;
        for (SpecColumnRow const* row : rows)
        {
            filled.insert(row->target.substr(row->target.find('.') + 1));
            auto rate = matchRates.find({ row->source, row->table, row->column });
            page << "| " << Cell(row->target.substr(row->target.find('.') + 1)) << " | "
                 << row->source << ":" << row->table << "." << row->column << " | " << Cell(row->rule) << " | "
                 << Cell(rate == matchRates.end() ? "" : rate->second) << " | " << Cell(row->reason) << " |\n";
        }

        bool heading = false;
        for (SpecDbcColumnRow const* row : dbcByTable[target])
        {
            if (filled.count(row->column))
                continue;
            if (!heading)
            {
                page << "\n## 1.15-only columns\n\n| column | mangosd reads | vanilla value |\n|---|---|---|\n";
                heading = true;
            }
            page << "| " << Cell(row->column) << " | " << Cell(row->mangosdReads) << " | " << Cell(row->vanillaValue) << " |\n";
        }

        WriteFile(outDir / "dbc" / (target + ".md"), page.str());
        dbcPages.push_back(target);
        ++result.pagesWritten;
    }

    // server/ and gone/ pages
    std::vector<std::string> serverPages;
    std::vector<std::string> gonePages;
    for (auto const& [key, rows] : byTable)
    {
        bool keeps = false;
        for (SpecColumnRow const* row : rows)
            keeps = keeps || row->disposition == "server" || row->disposition == "derived" || row->disposition == "constant";

        std::ostringstream page;
        page << "# " << rows.front()->source << ":" << rows.front()->table << "\n\n";
        if (keeps)
        {
            page << "| column | disposition | target / value | rule | reason |\n|---|---|---|---|---|\n";
            for (SpecColumnRow const* row : rows)
            {
                page << "| " << Cell(row->column) << " | " << Cell(row->disposition) << " | " << Cell(row->target) << " | "
                     << Cell(row->rule) << " | " << Cell(row->reason) << " |\n";
            }
        }
        else
        {
            page << "| column | destination or reason |\n|---|---|\n";
            for (SpecColumnRow const* row : rows)
            {
                std::string const detail = row->disposition == "dbc" ? "-> " + row->target + " (" + row->rule + ")" +
                    (row->reason.empty() ? "" : "; " + row->reason) : row->disposition + ": " + row->reason;
                page << "| " << Cell(row->column) << " | " << Cell(detail) << " |\n";
            }
        }

        WriteFile(outDir / (keeps ? "server" : "gone") / (key + ".md"), page.str());
        (keeps ? serverPages : gonePages).push_back(key);
        ++result.pagesWritten;
    }

    // index.md
    std::ostringstream index;
    index << "# Target schema spec\n\n"
          << "Every row says where one column of an *original* table (`srv_` vmangos, `vdbc_` 1.12 DBC) ends up "
             "in the finished world database. In the finished database a `dbc_` table is the one source of truth "
             "for every value it can hold, with no distinction by where a row came from; server tables hold only "
             "what no `dbc_` column can. Rules, match rates, \"overwrite\" and \"vanilla wins\" describe the "
             "migration that builds it, not the finished database (`plan.md` section 0).\n\n"
          << columns.size() << " columns, " << reviewed << " reviewed.\n\n"
          << "## Coverage\n\n| source | dispositions |\n|---|---|\n";
    for (auto const& [source, counts] : dispositionsBySource)
        index << "| " << source << " | " << CountLine(counts) << " |\n";

    auto list = [&](char const* title, char const* folder, std::vector<std::string> const& names)
    {
        index << "\n## " << title << " (" << names.size() << ")\n\n";
        for (std::string const& name : names)
            index << "- [" << name << "](" << folder << "/" << name << ".md)\n";
    };
    list("Client tables written", "dbc", dbcPages);
    list("Server tables kept", "server", serverPages);
    list("Tables that go away", "gone", gonePages);

    WriteFile(outDir / "index.md", index.str());
    ++result.pagesWritten;
    return result;
}
