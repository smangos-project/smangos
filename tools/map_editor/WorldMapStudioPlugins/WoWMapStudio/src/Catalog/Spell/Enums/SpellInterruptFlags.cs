namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellinterrupts.InterruptFlags`.</summary>
[System.Flags]
public enum SpellInterruptFlags : uint
{
    None = 0x0,
    Movement = 0x1,
    Damagepushbackplayeronly = 0x2,
    Stun = 0x4,
    Combat = 0x8,
    Damagecancelsplayeronly = 0x10,
    Meleecombat = 0x20,
    Immunity = 0x40,
    Damageabsorb = 0x80,
    Zerodamagecancels = 0x100,
    Damagepushback = 0x200,
    Damagecancels = 0x400,
}
