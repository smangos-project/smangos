using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>
/// The liquid-type/animation parameters shared by everything that renders through
/// <see cref="WowLiquidMaterialType"/>: <c>WaterSurfaceMeshFunction</c> and
/// <c>RiverSurfaceMeshFunction</c>, which declare these as their own <see cref="MeshParameter"/>s, and
/// <see cref="WowLiquidMaterialType"/> itself, which still exposes them as an ordinary
/// <see cref="IMeshMaterialType"/> for anything that *does* want the generic preset-binding path later.
/// </summary>
public static class WowLiquidParameters
{
    private const string LiquidTypeIdName = "liquid_type_id";
    private const string LiquidTypeIdDisplayName = "Liquid Type";
    private const string LiquidTypeIdDescription = "The LiquidType.dbc row this surface renders as.";

    /// <summary>Name/default only — for reading a value back out via
    /// <see cref="MeshParameterValues.GetInt(MeshParameter)"/>/<see cref="MeshMaterialBuildContext.Choice"/>,
    /// neither of which reads <see cref="MeshParameter.Options"/>. <see cref="LiquidTypeIdChoice"/> builds
    /// the real, live-optioned parameter the inspector draws.</summary>
    public static readonly MeshParameter LiquidTypeIdLookup =
        MeshParameter.Choice(LiquidTypeIdName, LiquidTypeIdDisplayName, [], "1", LiquidTypeIdDescription);

    public static readonly MeshParameter FrameDuration =
        MeshParameter.Float("frame_duration", "Frame duration", 0.15f, 0.01f, 5.0f, "Seconds each animation frame is held before advancing.");

    /// <summary>Options are built live from the <see cref="LiquidType"/> catalog every time this is
    /// read, so the dropdown only ever offers ids the client's liquid table actually defines — "select
    /// a valid liquid type" falls out of <see cref="MeshParameterKind.Choice"/> needing no compile-time
    /// option list.</summary>
    public static MeshParameter LiquidTypeIdChoice(EditorContext context) =>
        MeshParameter.Choice(LiquidTypeIdName, LiquidTypeIdDisplayName, LiquidTypeOptions(context), "1", LiquidTypeIdDescription);

    /// <summary>Builds the <see cref="MeshMaterial"/> a surface should render through, straight from
    /// resolved parameter values — no preset lookup, no format involved. Both water functions call this
    /// directly from their own <c>Build</c> rather than going through
    /// <see cref="ProceduralBuildContext.Material"/>.</summary>
    public static MeshMaterial BuildMaterial(int liquidTypeId, float frameDuration)
    {
        var values = new MeshParameterValues();
        values.Set(LiquidTypeIdLookup, liquidTypeId);
        values.Set(FrameDuration, frameDuration);
        return new MeshMaterial(WowLiquidMaterialType.TypeId, values);
    }

    private static IReadOnlyList<MeshParameterOption> LiquidTypeOptions(EditorContext context)
    {
        List<MeshParameterOption> options = WowLiquidCatalog.All(context)
            .Select(row => new MeshParameterOption(
                row.LiquidTypeId.ToString(System.Globalization.CultureInfo.InvariantCulture),
                $"{row.Name} ({row.BasicType})"))
            .ToList();

        return options.Count > 0 ? options : [new MeshParameterOption("1", "Water (no client configured)")];
    }
}
