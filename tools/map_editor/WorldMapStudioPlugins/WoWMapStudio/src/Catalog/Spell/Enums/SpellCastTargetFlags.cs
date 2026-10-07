namespace WorldMapStudio.WoW;

/// <summary>`dbc_spelltargetrestrictions.Targets` — target-type mask.</summary>
[System.Flags]
public enum SpellCastTargetFlags : uint
{
    None = 0x0,
    Unit = 0x2,
    UnitRaid = 0x4,
    UnitParty = 0x8,
    Item = 0x10,
    SourceLocation = 0x20,
    DestLocation = 0x40,
    UnitEnemy = 0x80,
    UnitAlly = 0x100,
    CorpseEnemy = 0x200,
    UnitDead = 0x400,
    Gameobject = 0x800,
    TradeItem = 0x1000,
    String = 0x2000,
    GameobjectItem = 0x4000,
    CorpseAlly = 0x8000,
    UnitMinipet = 0x10000,
    GlyphSlot = 0x20000,
    DestTarget = 0x40000,
    ExtraTargets = 0x80000,
    UnitPassenger = 0x100000,
    Unk400000 = 0x400000,
    Unk1000000 = 0x1000000,
    Unk4000000 = 0x4000000,
    Unk10000000 = 0x10000000,
    Unk40000000 = 0x40000000,
}
