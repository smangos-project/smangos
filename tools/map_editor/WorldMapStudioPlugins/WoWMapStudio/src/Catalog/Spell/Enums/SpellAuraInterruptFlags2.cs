namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellinterrupts.AuraInterruptFlags2` / `ChannelInterruptFlags2`.</summary>
[System.Flags]
public enum SpellAuraInterruptFlags2 : uint
{
    None = 0x0,
    Falling = 0x1,
    Swimming = 0x2,
    Notmoving = 0x4,
    Ground = 0x8,
    Transform = 0x10,
    Jump = 0x20,
    Changespec = 0x40,
    Abandonvehicle = 0x80,
    Startofraidencounterandstartofmythicplus = 0x100,
    Endofraidencounterandstartofmythicplus = 0x200,
    Disconnect = 0x400,
    Enteringinstance = 0x800,
    Duelend = 0x1000,
    Leavearenaorbattleground = 0x2000,
    Changetalent = 0x4000,
    Changeglyph = 0x8000,
    Seamlesstransfer = 0x10000,
    Warmodeleave = 0x20000,
    Touchingground = 0x40000,
    Chromietime = 0x80000,
    Splineflightorfreeflight = 0x100000,
    Procorperiodicattacking = 0x200000,
    Challengemodestart = 0x400000,
    Startofencounter = 0x800000,
    Endofencounter = 0x1000000,
}
