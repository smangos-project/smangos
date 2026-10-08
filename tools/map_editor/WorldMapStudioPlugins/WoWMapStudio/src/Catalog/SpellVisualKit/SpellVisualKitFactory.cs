using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellvisualkit` as a browsable catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SpellVisualKitFactory : ThinDbcFactory<SpellVisualKit, DbcSpellvisualkitRecord>
{
    public SpellVisualKitFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Spell Visual Kit";

    protected override string DisplayText(DbcSpellvisualkitRecord record) => $"Spell Visual Kit {record.ID}";

    protected override SpellVisualKit ToEntity(DbcSpellvisualkitRecord record) => new()
    {
        ID = record.ID,
        FallbackSpellVisualKitID = record.FallbackSpellVisualKitID,
        DelayMin = record.DelayMin,
        DelayMax = record.DelayMax,
        FallbackPriority = record.FallbackPriority,
        Flags1 = record.Flags1,
        Flags2 = record.Flags2,
        IsSaved = true,
    };

    protected override void Write(SpellVisualKit entity, DbcSpellvisualkitRecord record)
    {
        record.FallbackSpellVisualKitID = entity.FallbackSpellVisualKitID;
        record.DelayMin = entity.DelayMin;
        record.DelayMax = entity.DelayMax;
        record.FallbackPriority = entity.FallbackPriority;
        record.Flags1 = entity.Flags1;
        record.Flags2 = entity.Flags2;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(SpellVisualKit e) =>
    [
        F.Link("Fallback Spell Visual Kit ID", () => e.FallbackSpellVisualKitID, v => e.FallbackSpellVisualKitID = v, "Spell Visual Kit"),
        F.Int("Delay Min", () => e.DelayMin, v => e.DelayMin = v),
        F.Int("Delay Max", () => e.DelayMax, v => e.DelayMax = v),
        F.Float("Fallback Priority", () => e.FallbackPriority, v => e.FallbackPriority = v),
        F.Int("Flags1", () => e.Flags1, v => e.Flags1 = v),
        F.Int("Flags2", () => e.Flags2, v => e.Flags2 = v),
    ];
}
