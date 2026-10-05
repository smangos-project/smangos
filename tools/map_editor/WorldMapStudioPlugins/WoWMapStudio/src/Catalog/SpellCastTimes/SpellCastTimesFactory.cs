using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellcasttimes` as a browsable catalog — a plain <see cref="ThinDbcFactory{TEntity,TRecord}"/>
/// with a computed "1500 ms" search label.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SpellCastTimesFactory : ThinDbcFactory<SpellCastTimes, DbcSpellcasttimesRecord>
{
    public SpellCastTimesFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Spell Cast Times";

    protected override string DisplayText(DbcSpellcasttimesRecord record) => $"{record.Base} ms";

    protected override SpellCastTimes ToEntity(DbcSpellcasttimesRecord record) => new()
    {
        ID = record.ID,
        Base = record.Base,
        PerLevel = record.PerLevel,
        Minimum = record.Minimum,
        IsSaved = true,
    };

    protected override void Write(SpellCastTimes entity, DbcSpellcasttimesRecord record)
    {
        record.Base = entity.Base;
        record.PerLevel = entity.PerLevel;
        record.Minimum = entity.Minimum;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(SpellCastTimes e) =>
    [
        F.Int("Base (ms)", () => e.Base, v => e.Base = v),
        F.Int("Per Level (ms)", () => e.PerLevel, v => e.PerLevel = v),
        F.Int("Minimum (ms)", () => e.Minimum, v => e.Minimum = v),
    ];
}
