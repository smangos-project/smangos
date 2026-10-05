using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Recovers a placement's editor <see cref="Transform3D"/> from the raw MDDF/MODF fields. Position and
/// rotation invert through <see cref="WowPlacementTransform"/>; scale is <c>scale / 1024</c>. The one
/// subtlety is the origin: the exporter writes <c>transform · (−modelBoundsCentre)</c>
/// as the stored position, because <c>ModelRendererComponent</c> re-centres a model on its bounding
/// box — so recovering the transform's own origin needs the referenced model's local-bounds centre.
/// </summary>
public static class PlacementMapper
{
    public static Transform3D InvertTransform(
        Vector3 onDiskPosition,
        Vector3 onDiskRotationDegrees,
        ushort onDiskScale,
        Vector3 modelBoundsCentre)
    {
        float scale = onDiskScale / 1024.0f;
        Basis basis = WowPlacementTransform.FromOnDiskRotationDegrees(onDiskRotationDegrees)
            * Basis.FromScale(new Vector3(scale, scale, scale));

        Vector3 modelOrigin = WowPlacementTransform.FromOnDiskPosition(onDiskPosition);
        return new Transform3D(basis, modelOrigin + (basis * modelBoundsCentre));
    }
}
