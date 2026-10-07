namespace WorldMapStudio.WoW;

/// <summary>`dbc_spell.SchoolMask` — resistance schools, one bit each.</summary>
[System.Flags]
public enum SpellSchoolMaskFlags : uint
{
    None = 0,
    Normal = 0x1,
    Holy = 0x2,
    Fire = 0x4,
    Nature = 0x8,
    Frost = 0x10,
    Shadow = 0x20,
    Arcane = 0x40,
}
