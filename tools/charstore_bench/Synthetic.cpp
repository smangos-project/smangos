#include "Synthetic.h"

#include <cstring>
#include <sstream>

using namespace Persistence;

namespace CharStoreBench
{
    namespace
    {
        uint64 Mix(uint64 a, uint64 b)
        {
            uint64 x = a * 0x9E3779B97F4A7C15ull ^ (b + 0x632BE59BD9B4E019ull + (a << 6) + (a >> 2));
            x ^= x >> 31;
            x *= 0xBF58476D1CE4E5B9ull;
            return x ^ (x >> 29);
        }

        // Text widths seen in the dev characters DB, level-60 explored zones are wider
        size_t TextWidth(char const* column)
        {
            static std::pair<char const*, size_t> const widths[] = {
                { "explored_zones", 300 }, { "equipment_cache", 90 }, { "known_taxi_mask", 40 }, { "enchantments", 42 },
                { "charges", 10 }, { "action_bar_data", 51 }, { "teach_spell_data", 16 }, { "name", 10 }, { "subject", 20 },
            };
            for (auto const& width : widths)
                if (std::strcmp(width.first, column) == 0)
                    return width.second;
            return 16;
        }
    }

    RowCounts DefaultRowCounts()
    {
        RowCounts counts{};
        counts[Characters] = 1;
        counts[Customizations] = 6;
        counts[BattlegroundData] = 1;
        counts[Inventory] = 180;
        counts[ItemInstance] = 180;
        counts[QuestStatus] = 450;
        counts[Spell] = 220;
        counts[SpellCooldown] = 3;
        counts[Aura] = 8;
        counts[Skills] = 25;
        counts[Reputation] = 57;
        counts[HonorCp] = 5;
        counts[Action] = 90;
        counts[Tutorial] = 1;
        counts[Social] = 10;
        counts[Mail] = 10;
        counts[MailItems] = 5;
        counts[Pet] = 1;
        counts[PetSpell] = 15;
        counts[Homebind] = 1;
        counts[Instance] = 2;
        return counts;
    }

    bool ParseRowCounts(std::string const& text, RowCounts& counts, std::string& error)
    {
        std::stringstream stream(text);
        std::string item;
        while (std::getline(stream, item, ','))
        {
            size_t const eq = item.find('=');
            std::string const name = item.substr(0, eq);
            uint16 table = 0;
            while (table < CharTableCount && name != CharTableRegistry().Get(table).name)
                ++table;
            if (eq == std::string::npos || table == CharTableCount)
            {
                error = "bad row count '" + item + "'";
                return false;
            }
            counts[table] = uint32(std::stoul(item.substr(eq + 1)));
        }
        return true;
    }

    SyntheticRealm::SyntheticRealm(RowCounts const& counts, uint32 players, uint64 firstGuid) : m_counts(counts), m_firstGuid(firstGuid)
    {
        m_players.resize(players);
        for (auto& player : m_players)
            for (uint16 table = 0; table < CharTableCount; ++table)
            {
                player[table].version.assign(counts[table], 0);
                player[table].keyGen.assign(counts[table], 0);
            }

        uint32 total = 0;
        for (uint16 table = 0; table < CharTableCount; ++table)
            if (table != Characters && counts[table])
                m_weights.push_back({ total += counts[table], table });
    }

    uint64 SyntheticRealm::RowsPerPlayer() const
    {
        uint64 rows = 0;
        for (uint32 count : m_counts)
            rows += count;
        return rows;
    }

    void SyntheticRealm::Mutate(uint32 changes, uint32 keyChangePercent, std::mt19937_64& rng)
    {
        uint32 const total = m_weights.empty() ? 0 : m_weights.back().first;
        for (auto& player : m_players)
        {
            if (m_counts[Characters])
            {
                ++player[Characters].version[0];
                player[Characters].dirty = true;
            }
            for (uint32 i = 0; total && i < changes; ++i)
            {
                uint32 const pick = uint32(rng() % total);
                uint16 const table = std::upper_bound(m_weights.begin(), m_weights.end(), std::make_pair(pick, uint16(0xFFFF)))->second;
                TableState& state = player[table];
                uint32 const row = uint32(rng() % m_counts[table]);
                if (rng() % 100 < keyChangePercent)
                    ++state.keyGen[row];
                else
                    ++state.version[row];
                state.dirty = true;
            }
        }
    }

    void SyntheticRealm::ClearDirty()
    {
        for (auto& player : m_players)
            for (TableState& state : player)
                state.dirty = false;
    }

    void SyntheticRealm::Capture(RowWriter& writer, uint32 player, bool dirtyOnly) const
    {
        uint64 const owner = Guid(player);
        for (uint16 table = 0; table < CharTableCount; ++table)
        {
            TableState const& state = m_players[player][table];
            if (!m_counts[table] || (dirtyOnly && !state.dirty))
                continue;
            writer.BeginImage(table, owner);
            for (uint32 row = 0; row < m_counts[table]; ++row)
                WriteRow(writer, table, owner, row, state.version[row], state.keyGen[row]);
            writer.EndImage();
        }
    }

    void SyntheticRealm::WriteRow(RowWriter& writer, uint16 table, uint64 owner, uint32 row, uint32 version, uint32 keyGen) const
    {
        TableDesc const& desc = CharTableRegistry().Get(table);
        bool ownerInKey = false;
        for (uint8 i = 0; i < desc.keyCols; ++i)
            ownerInKey |= std::strcmp(desc.cols[i].name, desc.ownerCol) == 0;

        // the last key column makes the key unique: per owner when the owner is part of the key,
        // globally otherwise (item, mail and pet ids)
        int64 const uniqueKey = ownerInKey ? int64(row) * 1000 + keyGen : (int64(owner) * 100000 + row) * 1000 + keyGen;
        uint64 const seed = Mix(Mix(owner, table), Mix(row, uint64(version) << 32 | keyGen));

        writer.BeginRow();
        for (size_t i = 0; i < desc.cols.size(); ++i)
        {
            ColDesc const& col = desc.cols[i];
            if (std::strcmp(col.name, desc.ownerCol) == 0)
                writer.Int(int64(owner));
            else if (i < desc.keyCols)
                writer.Int(i + 1 == desc.keyCols ? uniqueKey : 0);
            else if (col.nullable && std::strncmp(col.name, "deleted_", 8) == 0)
                writer.Null();
            else
            {
                uint64 const value = Mix(seed, i);
                switch (col.type)
                {
                    case ColType::Int: writer.Int(int64(value % 5000)); break;
                    case ColType::Real: writer.Real(double(value % 100000) / 7.0); break;
                    default:
                    {
                        size_t const width = TextWidth(col.name);
                        m_text.assign(width, ' ');
                        for (size_t c = 0; c < width; c += 2)
                            m_text[c] = char('0' + (Mix(value, c) % 10));
                        if (col.type == ColType::Text)
                            writer.Text(m_text);
                        else
                            writer.Blob(m_text.data(), m_text.size());
                        break;
                    }
                }
            }
        }
        writer.EndRow();
    }
}
