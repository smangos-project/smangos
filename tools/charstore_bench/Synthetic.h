#ifndef CHARSTORE_BENCH_SYNTHETIC_H
#define CHARSTORE_BENCH_SYNTHETIC_H

#include "Persistence/CharTables.h"
#include "Persistence/EpochBuffer.h"

#include <array>
#include <random>
#include <string>
#include <vector>

namespace CharStoreBench
{
    using RowCounts = std::array<uint32, Persistence::CharTableCount>;

    // Rows per table of a rich level-60 player
    RowCounts DefaultRowCounts();
    // "table=n,table=n" with registry table names
    bool ParseRowCounts(std::string const& text, RowCounts& counts, std::string& error);

    // Deterministic character data for players [0, players). Every row has a version and a key
    // generation: a new version changes its values, a new key generation replaces the row with
    // another key (delete + insert).
    class SyntheticRealm
    {
        public:
            SyntheticRealm(RowCounts const& counts, uint32 players, uint64 firstGuid);

            uint32 Players() const { return uint32(m_players.size()); }
            uint64 Guid(uint32 player) const { return m_firstGuid + player; }
            uint64 RowsPerPlayer() const;

            // Changes the characters row of every player plus `changes` random rows per player;
            // keyChangePercent of those get a new key instead of new values.
            void Mutate(uint32 changes, uint32 keyChangePercent, std::mt19937_64& rng);
            // Writes the player's images: every table, or only the tables Mutate touched.
            void Capture(Persistence::RowWriter& writer, uint32 player, bool dirtyOnly) const;
            void ClearDirty();

        private:
            struct TableState
            {
                std::vector<uint32> version;
                std::vector<uint32> keyGen;
                bool dirty = false;
            };

            void WriteRow(Persistence::RowWriter& writer, uint16 table, uint64 owner, uint32 row, uint32 version, uint32 keyGen) const;

            RowCounts m_counts;
            uint64 m_firstGuid;
            std::vector<std::array<TableState, Persistence::CharTableCount>> m_players;
            std::vector<std::pair<uint32, uint16>> m_weights;   // cumulative row count -> table
            mutable std::string m_text;
    };
}

#endif
