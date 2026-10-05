using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_questsort` as a browsable catalog. Id or name search.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class QuestSortFactory : ThinDbcFactory<QuestSort, DbcQuestsortRecord>
{
    private const uint NameMaxLength = 100;

    public QuestSortFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Quest Sort";

    protected override string DisplayText(DbcQuestsortRecord record) => record.SortNameLang;

    protected override Expression<Func<DbcQuestsortRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.SortNameLang, $"%{term}%");

    protected override QuestSort ToEntity(DbcQuestsortRecord record) => new()
    {
        ID = record.ID,
        Name = record.SortNameLang,
        IsSaved = true,
    };

    protected override void Write(QuestSort entity, DbcQuestsortRecord record)
    {
        record.SortNameLang = entity.Name;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(QuestSort e) =>
    [
        F.Text("Name", () => e.Name, v => e.Name = v, NameMaxLength),
    ];
}
