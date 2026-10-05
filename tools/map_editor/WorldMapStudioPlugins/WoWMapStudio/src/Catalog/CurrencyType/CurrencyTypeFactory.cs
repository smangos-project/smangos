using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_currencytypes` as a browsable catalog. Id or name search.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class CurrencyTypeFactory : ThinDbcFactory<CurrencyType, DbcCurrencytypesRecord>
{
    private const uint NameMaxLength = 100;

    public CurrencyTypeFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Currency Type";

    protected override string DisplayText(DbcCurrencytypesRecord record) => record.NameLang;

    protected override Expression<Func<DbcCurrencytypesRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.NameLang, $"%{term}%");

    protected override CurrencyType ToEntity(DbcCurrencytypesRecord record) => new()
    {
        ID = record.ID,
        Name = record.NameLang,
        CategoryID = record.CategoryID,
        IsSaved = true,
    };

    protected override void Write(CurrencyType entity, DbcCurrencytypesRecord record)
    {
        record.NameLang = entity.Name;
        record.CategoryID = entity.CategoryID;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(CurrencyType e) =>
    [
        F.Text("Name", () => e.Name, v => e.Name = v, NameMaxLength),
        F.Int("Category ID", () => e.CategoryID, v => e.CategoryID = v),
    ];
}
