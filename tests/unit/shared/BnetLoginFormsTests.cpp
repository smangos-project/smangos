#include "Bnet/BnetLoginForms.h"

#include <catch_amalgamated.hpp>

TEST_CASE("BnetLoginForms round-trips a form-inputs body", "[bnet]")
{
    std::vector<std::pair<std::string, std::string>> const inputs = {
        { "public_A", "deadbeef" },
        { "client_evidence_M1", "cafef00d" },
    };

    std::string const body = BnetLoginForms::Build(inputs);

    nonstd::optional<std::vector<std::pair<std::string, std::string>>> parsed = BnetLoginForms::Parse(body);
    REQUIRE(parsed.has_value());
    CHECK(*parsed == inputs);
}

TEST_CASE("BnetLoginForms::Build matches LoginRESTService's ParseFormInputs shape", "[bnet]")
{
    std::string const body = BnetLoginForms::Build({ { "account_name", "bot1@bots" } });
    CHECK(body == R"({"inputs":[{"input_id":"account_name","value":"bot1@bots"}]})");
}

TEST_CASE("BnetLoginForms::Parse rejects malformed or missing-shape bodies", "[bnet]")
{
    CHECK_FALSE(BnetLoginForms::Parse("not json").has_value());
    CHECK_FALSE(BnetLoginForms::Parse(R"({"foo":"bar"})").has_value());
    CHECK_FALSE(BnetLoginForms::Parse(R"({"inputs":[{"input_id":"x"}]})").has_value());
}
