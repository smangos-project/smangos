#include "MigrationState.h"

#include <unordered_set>

MigrationState CompareMigrations(std::vector<MigrationFile> const& files, std::vector<std::string> const& appliedIds)
{
    std::unordered_set<std::string> applied(appliedIds.begin(), appliedIds.end());
    std::unordered_set<std::string> fileIds;

    MigrationState state;
    for (MigrationFile const& file : files)
    {
        fileIds.insert(file.id);
        if (!applied.count(file.id))
            state.pending.push_back(file);
    }

    for (std::string const& id : appliedIds)
    {
        if (!fileIds.count(id))
            state.orphaned.push_back(id);
    }

    return state;
}
