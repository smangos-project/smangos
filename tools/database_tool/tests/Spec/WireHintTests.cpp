#include "Spec/WireHint.h"

#include <catch_amalgamated.hpp>

TEST_CASE("IsPacketCodeFile recognizes the named packet files", "[WireHint]")
{
    CHECK(IsPacketCodeFile("src/game/QueryHandler.cpp", ""));
    CHECK(IsPacketCodeFile("src/game/GossipDef.cpp", ""));
    CHECK(IsPacketCodeFile("src/game/UpdateFieldFill.cpp", ""));
}

TEST_CASE("IsPacketCodeFile recognizes any *Handler.cpp file", "[WireHint]")
{
    CHECK(IsPacketCodeFile("src/game/MiscHandler.cpp", ""));
    CHECK(IsPacketCodeFile("src/game/Handlers/QuestHandler.cpp", ""));
    CHECK_FALSE(IsPacketCodeFile("src/game/HandlerUtils.cpp", ""));
}

TEST_CASE("IsPacketCodeFile recognizes any file that streams into a WorldPacket", "[WireHint]")
{
    CHECK(IsPacketCodeFile("src/game/ChatSystem.cpp", "WorldPacket data;\ndata << value;\n"));
    CHECK_FALSE(IsPacketCodeFile("src/game/ChatSystem.cpp", "WorldPacket data;\n")); // declared, never streamed into
    CHECK_FALSE(IsPacketCodeFile("src/game/ChatSystem.cpp", "int x = a << b;\n")); // shift, no WorldPacket
}

TEST_CASE("IsPacketCodeFile rejects an ordinary file", "[WireHint]")
{
    CHECK_FALSE(IsPacketCodeFile("src/game/ObjectMgr.cpp", "uint32 x = a->flags;\n"));
}

TEST_CASE("FindFieldUseLines finds whole-identifier . and -> accesses with line numbers", "[WireHint]")
{
    std::string text =
        "void F()\n"
        "{\n"
        "    Do(a->value);\n"
        "    Do(b.value);\n"
        "}\n";
    std::vector<WireHintUse> uses = FindFieldUseLines("F.cpp", text, "value");
    REQUIRE(uses.size() == 2);
    CHECK(uses[0].line == 3);
    CHECK(uses[1].line == 4);
    CHECK(uses[0].file == "F.cpp");
}

TEST_CASE("FindFieldUseLines does not match a field name that is a prefix of a longer identifier", "[WireHint]")
{
    std::string text = "Do(a->idx);\n";
    CHECK(FindFieldUseLines("F.cpp", text, "id").empty());
}
