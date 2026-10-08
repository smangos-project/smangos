using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`creature_template` as a browsable catalog: id or name search, the common fields editable in place.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class CreatureTemplateFactory : ThinDbcFactory<CreatureTemplate, CreatureTemplateRecord>
{
    public CreatureTemplateFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Template";

    protected override string KeyPropertyName => "Entry";

    protected override string DisplayText(CreatureTemplateRecord record) => record.Name;

    protected override Expression<Func<CreatureTemplateRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.Name, $"%{term}%");

    protected override CreatureTemplate ToEntity(CreatureTemplateRecord record) => new()
    {
        ID = record.Entry,
        Name = record.Name,
        Subname = record.Subname ?? string.Empty,
        LevelMin = record.LevelMin,
        LevelMax = record.LevelMax,
        Faction = record.Faction,
        NpcFlags = record.NpcFlags,
        DisplayId1 = record.DisplayId1,
        DisplayScale1 = record.DisplayScale1,
        Type = record.Type,
        Rank = record.Rank,
        LootId = record.LootId,
        VendorId = record.VendorId,
        TrainerId = record.TrainerId,
        GossipMenuId = record.GossipMenuId,
        AiName = record.AiName,
        ScriptName = record.ScriptName,
        IsSaved = true,
    };

    protected override void Write(CreatureTemplate entity, CreatureTemplateRecord record)
    {
        record.Entry = entity.ID;
        record.Name = entity.Name;
        record.Subname = entity.Subname.Length == 0 ? null : entity.Subname;
        record.LevelMin = entity.LevelMin;
        record.LevelMax = entity.LevelMax;
        record.Faction = entity.Faction;
        record.NpcFlags = entity.NpcFlags;
        record.DisplayId1 = entity.DisplayId1;
        record.DisplayScale1 = entity.DisplayScale1;
        record.Type = entity.Type;
        record.Rank = entity.Rank;
        record.LootId = entity.LootId;
        record.VendorId = entity.VendorId;
        record.TrainerId = entity.TrainerId;
        record.GossipMenuId = entity.GossipMenuId;
        record.AiName = entity.AiName;
        record.ScriptName = entity.ScriptName;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(CreatureTemplate e) =>
    [
        F.Text("Name", () => e.Name, v => e.Name = v, 100),
        F.Text("Subname", () => e.Subname, v => e.Subname = v, 100),
        F.Int("Level Min", () => e.LevelMin, v => e.LevelMin = v),
        F.Int("Level Max", () => e.LevelMax, v => e.LevelMax = v),
        F.Int("Faction", () => e.Faction, v => e.Faction = v),
        F.Int("NPC Flags", () => e.NpcFlags, v => e.NpcFlags = v),
        F.Int("Display Id", () => e.DisplayId1, v => e.DisplayId1 = v),
        F.Float("Display Scale", () => e.DisplayScale1, v => e.DisplayScale1 = v),
        F.Int("Type", () => e.Type, v => e.Type = v),
        F.Int("Rank", () => e.Rank, v => e.Rank = v),
        F.Link("Loot Id", () => e.LootId, v => e.LootId = v, "Creature Loot"),
        F.Int("Vendor Id", () => e.VendorId, v => e.VendorId = v),
        F.Int("Trainer Id", () => e.TrainerId, v => e.TrainerId = v),
        F.Int("Gossip Menu Id", () => e.GossipMenuId, v => e.GossipMenuId = v),
        F.Text("AI Name", () => e.AiName, v => e.AiName = v, 64),
        F.Text("Script Name", () => e.ScriptName, v => e.ScriptName = v, 64),
    ];

    internal CreatureTemplate CallToEntity(CreatureTemplateRecord record) => ToEntity(record);

    internal void CallWrite(CreatureTemplate entity, CreatureTemplateRecord record) => Write(entity, record);
}
