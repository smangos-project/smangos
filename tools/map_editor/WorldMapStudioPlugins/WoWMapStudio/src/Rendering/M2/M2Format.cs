namespace WorldMapStudio.WoW;

/// <summary>
/// M2: rigid, uniformly-authored geometry (no non-uniform stretching), rendered through
/// <see cref="M2MaterialType"/>. <see cref="CanAuthor"/> is what makes "a procedural model that is an
/// M2" a real thing — it renders with M2's own material vocabulary and places with M2's own
/// constraints, without an actual <c>.m2</c> file involved.
/// </summary>
[Subsystem(nameof(ModelFormatSystem))]
public sealed class M2Format : IModelFormat
{
    public const string FormatId = "wow.format.m2";

    public M2Format(ModelFormatSystem system)
    {
    }

    public string Id => FormatId;

    public string DisplayName => "M2";

    public string Extension => ".m2";

    public string MaterialTypeId => M2Material.TypeId;

    public SelfScale SelfScale => SelfScale.Uniform;

    public bool CanAuthor => true;
}
