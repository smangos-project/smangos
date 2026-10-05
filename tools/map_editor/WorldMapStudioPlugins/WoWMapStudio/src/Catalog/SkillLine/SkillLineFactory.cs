using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_skillline` as a browsable catalog. Id or name search.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SkillLineFactory : ThinDbcFactory<SkillLine, DbcSkilllineRecord>
{
    private const uint NameMaxLength = 100;

    public SkillLineFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Skill Line";

    protected override string DisplayText(DbcSkilllineRecord record) => record.DisplayNameLang;

    protected override Expression<Func<DbcSkilllineRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.DisplayNameLang, $"%{term}%");

    protected override SkillLine ToEntity(DbcSkilllineRecord record) => new()
    {
        ID = record.ID,
        Name = record.DisplayNameLang,
        CategoryID = record.CategoryID,
        SpellIconFileID = record.SpellIconFileID,
        IsSaved = true,
    };

    protected override void Write(SkillLine entity, DbcSkilllineRecord record)
    {
        record.DisplayNameLang = entity.Name;
        record.CategoryID = entity.CategoryID;
        record.SpellIconFileID = entity.SpellIconFileID;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(SkillLine e) =>
    [
        F.Text("Name", () => e.Name, v => e.Name = v, NameMaxLength),
        F.Int("Category ID", () => e.CategoryID, v => e.CategoryID = v),
        F.Int("Spell Icon File ID", () => e.SpellIconFileID, v => e.SpellIconFileID = v),
    ];
}
