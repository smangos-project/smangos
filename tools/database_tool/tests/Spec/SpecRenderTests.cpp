#include "Spec/SpecRender.h"

#include "Spec/SpecFile.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace
{
    struct TempDirs
    {
        std::filesystem::path spec;
        std::filesystem::path out;

        explicit TempDirs(std::string const& name)
            : spec(std::filesystem::temp_directory_path() / ("database_tool_test_specrender_" + name) / "spec")
            , out(std::filesystem::temp_directory_path() / ("database_tool_test_specrender_" + name) / "out")
        {
            std::filesystem::remove_all(spec.parent_path());
            std::filesystem::create_directories(spec);
        }

        ~TempDirs() { std::filesystem::remove_all(spec.parent_path()); }
    };

    std::string ReadAll(std::filesystem::path const& path)
    {
        std::ifstream in(path, std::ios::binary);
        std::ostringstream text;
        text << in.rdbuf();
        return text.str();
    }
}

TEST_CASE("RenderSpec errors on an empty spec", "[SpecRender]")
{
    TempDirs dirs("empty");
    CHECK_FALSE(RenderSpec(dirs.spec, dirs.out).error.empty());
}

TEST_CASE("RenderSpec writes target, kept and vanished table pages and an index", "[SpecRender]")
{
    TempDirs dirs("pages");
    WriteColumnsTsv(dirs.spec / "columns.tsv", {
        { "vdbc", "bar", "ID", "dbc", "dbc_bar.ID", "identity", "", "yes" },
        { "vdbc", "bar", "Junk", "drop", "", "", "not imported", "yes" },
        { "srv", "foo", "id", "server", "", "", "a|b", "yes" },
        { "srv", "foo", "gone", "drop", "", "", "unused", "" },
    });
    WriteDbcColumnsTsv(dirs.spec / "dbc-columns.tsv", { { "dbc_bar", "ID", "", "" }, { "dbc_bar", "NewCol", "yes", "0" } });
    WriteEvidenceTsv(dirs.spec / "evidence.tsv", { { "vdbc", "bar", "ID", "INTEGER", "none", "", "cdbc_bar.ID", "identity", "1.0", "" } });

    SpecRenderResult result = RenderSpec(dirs.spec, dirs.out);
    CHECK(result.error.empty());
    CHECK(result.pagesWritten == 4);

    std::string target = ReadAll(dirs.out / "dbc" / "dbc_bar.md");
    CHECK(target.find("| ID | vdbc:bar.ID | identity | cdbc_bar.ID (1.0) |") != std::string::npos);
    CHECK(target.find("| NewCol | yes | 0 |") != std::string::npos);

    CHECK(ReadAll(dirs.out / "server" / "srv_foo.md").find("a\\|b") != std::string::npos);
    CHECK(ReadAll(dirs.out / "gone" / "vdbc_bar.md").find("-> dbc_bar.ID (identity)") != std::string::npos);

    std::string index = ReadAll(dirs.out / "index.md");
    CHECK(index.find("3 reviewed") != std::string::npos);
    CHECK(index.find("[dbc_bar](dbc/dbc_bar.md)") != std::string::npos);
}
