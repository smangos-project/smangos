using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>A catalog entity that edits a copy of its database record directly, so wide tables need no
/// mirrored property list.</summary>
public sealed class RecordEntity<TRecord> : CatalogEntity, IDbcCatalogEntity
    where TRecord : class, new()
{
    public string Kind { get; set; } = typeof(TRecord).Name;

    [ScriptProperty]
    public int ID { get; set; }

    public TRecord Record { get; } = new();

    public bool IsSaved { get; set; }

    public override string DisplayName => $"{Kind} {ID}";

    /// <summary>Sets a numeric column by name.</summary>
    [ScriptFunction]
    public void SetNumber(string field, double value)
    {
        PropertyInfo prop = typeof(TRecord).GetProperty(field) ?? throw new InvalidOperationException($"{Kind} has no field '{field}'.");
        prop.SetValue(Record, Convert.ChangeType(value, prop.PropertyType));
    }
}

/// <summary>Base for catalogs over a record with many columns: the fields are the record's own
/// properties, listed by prefix (arrays such as ItemID1..N) or name.</summary>
public abstract class RecordBackedFactory<TRecord> : ThinDbcFactory<RecordEntity<TRecord>, TRecord>
    where TRecord : class, new()
{
    private static readonly PropertyInfo[] Props = typeof(TRecord).GetProperties()
        .Where(p => p.CanRead && p.CanWrite).ToArray();

    protected RecordBackedFactory(WorldStorage storage) : base(storage)
    {
    }

    private PropertyInfo IdProp => typeof(TRecord).GetProperty(KeyPropertyName)!;

    protected override RecordEntity<TRecord> ToEntity(TRecord record)
    {
        var entity = new RecordEntity<TRecord>
        {
            Kind = CatalogName,
            ID = (int)IdProp.GetValue(record)!,
            IsSaved = true,
        };
        Copy(record, entity.Record);
        return entity;
    }

    protected override void Write(RecordEntity<TRecord> entity, TRecord record)
    {
        Copy(entity.Record, record);
        IdProp.SetValue(record, entity.ID);
    }

    private static void Copy(TRecord from, TRecord to)
    {
        foreach (PropertyInfo p in Props)
        {
            p.SetValue(to, p.GetValue(from));
        }
    }

    internal RecordEntity<TRecord> CallToEntity(TRecord record) => ToEntity(record);

    internal void CallWrite(RecordEntity<TRecord> entity, TRecord record) => Write(entity, record);

    private static PropertyInfo Column(string name) =>
        typeof(TRecord).GetProperty(name) ?? throw new InvalidOperationException($"{typeof(TRecord).Name} has no column '{name}'.");

    protected static CatalogFieldSpec Int(RecordEntity<TRecord> e, string label, string column)
    {
        PropertyInfo p = Column(column);
        return F.Int(label, () => (int)p.GetValue(e.Record)!, v => p.SetValue(e.Record, v));
    }

    protected static CatalogFieldSpec Float(RecordEntity<TRecord> e, string label, string column)
    {
        PropertyInfo p = Column(column);
        return F.Float(label, () => (float)p.GetValue(e.Record)!, v => p.SetValue(e.Record, v));
    }

    protected static CatalogFieldSpec Text(RecordEntity<TRecord> e, string label, string column, uint max = 255)
    {
        PropertyInfo p = Column(column);
        return F.Text(label, () => (string)p.GetValue(e.Record)!, v => p.SetValue(e.Record, v), max);
    }

    protected static CatalogFieldSpec Link(RecordEntity<TRecord> e, string label, string column, string catalog)
    {
        PropertyInfo p = Column(column);
        return F.Link(label, () => (int)p.GetValue(e.Record)!, v => p.SetValue(e.Record, v), catalog);
    }

    /// <summary>One field per column <c>{prefix}1..{prefix}{count}</c>.</summary>
    protected static IEnumerable<CatalogFieldSpec> Series(RecordEntity<TRecord> e, string label, string prefix, int count,
        string? linkCatalog = null) =>
        Enumerable.Range(1, count).Select(i => linkCatalog is null
            ? Int(e, $"{label} {i}", prefix + i)
            : Link(e, $"{label} {i}", prefix + i, linkCatalog));

    protected static Expression<Func<TRecord, bool>> NameLike(string column, string term)
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        MethodInfo like = typeof(DbFunctionsExtensions).GetMethod(nameof(DbFunctionsExtensions.Like),
            [typeof(DbFunctions), typeof(string), typeof(string)])!;
        return Expression.Lambda<Func<TRecord, bool>>(
            Expression.Call(like, Expression.Constant(EF.Functions), Expression.Property(p, column), Expression.Constant($"%{term}%")), p);
    }
}

[Subsystem(nameof(WorldStorage))]
public sealed class ItemSetFactory : RecordBackedFactory<DbcItemsetRecord>
{
    public ItemSetFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Item Set";

    protected override string DisplayText(DbcItemsetRecord record) => record.NameLang;

    protected override Expression<Func<DbcItemsetRecord, bool>> TextFilter(string term) => NameLike(nameof(DbcItemsetRecord.NameLang), term);

    protected override IEnumerable<CatalogFieldSpec> Fields(RecordEntity<DbcItemsetRecord> e) =>
        new[]
        {
            Text(e, "Name", nameof(DbcItemsetRecord.NameLang)),
            Int(e, "Flags", nameof(DbcItemsetRecord.SetFlags)),
            Link(e, "Required Skill", nameof(DbcItemsetRecord.RequiredSkill), "Skill Line"),
            Int(e, "Required Skill Rank", nameof(DbcItemsetRecord.RequiredSkillRank)),
        }.Concat(Series(e, "Item", "ItemID", 17, "Item"));
}

[Subsystem(nameof(WorldStorage))]
public sealed class LockFactory : RecordBackedFactory<DbcLockRecord>
{
    public LockFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Lock";

    protected override IEnumerable<CatalogFieldSpec> Fields(RecordEntity<DbcLockRecord> e) =>
        Series(e, "Type", "Type", 8)
            .Concat(Series(e, "Index", "Index", 8))
            .Concat(Series(e, "Skill", "Skill", 8))
            .Concat(Series(e, "Action", "Action", 8));
}

[Subsystem(nameof(WorldStorage))]
public sealed class SpellItemEnchantmentFactory : RecordBackedFactory<DbcSpellitemenchantmentRecord>
{
    public SpellItemEnchantmentFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Enchantment";

    protected override string DisplayText(DbcSpellitemenchantmentRecord record) => record.NameLang;

    protected override Expression<Func<DbcSpellitemenchantmentRecord, bool>> TextFilter(string term) =>
        NameLike(nameof(DbcSpellitemenchantmentRecord.NameLang), term);

    protected override IEnumerable<CatalogFieldSpec> Fields(RecordEntity<DbcSpellitemenchantmentRecord> e) =>
        new[]
        {
            Text(e, "Name", nameof(DbcSpellitemenchantmentRecord.NameLang)),
            Int(e, "Charges", nameof(DbcSpellitemenchantmentRecord.Charges)),
            Int(e, "Flags", nameof(DbcSpellitemenchantmentRecord.Flags)),
            Link(e, "Gem Item", nameof(DbcSpellitemenchantmentRecord.GemItemID), "Item"),
            Int(e, "Required Skill", nameof(DbcSpellitemenchantmentRecord.RequiredSkillID)),
            Int(e, "Required Skill Rank", nameof(DbcSpellitemenchantmentRecord.RequiredSkillRank)),
            Int(e, "Min Level", nameof(DbcSpellitemenchantmentRecord.MinLevel)),
            Int(e, "Max Level", nameof(DbcSpellitemenchantmentRecord.MaxLevel)),
            Int(e, "Item Visual", nameof(DbcSpellitemenchantmentRecord.ItemVisual)),
        }
        .Concat(Series(e, "Effect", "Effect", 3))
        .Concat(Series(e, "Points Min", "EffectPointsMin", 3))
        .Concat(Series(e, "Points Max", "EffectPointsMax", 3))
        .Concat(Series(e, "Arg", "EffectArg", 3));
}

[Subsystem(nameof(WorldStorage))]
public sealed class TalentFactory : RecordBackedFactory<DbcTalentRecord>
{
    public TalentFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Talent";

    protected override string DisplayText(DbcTalentRecord record) => record.DescriptionLang;

    protected override IEnumerable<CatalogFieldSpec> Fields(RecordEntity<DbcTalentRecord> e) =>
        new[]
        {
            Int(e, "Tab", nameof(DbcTalentRecord.TabID)),
            Int(e, "Tier", nameof(DbcTalentRecord.TierID)),
            Int(e, "Column", nameof(DbcTalentRecord.ColumnIndex)),
            Int(e, "Class", nameof(DbcTalentRecord.ClassID)),
            Int(e, "Flags", nameof(DbcTalentRecord.Flags)),
            Link(e, "Required Spell", nameof(DbcTalentRecord.RequiredSpellID), "Spell"),
        }
        .Concat(Series(e, "Rank Spell", "SpellRank", 9, "Spell"))
        .Concat(Series(e, "Prerequisite Talent", "PrereqTalent", 3, "Talent"))
        .Concat(Series(e, "Prerequisite Rank", "PrereqRank", 3));
}

[Subsystem(nameof(WorldStorage))]
public sealed class HolidayFactory : RecordBackedFactory<DbcHolidaysRecord>
{
    public HolidayFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Holiday";

    protected override IEnumerable<CatalogFieldSpec> Fields(RecordEntity<DbcHolidaysRecord> e) =>
        new[]
        {
            Int(e, "Region", nameof(DbcHolidaysRecord.Region)),
            Int(e, "Looping", nameof(DbcHolidaysRecord.Looping)),
            Int(e, "Name Id", nameof(DbcHolidaysRecord.HolidayNameID)),
            Int(e, "Description Id", nameof(DbcHolidaysRecord.HolidayDescriptionID)),
            Int(e, "Priority", nameof(DbcHolidaysRecord.Priority)),
            Int(e, "Calendar Filter", nameof(DbcHolidaysRecord.CalendarFilterType)),
            Int(e, "Flags", nameof(DbcHolidaysRecord.Flags)),
        }
        .Concat(Series(e, "Duration", "Duration", 10))
        .Concat(Series(e, "Date", "Date", 16))
        .Concat(Series(e, "Calendar Flags", "CalendarFlags", 10))
        .Concat(Series(e, "Texture", "TextureFileDataID", 3));
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureSpellsFactory : RecordBackedFactory<CreatureSpellsRecord>
{
    private static readonly (string Label, string Column)[] Columns =
    [
        ("Spell", "SpellId"), ("Probability", "Probability"), ("Cast Target", "CastTarget"),
        ("Target Param 1", "TargetParam1"), ("Target Param 2", "TargetParam2"), ("Cast Flags", "CastFlags"),
        ("Initial Delay Min", "DelayInitialMin"), ("Initial Delay Max", "DelayInitialMax"),
        ("Repeat Delay Min", "DelayRepeatMin"), ("Repeat Delay Max", "DelayRepeatMax"), ("Script", "ScriptId"),
    ];

    public CreatureSpellsFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Spells";

    protected override string KeyPropertyName => "Entry";

    protected override IEnumerable<CatalogFieldSpec> Fields(RecordEntity<CreatureSpellsRecord> e) =>
        Enumerable.Range(1, 8).SelectMany(slot => Columns.Select(c =>
            c.Column == "SpellId"
                ? Link(e, $"Slot {slot} {c.Label}", c.Column + slot, "Spell")
                : Int(e, $"Slot {slot} {c.Label}", c.Column + slot)));
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureAiEventFactory : RecordBackedFactory<CreatureAiEventsRecord>
{
    public CreatureAiEventFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature AI Event";

    protected override string KeyPropertyName => "Id";

    protected override IEnumerable<CatalogFieldSpec> Fields(RecordEntity<CreatureAiEventsRecord> e) =>
    [
        Link(e, "Creature", nameof(CreatureAiEventsRecord.CreatureId), "Creature Template"),
        Int(e, "Condition", nameof(CreatureAiEventsRecord.ConditionId)),
        Int(e, "Event Type", nameof(CreatureAiEventsRecord.EventType)),
        Int(e, "Inverse Phase Mask", nameof(CreatureAiEventsRecord.EventInversePhaseMask)),
        Int(e, "Chance", nameof(CreatureAiEventsRecord.EventChance)),
        Int(e, "Flags", nameof(CreatureAiEventsRecord.EventFlags)),
        Int(e, "Param 1", nameof(CreatureAiEventsRecord.EventParam1)),
        Int(e, "Param 2", nameof(CreatureAiEventsRecord.EventParam2)),
        Int(e, "Param 3", nameof(CreatureAiEventsRecord.EventParam3)),
        Int(e, "Param 4", nameof(CreatureAiEventsRecord.EventParam4)),
        Int(e, "Action 1 Script", nameof(CreatureAiEventsRecord.Action1Script)),
        Int(e, "Action 2 Script", nameof(CreatureAiEventsRecord.Action2Script)),
        Int(e, "Action 3 Script", nameof(CreatureAiEventsRecord.Action3Script)),
    ];
}
