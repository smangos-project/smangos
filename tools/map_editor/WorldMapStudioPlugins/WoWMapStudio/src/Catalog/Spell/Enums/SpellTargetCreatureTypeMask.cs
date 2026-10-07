namespace WorldMapStudio.WoW;

/// <summary>`dbc_spelltargetrestrictions.TargetCreatureType` — mask of creature types (bit N = type N+1).</summary>
[System.Flags]
public enum SpellTargetCreatureTypeMask : uint
{
    None = 0,
    Beast = 0x1,
    Dragonkin = 0x2,
    Demon = 0x4,
    Elemental = 0x8,
    Giant = 0x10,
    Undead = 0x20,
    Humanoid = 0x40,
    Critter = 0x80,
    Mechanical = 0x100,
    NotSpecified = 0x200,
    Totem = 0x400,
    NonCombatPet = 0x800,
    GasCloud = 0x1000,
    WildPet = 0x2000,
    Aberration = 0x4000,
}
