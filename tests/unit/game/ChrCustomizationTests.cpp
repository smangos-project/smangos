#include "Objects/ChrCustomization.h"
#include <catch_amalgamated.hpp>

// Race 1 male: skin 1 (101, 102), face 2 (201, 202 hidden from class 6, 203 masked by a req without the
// requirements bit), hair 3 (301 needs skin 101). Race 2 male has only option 4. Option 5 has no choices.
static ChrCustomizationStore MakeStore()
{
    ChrCustomizationStore store;
    store.AddRaceModel(1, 0, 10);
    store.AddRaceModel(2, 0, 11);
    store.AddOption(1, 10, 0);
    store.AddOption(2, 10, 0);
    store.AddOption(3, 10, 0);
    store.AddOption(5, 10, 0);
    store.AddOption(4, 11, 0);

    store.AddReq(1, 1, 0, ~(1 << 5), 0, 0, 0);
    store.AddReq(2, 2, 0, 1, 0, 0, 0);
    store.AddReq(3, 1, 0, 0, 0, 0, 0);
    store.AddReq(4, 1, 0, 0, 7, 0, 0);

    store.AddChoice(101, 1, 0, 0);
    store.AddChoice(102, 1, 0, 1);
    store.AddChoice(201, 2, 0, 1);
    store.AddChoice(202, 2, 1, 0);
    store.AddChoice(203, 2, 2, 2);
    store.AddChoice(301, 3, 3, 0);
    store.AddChoice(401, 4, 0, 0);
    store.AddChoice(402, 4, 4, 1);
    store.AddReqChoice(3, 101);
    store.Finish();
    return store;
}

static ChrCustomizationList Look(uint32 skin, uint32 face, uint32 hair)
{
    return { { 1, skin }, { 2, face }, { 3, hair } };
}

TEST_CASE("Customization options without choices are dropped", "[customization]")
{
    CHECK(MakeStore().GetOptionCount() == 4);
}

TEST_CASE("Customization validation", "[customization]")
{
    ChrCustomizationStore store = MakeStore();

    SECTION("valid lists")
    {
        CHECK(store.Validate(1, 1, 0, Look(101, 201, 301)));
        CHECK(store.Validate(1, 1, 0, { { 1, 102 } }));
        CHECK(store.Validate(1, 1, 0, {}));
    }
    SECTION("structure")
    {
        CHECK_FALSE(store.Validate(1, 1, 0, { { 1, 101 }, { 1, 102 } }));
        CHECK_FALSE(store.Validate(1, 1, 0, { { 2, 201 }, { 1, 101 } }));
        CHECK_FALSE(store.Validate(1, 1, 0, { { 4, 401 } }));
        CHECK_FALSE(store.Validate(1, 1, 0, { { 5, 101 } }));
        CHECK_FALSE(store.Validate(1, 1, 1, { { 1, 101 } }));
        CHECK_FALSE(store.Validate(1, 1, 0, { { 1, 201 } }));
    }
    SECTION("class mask")
    {
        CHECK(store.Validate(1, 1, 0, { { 2, 202 } }));
        CHECK_FALSE(store.Validate(1, 6, 0, { { 2, 202 } }));
    }
    SECTION("req without the requirements bit ignores its masks")
    {
        CHECK(store.Validate(1, 1, 0, { { 2, 203 } }));
        CHECK(store.Validate(1, 6, 0, { { 2, 203 } }));
    }
    SECTION("required choice")
    {
        CHECK(store.Validate(1, 1, 0, { { 1, 101 }, { 3, 301 } }));
        CHECK_FALSE(store.Validate(1, 1, 0, { { 1, 102 }, { 3, 301 } }));
        CHECK_FALSE(store.Validate(1, 1, 0, { { 3, 301 } }));
    }
    SECTION("unsupported requirement is unmet")
    {
        CHECK_FALSE(store.Validate(2, 1, 0, { { 4, 402 } }));
        CHECK(store.Validate(2, 1, 0, { { 4, 401 } }));
    }
}

TEST_CASE("Customization completion", "[customization]")
{
    ChrCustomizationStore store = MakeStore();

    SECTION("fills missing options with their first valid choice")
    {
        ChrCustomizationList list;
        CHECK(store.Complete(1, 6, 0, list));
        CHECK(list == ChrCustomizationList{ { 1, 101 }, { 2, 201 }, { 3, 301 } });
    }
    SECTION("replaces invalid pairs")
    {
        ChrCustomizationList list = { { 2, 202 }, { 1, 999 }, { 9, 1 }, { 3, 301 } };
        CHECK(store.Complete(1, 6, 0, list));
        CHECK(list == ChrCustomizationList{ { 1, 101 }, { 2, 201 }, { 3, 301 } });
    }
    SECTION("keeps a valid list")
    {
        ChrCustomizationList partial = { { 1, 102 }, { 2, 202 } };
        CHECK_FALSE(store.Complete(1, 1, 0, partial));

        ChrCustomizationList full = { { 1, 101 }, { 2, 202 }, { 3, 301 } };
        CHECK_FALSE(store.Complete(1, 1, 0, full));
        CHECK(full == ChrCustomizationList{ { 1, 101 }, { 2, 202 }, { 3, 301 } });
    }
    SECTION("unknown race and sex give an empty list")
    {
        ChrCustomizationList list = { { 1, 101 } };
        CHECK(store.Complete(3, 1, 0, list));
        CHECK(list.empty());
    }
}

TEST_CASE("Random customization always validates", "[customization]")
{
    ChrCustomizationStore store = MakeStore();
    for (int i = 0; i < 200; ++i)
    {
        ChrCustomizationList list;
        store.SelectRandom(1, 6, 0, list);
        CHECK(list.size() >= 2);
        CHECK(store.Validate(1, 6, 0, list));
        store.SelectRandom(2, 1, 0, list);
        CHECK(list.size() == 1);
        CHECK(store.Validate(2, 1, 0, list));
    }
}
