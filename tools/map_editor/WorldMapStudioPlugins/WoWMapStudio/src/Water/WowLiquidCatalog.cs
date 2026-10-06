using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>
/// Reads the always-loaded <see cref="LiquidType"/> catalog. Everything WoW-liquid that needs
/// liquid type data (the rasterizer bridge, the material type, the authoring dropdown, the texture
/// library) resolves through here.
/// </summary>
public static class WowLiquidCatalog
{
    /// <summary>The <see cref="LiquidType"/> row for <paramref name="liquidTypeId"/>, or null when
    /// the catalog has no such id (no client configured, or an unknown id).</summary>
    public static LiquidType? Find(EditorContext context, int liquidTypeId) =>
        context.Catalog.OfType<LiquidType>().FirstOrDefault(row => row.LiquidTypeId == liquidTypeId);

    /// <summary>The basic behaviour of <paramref name="liquidTypeId"/>, defaulting to
    /// <see cref="LiquidBasicType.Water"/> when unknown — the same fallback the DBC reader used.</summary>
    public static LiquidBasicType BasicType(EditorContext context, int liquidTypeId) =>
        Find(context, liquidTypeId)?.BasicType ?? LiquidBasicType.Water;

    /// <summary>Every liquid type, ordered by id — what the authoring dropdown lists.</summary>
    public static IEnumerable<LiquidType> All(EditorContext context) =>
        context.Catalog.OfType<LiquidType>().OrderBy(row => row.LiquidTypeId);
}
