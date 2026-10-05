using Godot;

namespace WorldMapStudio.WoW;

/// <summary>The translucent trigger volume of a <see cref="WowAreaTrigger"/>: a sphere when
/// <paramref name="radius"/> is positive, otherwise a box of server length (X), width (Y) and height (Z)
/// turned by <paramref name="yaw"/> about the up axis.</summary>
public sealed class WowAreaTriggerVolume(float radius, Vector3 boxSize, float yaw)
    : SceneComponent, ISceneBoundsProvider, ISceneNodeComponent
{
    public const string Kind = "wow-area-trigger-volume";

    private static readonly Color VolumeColor = new(0.9f, 0.5f, 1.0f, 0.25f);

    public override string TypeId => Kind;

    public override string DisplayName => "Area Trigger Volume";

    public override int ContentVersion => System.HashCode.Combine(radius, boxSize, yaw);

    public Aabb LocalBounds
    {
        get
        {
            Vector3 half = radius > 0.0f ? Vector3.One * radius : (Basis.FromEuler(new Vector3(0.0f, yaw, 0.0f)) * boxSize).Abs() * 0.5f;
            return new Aabb(-half, half * 2.0f);
        }
    }

    public override SceneComponent Clone() => new WowAreaTriggerVolume(radius, boxSize, yaw);

    public Node3D? BuildNode()
    {
        Mesh mesh = radius > 0.0f
            ? new SphereMesh { Radius = radius, Height = radius * 2.0f }
            : new BoxMesh { Size = boxSize };
        return new MeshInstance3D
        {
            Name = "Volume",
            Mesh = mesh,
            Rotation = radius > 0.0f ? Vector3.Zero : new Vector3(0.0f, yaw, 0.0f),
            MaterialOverride = new StandardMaterial3D
            {
                AlbedoColor = VolumeColor,
                Transparency = BaseMaterial3D.TransparencyEnum.Alpha,
                ShadingMode = BaseMaterial3D.ShadingModeEnum.Unshaded,
                CullMode = BaseMaterial3D.CullModeEnum.Disabled,
            },
        };
    }

    /// <summary>Builds the volume for a trigger's current shape fields.</summary>
    public static WowAreaTriggerVolume For(WowAreaTrigger trigger) =>
        new(trigger.Radius, new Vector3(trigger.BoxLength, trigger.BoxHeight, trigger.BoxWidth), trigger.BoxYaw);
}
