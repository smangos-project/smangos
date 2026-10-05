using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellduration` as a browsable catalog — a plain <see cref="ThinDbcFactory{TEntity,TRecord}"/>.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SpellDurationFactory : ThinDbcFactory<SpellDuration, DbcSpelldurationRecord>
{
    public SpellDurationFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Spell Duration";

    protected override string DisplayText(DbcSpelldurationRecord record) =>
        record.Duration == record.MaxDuration
            ? $"{record.Duration} ms"
            : $"{record.Duration}–{record.MaxDuration} ms";

    protected override SpellDuration ToEntity(DbcSpelldurationRecord record) => new()
    {
        ID = record.ID,
        Duration = record.Duration,
        DurationPerLevel = record.DurationPerLevel,
        MaxDuration = record.MaxDuration,
        IsSaved = true,
    };

    protected override void Write(SpellDuration entity, DbcSpelldurationRecord record)
    {
        record.Duration = entity.Duration;
        record.DurationPerLevel = entity.DurationPerLevel;
        record.MaxDuration = entity.MaxDuration;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(SpellDuration e) =>
    [
        F.Int("Duration (ms)", () => e.Duration, v => e.Duration = v),
        F.Int("Duration Per Level (ms)", () => e.DurationPerLevel, v => e.DurationPerLevel = v),
        F.Int("Max Duration (ms)", () => e.MaxDuration, v => e.MaxDuration = v),
    ];
}
