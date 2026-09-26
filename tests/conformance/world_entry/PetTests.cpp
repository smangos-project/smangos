// A summoned pet: its action bar, name lookup, commands, casts and dismissal.
#include "Packets/PetMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr uint32 ActionCommand = 0x07;
constexpr uint32 CommandAttack = 2;

uint32 ActionButtonType(uint32 button) { return button >> 24; }
}

TEST_CASE("pet: a summoned imp gets its action bar, answers commands and is dismissed", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        SayCommand(gm, ".cast 688 triggered"); // Summon Imp
        REQUIRE(co_await gm.WaitReceived<PetSpellsMessage>(15000));
        auto spells = gm.FindReceived<PetSpellsMessage>();
        REQUIRE(spells);

        ClassicGuid const pet = spells->Pet;
        CHECK_FALSE(pet.IsEmpty());
        CHECK(spells->TimeLimit == 0);

        uint32 commands = 0;
        for (uint32 const button : spells->ActionButtons)
            commands += ActionButtonType(button) == ActionCommand ? 1 : 0;
        CHECK(commands >= 3);

        QueryPetName nameQuery;
        nameQuery.Pet = pet;
        gm.Send(nameQuery);
        auto name = co_await gm.Packet<QueryPetNameResponse>(10000, [&](QueryPetNameResponse const& p) { return p.Pet == pet; });
        REQUIRE(name);
        CHECK(name->Allow);
        CHECK_FALSE(name->Name.empty());

        // An attack command with nothing selected is refused with feedback.
        PetAction attack;
        attack.Pet = pet;
        attack.Action = CommandAttack | (ActionCommand << 24);
        gm.Send(attack);
        REQUIRE(co_await gm.WaitReceived<PetActionFeedback>(10000));

        // A castable spell with no target is refused, under the request's own cast id.
        uint32 castable = 0;
        for (uint32 const action : spells->ActionButtons)
        {
            uint32 const type = ActionButtonType(action);
            if (type == 0x81 || type == 0xC1)
            {
                castable = action & 0x00FFFFFF;
                break;
            }
        }
        REQUIRE(castable != 0);
        {
            PetCastSpell cast;
            cast.Pet = pet;
            cast.Cast.CastId = { 424242, 0x1000000000000000ULL };
            cast.Cast.SpellId = int32(castable);
            gm.Send(cast);
            auto failed = co_await gm.Packet<PetCastFailed>(10000, [&](PetCastFailed const& p) { return p.SpellId == int32(castable); });
            REQUIRE(failed);
            CHECK(failed->CastId == cast.Cast.CastId);
        }

        PetAbandon abandon;
        abandon.Pet = pet;
        gm.Send(abandon);
        REQUIRE(co_await gm.WaitReceived<PetClearSpells>(10000));
    });
}
