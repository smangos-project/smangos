namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellrange.Flags`.</summary>
[System.Flags]
public enum SpellRangeFlags : uint
{
    None = 0,
    Melee = 0x1,
    Ranged = 0x2,
}
