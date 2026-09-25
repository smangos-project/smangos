#include "BnetLoginForms.h"
#include <nlohmann/json.hpp>

std::string BnetLoginForms::Build(std::vector<std::pair<std::string, std::string>> const& inputs)
{
    nlohmann::json body;
    body["inputs"] = nlohmann::json::array();
    for (auto const& [id, value] : inputs)
        body["inputs"].push_back({ { "input_id", id }, { "value", value } });
    return body.dump();
}

nonstd::optional<std::vector<std::pair<std::string, std::string>>> BnetLoginForms::Parse(std::string const& jsonBody)
{
    try
    {
        nlohmann::json request = nlohmann::json::parse(jsonBody);
        std::vector<std::pair<std::string, std::string>> out;
        for (auto const& input : request.at("inputs"))
            out.emplace_back(input.at("input_id").get<std::string>(), input.at("value").get<std::string>());
        return out;
    }
    catch (nlohmann::json::exception const&)
    {
        return nonstd::nullopt;
    }
}
