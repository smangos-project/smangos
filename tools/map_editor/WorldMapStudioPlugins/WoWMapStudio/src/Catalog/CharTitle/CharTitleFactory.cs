using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_chartitles` as a browsable catalog. Id or name search.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class CharTitleFactory : ThinDbcFactory<CharTitle, DbcChartitlesRecord>
{
    private const uint NameMaxLength = 100;

    public CharTitleFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Char Title";

    protected override string DisplayText(DbcChartitlesRecord record) => record.NameLang;

    protected override Expression<Func<DbcChartitlesRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.NameLang, $"%{term}%");

    protected override CharTitle ToEntity(DbcChartitlesRecord record) => new()
    {
        ID = record.ID,
        Name = record.NameLang,
        Name1 = record.Name1Lang,
        IsSaved = true,
    };

    protected override void Write(CharTitle entity, DbcChartitlesRecord record)
    {
        record.NameLang = entity.Name;
        record.Name1Lang = entity.Name1;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(CharTitle e) =>
    [
        F.Text("Name (male)", () => e.Name, v => e.Name = v, NameMaxLength),
        F.Text("Name (female)", () => e.Name1, v => e.Name1 = v, NameMaxLength),
    ];
}
