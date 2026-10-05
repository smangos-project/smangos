using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_faction` as a browsable catalog. Id or name search; `ParentFactionID` links back into
/// this same catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class FactionFactory : ThinDbcFactory<Faction, DbcFactionRecord>
{
    private const uint NameMaxLength = 100;

    public FactionFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Faction";

    protected override string DisplayText(DbcFactionRecord record) => record.NameLang;

    protected override Expression<Func<DbcFactionRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.NameLang, $"%{term}%");

    protected override Faction ToEntity(DbcFactionRecord record) => new()
    {
        ID = record.ID,
        Name = record.NameLang,
        ReputationIndex = record.ReputationIndex,
        ParentFactionID = record.ParentFactionID,
        Expansion = record.Expansion,
        IsSaved = true,
    };

    protected override void Write(Faction entity, DbcFactionRecord record)
    {
        record.NameLang = entity.Name;
        record.ReputationIndex = entity.ReputationIndex;
        record.ParentFactionID = entity.ParentFactionID;
        record.Expansion = entity.Expansion;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(Faction e) =>
    [
        F.Text("Name", () => e.Name, v => e.Name = v, NameMaxLength),
        F.Int("Reputation Index", () => e.ReputationIndex, v => e.ReputationIndex = v),
        F.Link("Parent Faction", () => e.ParentFactionID, v => e.ParentFactionID = v, "Faction"),
        F.Int("Expansion", () => e.Expansion, v => e.Expansion = v),
    ];
}
