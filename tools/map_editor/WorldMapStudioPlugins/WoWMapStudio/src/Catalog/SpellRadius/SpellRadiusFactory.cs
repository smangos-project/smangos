using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellradius` as a browsable catalog — a plain <see cref="ThinDbcFactory{TEntity,TRecord}"/>
/// with a computed "8 yd" search label.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SpellRadiusFactory : ThinDbcFactory<SpellRadius, DbcSpellradiusRecord>
{
    public SpellRadiusFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Spell Radius";

    protected override string DisplayText(DbcSpellradiusRecord record) => $"{record.Radius:0.#} yd";

    protected override SpellRadius ToEntity(DbcSpellradiusRecord record) => new()
    {
        ID = record.ID,
        Radius = record.Radius,
        RadiusPerLevel = record.RadiusPerLevel,
        RadiusMax = record.RadiusMax,
        IsSaved = true,
    };

    protected override void Write(SpellRadius entity, DbcSpellradiusRecord record)
    {
        record.Radius = entity.Radius;
        record.RadiusPerLevel = entity.RadiusPerLevel;
        record.RadiusMax = entity.RadiusMax;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(SpellRadius e) =>
    [
        F.Float("Radius (yd)", () => e.Radius, v => e.Radius = v),
        F.Float("Radius Per Level (yd)", () => e.RadiusPerLevel, v => e.RadiusPerLevel = v),
        F.Float("Radius Max (yd)", () => e.RadiusMax, v => e.RadiusMax = v),
    ];
}
