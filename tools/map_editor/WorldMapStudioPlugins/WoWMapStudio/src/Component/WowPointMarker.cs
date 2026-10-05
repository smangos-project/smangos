using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// A small pickable marker at a placed-point entity's origin, so it can be clicked in the viewport —
/// the role <c>WowLightPlaceholder</c> plays for a light and <c>CreaturePlaceholder</c> plays for a
/// creature spawn, but as a real always-attached component (like <c>WowZoneLightBoundary</c>) rather
/// than a conditional fallback, since a point entity like this never gains any other representation to
/// fall back from. Parameterised by <paramref name="typeId"/>/<paramref name="displayName"/> so one
/// class serves every kind that is nothing but a coloured box at a point — a taxi node, a graveyard, a
/// game tele, a spell target position — rather than one near-identical marker class per kind.
///
/// Implements <see cref="ITransformPolicy"/> as a fixed yaw-only, unscaled policy: a graveyard, game
/// tele and spell target position each store a single facing column and no scale at all, so every kind
/// this component serves wants the same restriction — a taxi node simply never edits the rotation it
/// permits.
/// </summary>
public sealed class WowPointMarker(string typeId, string displayName, Color color, float size)
    : SceneComponent, ISceneBoundsProvider, ISceneNodeComponent, ITransformPolicy, IMeshPickable
{
    // Centred on the origin: a point entity's origin is its centre, not its foot.
    private readonly Aabb _bounds = new(Vector3.One * (-size * 0.5f), Vector3.One * size);

    public override string TypeId => typeId;

    public override string DisplayName => displayName;

    public Aabb LocalBounds => _bounds;

    public SelfRotation SelfRotation => SelfRotation.HeightOnly;

    public SelfScale SelfScale => SelfScale.None;

    public override SceneComponent Clone() => new WowPointMarker(typeId, displayName, color, size);

    public Node3D? BuildNode() => new MeshInstance3D
    {
        Name = "Placeholder",
        Mesh = new BoxMesh { Size = Vector3.One * size },
        MaterialOverride = new StandardMaterial3D
        {
            AlbedoColor = color,
            ShadingMode = BaseMaterial3D.ShadingModeEnum.Unshaded,
        },
    };
}
