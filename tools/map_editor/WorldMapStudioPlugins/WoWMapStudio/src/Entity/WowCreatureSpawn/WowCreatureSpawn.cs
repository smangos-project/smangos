namespace WorldMapStudio.WoW;

/// <summary>A `creature` row as a streamed scene entity. Position and orientation live in
/// <see cref="SceneEntity.Transform"/>; columns not exposed here round-trip untouched.</summary>
public sealed class WowCreatureSpawn : SceneEntity
{
    /// <summary>The `creature.guid` primary key; null until the spawn is first committed.</summary>
    public uint? Guid { get; set; }

    /// <summary>`creature.id`, the template this spawn instances.</summary>
    [ScriptProperty(Mutable = true)]
    public uint TemplateEntry { get; set; }

    [ScriptProperty(Mutable = true)]
    public uint SpawnTimeMin { get; set; } = 120;

    [ScriptProperty(Mutable = true)]
    public uint SpawnTimeMax { get; set; } = 120;

    [ScriptProperty(Mutable = true)]
    public float WanderDistance { get; set; }

    [ScriptProperty(Mutable = true)]
    public byte MovementType { get; set; }

    public override string DisplayName => $"Creature {TemplateEntry}";
}
