namespace WorldMapStudio.WoW;

/// <summary>
/// MH2O: a tile's liquid, authored as a lattice-aligned surface and written into the ADT's per-tile
/// water chunk rather than a file of its own — <see cref="Extension"/> is empty, like
/// <see cref="MeshModelFormat"/>. <see cref="CanAuthor"/> is what makes "a procedural model that is
/// MH2O" a real thing: <c>WaterSurfaceMeshFunction</c>/<c>RiverSurfaceMeshFunction</c> bind their one
/// output slot here, so <see cref="ProceduralBuildContext.MaterialType"/> reports <c>wow.liquid</c>
/// honestly instead of the standard material type. The functions still build their own
/// <see cref="MeshMaterial"/> from their parameters; instantiation uses that material as-is (see
/// <see cref="ModelAsset.Instantiate"/>), so routing through this format does not change how water
/// renders.
/// </summary>
[Subsystem(nameof(ModelFormatSystem))]
public sealed class Mh2oFormat : IModelFormat
{
    public const string FormatId = "wow.format.mh2o";

    public Mh2oFormat(ModelFormatSystem system)
    {
    }

    public string Id => FormatId;

    public string DisplayName => "Liquid (MH2O)";

    public string Extension => "";

    public string MaterialTypeId => WowLiquidMaterialType.TypeId;

    public bool CanAuthor => true;
}
