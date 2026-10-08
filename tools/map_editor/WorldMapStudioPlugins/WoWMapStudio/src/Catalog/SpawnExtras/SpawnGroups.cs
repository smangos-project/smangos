using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>A `game_event` row. `ID` is the event entry.</summary>
public sealed class GameEvent : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string StartTime { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string EndTime { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int Occurence { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Length { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Holiday { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Description { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int Hardcoded { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Disabled { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Game Event {ID}: {Description}";
}

/// <summary>A `creature_groups` row. `ID` is the member spawn guid.</summary>
public sealed class CreatureGroupMember : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int LeaderGuid { get; set; }

    [ScriptProperty(Mutable = true)]
    public float Dist { get; set; }

    [ScriptProperty(Mutable = true)]
    public float Angle { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Group Member {ID} (leader {LeaderGuid})";
}

/// <summary>A `creature_linking` row. `ID` is the linked spawn guid.</summary>
public sealed class CreatureLink : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MasterGuid { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flag { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Link {ID} (master {MasterGuid})";
}

[Subsystem(nameof(WorldStorage))]
public sealed class GameEventFactory : ThinDbcFactory<GameEvent, GameEventRecord>
{
    public GameEventFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Game Event";

    protected override string KeyPropertyName => "Entry";

    protected override string DisplayText(GameEventRecord record) => record.Description ?? string.Empty;

    protected override Expression<Func<GameEventRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.Description, $"%{term}%");

    protected override GameEvent ToEntity(GameEventRecord r) => new()
    {
        ID = r.Entry,
        StartTime = r.StartTime,
        EndTime = r.EndTime,
        Occurence = r.Occurence,
        Length = r.Length,
        Holiday = r.Holiday,
        Description = r.Description ?? string.Empty,
        Hardcoded = r.Hardcoded,
        Disabled = r.Disabled,
        IsSaved = true,
    };

    protected override void Write(GameEvent e, GameEventRecord r)
    {
        r.Entry = e.ID;
        r.StartTime = e.StartTime;
        r.EndTime = e.EndTime;
        r.Occurence = e.Occurence;
        r.Length = e.Length;
        r.Holiday = e.Holiday;
        r.Description = e.Description.Length == 0 ? null : e.Description;
        r.Hardcoded = e.Hardcoded;
        r.Disabled = e.Disabled;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(GameEvent e) =>
    [
        F.Text("Description", () => e.Description, v => e.Description = v, 255),
        F.Text("Start Time", () => e.StartTime, v => e.StartTime = v, 32),
        F.Text("End Time", () => e.EndTime, v => e.EndTime = v, 32),
        F.Int("Occurence (min)", () => e.Occurence, v => e.Occurence = v),
        F.Int("Length (min)", () => e.Length, v => e.Length = v),
        F.Int("Holiday", () => e.Holiday, v => e.Holiday = v),
        F.Int("Hardcoded", () => e.Hardcoded, v => e.Hardcoded = v),
        F.Int("Disabled", () => e.Disabled, v => e.Disabled = v),
    ];

    internal GameEvent CallToEntity(GameEventRecord record) => ToEntity(record);

    internal void CallWrite(GameEvent entity, GameEventRecord record) => Write(entity, record);
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureGroupMemberFactory : ThinDbcFactory<CreatureGroupMember, CreatureGroupsRecord>
{
    public CreatureGroupMemberFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Group Member";

    protected override string KeyPropertyName => "MemberGuid";

    protected override CreatureGroupMember ToEntity(CreatureGroupsRecord r) => new()
    {
        ID = r.MemberGuid,
        LeaderGuid = r.LeaderGuid,
        Dist = r.Dist,
        Angle = r.Angle,
        Flags = r.Flags,
        IsSaved = true,
    };

    protected override void Write(CreatureGroupMember e, CreatureGroupsRecord r)
    {
        r.MemberGuid = e.ID;
        r.LeaderGuid = e.LeaderGuid;
        r.Dist = e.Dist;
        r.Angle = e.Angle;
        r.Flags = e.Flags;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(CreatureGroupMember e) =>
    [
        F.Int("Leader Guid", () => e.LeaderGuid, v => e.LeaderGuid = v),
        F.Float("Distance", () => e.Dist, v => e.Dist = v),
        F.Float("Angle", () => e.Angle, v => e.Angle = v),
        F.Int("Flags", () => e.Flags, v => e.Flags = v),
    ];

    internal CreatureGroupMember CallToEntity(CreatureGroupsRecord record) => ToEntity(record);

    internal void CallWrite(CreatureGroupMember entity, CreatureGroupsRecord record) => Write(entity, record);
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureLinkFactory : ThinDbcFactory<CreatureLink, CreatureLinkingRecord>
{
    public CreatureLinkFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Link";

    protected override string KeyPropertyName => "Guid";

    protected override CreatureLink ToEntity(CreatureLinkingRecord r) => new()
    {
        ID = r.Guid,
        MasterGuid = r.MasterGuid,
        Flag = r.Flag,
        IsSaved = true,
    };

    protected override void Write(CreatureLink e, CreatureLinkingRecord r)
    {
        r.Guid = e.ID;
        r.MasterGuid = e.MasterGuid;
        r.Flag = e.Flag;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(CreatureLink e) =>
    [
        F.Int("Master Guid", () => e.MasterGuid, v => e.MasterGuid = v),
        F.Int("Flag", () => e.Flag, v => e.Flag = v),
    ];

    internal CreatureLink CallToEntity(CreatureLinkingRecord record) => ToEntity(record);

    internal void CallWrite(CreatureLink entity, CreatureLinkingRecord record) => Write(entity, record);
}

/// <summary>A `PoolCreature` row. `ID` is the creature spawn guid.</summary>
public sealed class PoolCreatureMember : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int PoolEntry { get; set; }

    [ScriptProperty(Mutable = true)]
    public float Chance { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Pool Creature Member {ID} (pool {PoolEntry})";
}

[Subsystem(nameof(WorldStorage))]
public sealed class PoolCreatureMemberFactory : ThinDbcFactory<PoolCreatureMember, PoolCreatureRecord>
{
    public PoolCreatureMemberFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Pool Creature Member";

    protected override string KeyPropertyName => "Guid";

    protected override PoolCreatureMember ToEntity(PoolCreatureRecord r) => new()
    {
        ID = r.Guid,
        PoolEntry = r.PoolEntry,
        Chance = r.Chance,
        Flags = r.Flags,
        IsSaved = true,
    };

    protected override void Write(PoolCreatureMember e, PoolCreatureRecord r)
    {
        r.Guid = e.ID;
        r.PoolEntry = e.PoolEntry;
        r.Chance = e.Chance;
        r.Flags = e.Flags;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(PoolCreatureMember e) =>
    [
        F.Link("Pool Entry", () => e.PoolEntry, v => e.PoolEntry = v, "Pool"),
        F.Float("Chance", () => e.Chance, v => e.Chance = v),
        F.Int("Flags", () => e.Flags, v => e.Flags = v),
    ];

    internal PoolCreatureMember CallToEntity(PoolCreatureRecord record) => ToEntity(record);

    internal void CallWrite(PoolCreatureMember entity, PoolCreatureRecord record) => Write(entity, record);
}

/// <summary>A `PoolGameobject` row. `ID` is the gameobject spawn guid.</summary>
public sealed class PoolGameObjectMember : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int PoolEntry { get; set; }

    [ScriptProperty(Mutable = true)]
    public float Chance { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Pool GameObject Member {ID} (pool {PoolEntry})";
}

[Subsystem(nameof(WorldStorage))]
public sealed class PoolGameObjectMemberFactory : ThinDbcFactory<PoolGameObjectMember, PoolGameobjectRecord>
{
    public PoolGameObjectMemberFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Pool GameObject Member";

    protected override string KeyPropertyName => "Guid";

    protected override PoolGameObjectMember ToEntity(PoolGameobjectRecord r) => new()
    {
        ID = r.Guid,
        PoolEntry = r.PoolEntry,
        Chance = r.Chance,
        Flags = r.Flags,
        IsSaved = true,
    };

    protected override void Write(PoolGameObjectMember e, PoolGameobjectRecord r)
    {
        r.Guid = e.ID;
        r.PoolEntry = e.PoolEntry;
        r.Chance = e.Chance;
        r.Flags = e.Flags;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(PoolGameObjectMember e) =>
    [
        F.Link("Pool Entry", () => e.PoolEntry, v => e.PoolEntry = v, "Pool"),
        F.Float("Chance", () => e.Chance, v => e.Chance = v),
        F.Int("Flags", () => e.Flags, v => e.Flags = v),
    ];

    internal PoolGameObjectMember CallToEntity(PoolGameobjectRecord record) => ToEntity(record);

    internal void CallWrite(PoolGameObjectMember entity, PoolGameobjectRecord record) => Write(entity, record);
}

/// <summary>A `PoolPool` row. `ID` is the child pool entry.</summary>
public sealed class PoolPoolMember : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MotherPool { get; set; }

    [ScriptProperty(Mutable = true)]
    public float Chance { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Pool Pool Member {ID} (pool {MotherPool})";
}

[Subsystem(nameof(WorldStorage))]
public sealed class PoolPoolMemberFactory : ThinDbcFactory<PoolPoolMember, PoolPoolRecord>
{
    public PoolPoolMemberFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Pool Pool Member";

    protected override string KeyPropertyName => "PoolId";

    protected override PoolPoolMember ToEntity(PoolPoolRecord r) => new()
    {
        ID = r.PoolId,
        MotherPool = r.MotherPool,
        Chance = r.Chance,
        Flags = r.Flags,
        IsSaved = true,
    };

    protected override void Write(PoolPoolMember e, PoolPoolRecord r)
    {
        r.PoolId = e.ID;
        r.MotherPool = e.MotherPool;
        r.Chance = e.Chance;
        r.Flags = e.Flags;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(PoolPoolMember e) =>
    [
        F.Link("Mother Pool", () => e.MotherPool, v => e.MotherPool = v, "Pool"),
        F.Float("Chance", () => e.Chance, v => e.Chance = v),
        F.Int("Flags", () => e.Flags, v => e.Flags = v),
    ];

    internal PoolPoolMember CallToEntity(PoolPoolRecord record) => ToEntity(record);

    internal void CallWrite(PoolPoolMember entity, PoolPoolRecord record) => Write(entity, record);
}

/// <summary>One spawn guid of a `game_event_creature` / `game_event_gameobject` entry.</summary>
public readonly record struct EventSpawnRow
{
    public int Guid { get; init; }
}

[Subsystem(nameof(WorldStorage))]
public sealed class GameEventCreatureFactory : PlayerRowsFactory<GameEventCreatureRecord, EventSpawnRow>
{
    public GameEventCreatureFactory(WorldStorage storage) : base(storage, ["Creature Guid"])
    {
    }

    public override string CatalogName => "Game Event Creatures";

    protected override System.Linq.Expressions.Expression<Func<GameEventCreatureRecord, int>> EntryExpression() => r => r.Event;

    protected override void SetEntry(GameEventCreatureRecord r, int entry) => r.Event = entry;

    protected override EventSpawnRow ToRow(GameEventCreatureRecord r) => new() { Guid = r.Guid };

    protected override void WriteRow(EventSpawnRow row, GameEventCreatureRecord r)
    {
    }

    protected override void WriteKey(EventSpawnRow row, GameEventCreatureRecord r) => r.Guid = row.Guid;

    protected override IEnumerable<EventSpawnRow> Order(IEnumerable<EventSpawnRow> rows) => System.Linq.Enumerable.OrderBy(rows, r => r.Guid);
}

[Subsystem(nameof(WorldStorage))]
public sealed class GameEventGameObjectFactory : PlayerRowsFactory<GameEventGameobjectRecord, EventSpawnRow>
{
    public GameEventGameObjectFactory(WorldStorage storage) : base(storage, ["GameObject Guid"])
    {
    }

    public override string CatalogName => "Game Event GameObjects";

    protected override System.Linq.Expressions.Expression<Func<GameEventGameobjectRecord, int>> EntryExpression() => r => r.Event;

    protected override void SetEntry(GameEventGameobjectRecord r, int entry) => r.Event = entry;

    protected override EventSpawnRow ToRow(GameEventGameobjectRecord r) => new() { Guid = r.Guid };

    protected override void WriteRow(EventSpawnRow row, GameEventGameobjectRecord r)
    {
    }

    protected override void WriteKey(EventSpawnRow row, GameEventGameobjectRecord r) => r.Guid = row.Guid;

    protected override IEnumerable<EventSpawnRow> Order(IEnumerable<EventSpawnRow> rows) => System.Linq.Enumerable.OrderBy(rows, r => r.Guid);
}

/// <summary>One quest of a `game_event_quest` entry.</summary>
public readonly record struct EventQuestRow
{
    public int Quest { get; init; }
}

[Subsystem(nameof(WorldStorage))]
public sealed class GameEventQuestFactory : PlayerRowsFactory<GameEventQuestRecord, EventQuestRow>
{
    public GameEventQuestFactory(WorldStorage storage) : base(storage, ["Quest"])
    {
    }

    public override string CatalogName => "Game Event Quests";

    protected override System.Linq.Expressions.Expression<Func<GameEventQuestRecord, int>> EntryExpression() => r => r.Event;

    protected override void SetEntry(GameEventQuestRecord r, int entry) => r.Event = entry;

    protected override EventQuestRow ToRow(GameEventQuestRecord r) => new() { Quest = r.Quest };

    protected override void WriteRow(EventQuestRow row, GameEventQuestRecord r)
    {
    }

    protected override void WriteKey(EventQuestRow row, GameEventQuestRecord r) => r.Quest = row.Quest;

    protected override IEnumerable<EventQuestRow> Order(IEnumerable<EventQuestRow> rows) => System.Linq.Enumerable.OrderBy(rows, r => r.Quest);
}

/// <summary>Keys the game event spawn tables on (event, guid).</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GameEventSpawnKeys : ITableConfiguration
{
    public GameEventSpawnKeys(WorldStorage storage)
    {
    }

    public void Configure(ModelBuilder model)
    {
        model.Entity<GameEventCreatureRecord>().HasKey(r => new { r.Event, r.Guid });
        model.Entity<GameEventGameobjectRecord>().HasKey(r => new { r.Event, r.Guid });
        model.Entity<GameEventQuestRecord>().HasKey(r => new { r.Event, r.Quest });
    }
}
