namespace WorldMapStudio.WoW;

/// <summary>
/// WMO: placed level geometry — only yaw about the world's vertical axis, no scaling — rendered
/// through <see cref="WmoMaterialType"/>. <see cref="CanAuthor"/> is what makes "a procedural model
/// that is a WMO" a real thing — it renders with WMO's own material vocabulary and places with WMO's
/// own constraints, without an actual <c>.wmo</c> file involved.
/// </summary>
[Subsystem(nameof(ModelFormatSystem))]
public sealed class WmoFormat : IModelFormat
{
    public const string FormatId = "wow.format.wmo";

    public WmoFormat(ModelFormatSystem system)
    {
    }

    public string Id => FormatId;

    public string DisplayName => "WMO";

    public string Extension => ".wmo";

    public string MaterialTypeId => WmoMaterial.TypeId;

    public SelfRotation SelfRotation => SelfRotation.HeightOnly;

    public SelfScale SelfScale => SelfScale.None;

    public bool CanAuthor => true;
}
