#include "Spells/SpellDefines.h"
#include <catch_amalgamated.hpp>

TEST_CASE("Client spell effect numbers", "[spell]")
{
    CHECK(GetClientSpellEffect(SPELL_EFFECT_TELEPORT_UNITS) == 252);
    CHECK(GetClientSpellEffect(SPELL_EFFECT_ADD_COMBO_POINTS) == 328);
    CHECK(GetClientSpellEffect(SPELL_EFFECT_SUMMON_TOTEM_SLOT3) == 28);
    CHECK(GetClientSpellEffect(SPELL_EFFECT_SUMMON_OBJECT_SLOT2) == 321);
    CHECK(GetClientSpellEffect(SPELL_EFFECT_SCHOOL_DAMAGE) == SPELL_EFFECT_SCHOOL_DAMAGE);
}
