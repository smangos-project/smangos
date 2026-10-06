using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>The <c>SoundBank</c> column of <c>dbc_liquidtype</c> — what a liquid id fundamentally
/// behaves like. Values match the game's own: Water 0, Ocean 1, Magma 2, Slime 3.</summary>
public enum LiquidBasicType
{
    Water = 0,
    Ocean = 1,
    Magma = 2,
    Slime = 3,
}

/// <summary>
/// A <c>dbc_liquidtype</c> row as a catalog entity — the client's liquid table (id, name, basic type,
/// animation, and the texture template a surface renders through). Read-only reference data:
/// <see cref="LiquidTypeFactory"/> loads it whole for lookups and browsing but gives it no
/// create/edit surface. The database is the authority for DBC data in this project.
/// </summary>
public sealed class LiquidType : CatalogEntity
{
    [ScriptProperty]
    public int LiquidTypeId { get; set; }

    [ScriptProperty]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty]
    public LiquidBasicType BasicType { get; set; }

    /// <summary><c>dbc_liquidmaterial</c> key — its <c>LVF</c> is the on-disk liquid vertex format.</summary>
    [ScriptProperty]
    public int MaterialId { get; set; }

    /// <summary>Animation frame FileDataIDs from <c>dbc_liquidtypextexture</c> (type 0, by order index).
    /// Empty for a type with no client texture (procedural water).</summary>
    public IReadOnlyList<int> FrameFileDataIds { get; set; } = [];

    /// <summary><c>Float1</c>: UV scale for water/ocean, scroll rate for magma/slime — see
    /// <see cref="WowLiquidMaterialType"/>.</summary>
    [ScriptProperty]
    public float AnimationX { get; set; }

    /// <summary><c>Float2</c>: UV rotation (degrees) for water/ocean, unused for magma/slime.</summary>
    [ScriptProperty]
    public float AnimationY { get; set; }

    public override string DisplayName => $"Liquid {LiquidTypeId}: {(Name.Length > 0 ? Name : "(unnamed)")} ({BasicType})";
}
