using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>The `creature_movement` rows of one creature spawn as a streamed scene entity. The nodes live
/// in an always-attached <see cref="WowWaypointPathComponent"/>, edited with the shared path tool.</summary>
public sealed class WowWaypointPath : SceneEntity
{
    /// <summary>`creature_movement.id`, the `creature.guid` the path belongs to. Null while unassigned.</summary>
    public uint? CreatureGuid { get; set; }

    /// <summary>Whether `creature_movement` rows already exist for <see cref="CreatureGuid"/>.</summary>
    public bool IsSaved { get; set; }

    public override string DisplayName => $"Waypoint Path {CreatureGuid}";

    public override Aabb LocalBounds =>
        Components.OfType<ISceneBoundsProvider>().Any() ? EffectiveLocalBounds : new Aabb(Vector3.One * -0.5f, Vector3.One);
}
