#ifndef SHARED_BNETLOGINFORMS_H
#define SHARED_BNETLOGINFORMS_H

#include "nonstd/optional.hpp"
#include <string>
#include <utility>
#include <vector>

// The {"inputs":[{"input_id":"...","value":"..."}]} body the login portal's form-input
// endpoints (account_name, then the SRP proof) carry, matching LoginRESTService's
// ParseFormInputs on the way in.
class BnetLoginForms
{
    public:
        static std::string Build(std::vector<std::pair<std::string, std::string>> const& inputs);

        // Returns nullopt on anything that isn't the expected {"inputs":[...]} shape.
        static nonstd::optional<std::vector<std::pair<std::string, std::string>>> Parse(std::string const& jsonBody);
};

#endif
