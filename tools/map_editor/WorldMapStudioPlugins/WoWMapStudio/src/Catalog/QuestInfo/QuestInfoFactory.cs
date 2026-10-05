using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_questinfo` as a browsable catalog. Id or name search.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class QuestInfoFactory : ThinDbcFactory<QuestInfo, DbcQuestinfoRecord>
{
    private const uint NameMaxLength = 100;

    public QuestInfoFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Quest Info";

    protected override string DisplayText(DbcQuestinfoRecord record) => record.InfoNameLang;

    protected override Expression<Func<DbcQuestinfoRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.InfoNameLang, $"%{term}%");

    protected override QuestInfo ToEntity(DbcQuestinfoRecord record) => new()
    {
        ID = record.ID,
        Name = record.InfoNameLang,
        IsSaved = true,
    };

    protected override void Write(QuestInfo entity, DbcQuestinfoRecord record)
    {
        record.InfoNameLang = entity.Name;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(QuestInfo e) =>
    [
        F.Text("Name", () => e.Name, v => e.Name = v, NameMaxLength),
    ];
}
