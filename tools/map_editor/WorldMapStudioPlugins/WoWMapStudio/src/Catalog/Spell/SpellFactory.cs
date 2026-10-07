using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// `dbc_spell` plus the difficulty-0 rows the 1.15 client keeps in its satellite tables (see
/// <see cref="Spell"/>). Satellite rows are updated in place so unmodelled columns survive; a missing
/// row is only created once one of its fields is non-zero.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SpellFactory : ThinDbcFactory<Spell, DbcSpellRecord>
{
    private const uint NameMaxLength = 512;
    private const uint TextMaxLength = 8192;

    public SpellFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Spell";

    protected override Spell ToEntity(DbcSpellRecord record) => new()
    {
        ID = record.ID,
        NameSubtext = record.NameSubtextLang,
        Description = record.DescriptionLang,
        AuraDescription = record.AuraDescriptionLang,
        IsSaved = true,
    };

    protected override void Write(Spell entity, DbcSpellRecord record)
    {
        record.NameSubtextLang = entity.NameSubtext;
        record.DescriptionLang = entity.Description;
        record.AuraDescriptionLang = entity.AuraDescription;
    }

    public override async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext db = Storage.CreateContext();
        IQueryable<DbcSpellnameRecord> query = db.DbcSpellname.AsNoTracking();
        if (!string.IsNullOrWhiteSpace(filter))
        {
            query = int.TryParse(filter, out int id)
                ? query.Where(r => r.ID == id)
                : query.Where(r => EF.Functions.Like(r.NameLang, $"%{filter}%"));
        }

        List<DbcSpellnameRecord> rows = await query.OrderBy(r => r.ID).Take(SearchLimit).ToListAsync().ConfigureAwait(false);
        return rows.Select(r => new CatalogSearchResult(r.ID.ToString(), r.NameLang)).ToList();
    }

    protected override async Task LoadExtrasAsync(WorldDbContext db, Spell s)
    {
        int id = s.ID;
        s.Name = (await db.DbcSpellname.AsNoTracking().FirstOrDefaultAsync(r => r.ID == id).ConfigureAwait(false))?.NameLang ?? string.Empty;

        if (await db.DbcSpellmisc.AsNoTracking().FirstOrDefaultAsync(r => r.SpellID == id && r.DifficultyID == 0).ConfigureAwait(false) is { } m)
        {
            ReadMisc(m, s);
        }

        if (await db.DbcSpellcooldowns.AsNoTracking().FirstOrDefaultAsync(r => r.SpellID == id && r.DifficultyID == 0).ConfigureAwait(false) is { } c)
        {
            ReadCooldowns(c, s);
        }

        if (await db.DbcSpellcategories.AsNoTracking().FirstOrDefaultAsync(r => r.SpellID == id && r.DifficultyID == 0).ConfigureAwait(false) is { } k)
        {
            ReadCategories(k, s);
        }

        if (await db.DbcSpelllevels.AsNoTracking().FirstOrDefaultAsync(r => r.SpellID == id && r.DifficultyID == 0).ConfigureAwait(false) is { } l)
        {
            ReadLevels(l, s);
        }

        if (await db.DbcSpellpower.AsNoTracking().FirstOrDefaultAsync(r => r.SpellID == id && r.OrderIndex == 0).ConfigureAwait(false) is { } p)
        {
            ReadPower(p, s);
        }

        List<DbcSpelleffectRecord> effects = await db.DbcSpelleffect.AsNoTracking()
            .Where(r => r.SpellID == id && r.DifficultyID == 0).ToListAsync().ConfigureAwait(false);
        s.ReplaceEffects(effects.Select(ToEffectEntry));
    }

    protected override void StageExtras(WorldDbContext db, Spell s)
    {
        int id = s.ID;

        DbcSpellnameRecord? name = db.DbcSpellname.Find(id);
        if (name is null)
        {
            db.DbcSpellname.Add(new DbcSpellnameRecord { ID = id, NameLang = s.Name });
        }
        else
        {
            name.NameLang = s.Name;
        }

        Upsert(db.DbcSpellmisc, r => r.ID, r => r.SpellID == id && r.DifficultyID == 0,
            r => { r.SpellID = id; r.DifficultyID = 0; }, r => WriteMisc(s, r),
            s.Attributes.Any(a => a != 0) || s.SchoolMask != 0 || s.CastingTimeIndex != 0 || s.DurationIndex != 0
            || s.RangeIndex != 0 || s.Speed != 0 || s.SpellIconFileDataID != 0 || s.ActiveIconFileDataID != 0);

        Upsert(db.DbcSpellcooldowns, r => r.ID, r => r.SpellID == id && r.DifficultyID == 0,
            r => { r.SpellID = id; r.DifficultyID = 0; }, r => WriteCooldowns(s, r),
            s.RecoveryTime != 0 || s.CategoryRecoveryTime != 0 || s.StartRecoveryTime != 0);

        Upsert(db.DbcSpellcategories, r => r.ID, r => r.SpellID == id && r.DifficultyID == 0,
            r => { r.SpellID = id; r.DifficultyID = 0; }, r => WriteCategories(s, r),
            s.Category != 0 || s.DispelType != 0 || s.Mechanic != 0 || s.StartRecoveryCategory != 0);

        Upsert(db.DbcSpelllevels, r => r.ID, r => r.SpellID == id && r.DifficultyID == 0,
            r => { r.SpellID = id; r.DifficultyID = 0; }, r => WriteLevels(s, r),
            s.BaseLevel != 0 || s.MaxLevel != 0 || s.SpellLevel != 0);

        Upsert(db.DbcSpellpower, r => r.ID, r => r.SpellID == id && r.OrderIndex == 0,
            r => { r.SpellID = id; r.OrderIndex = 0; }, r => WritePower(s, r),
            s.PowerType != 0 || s.ManaCost != 0 || s.PowerCostPct != 0);

        List<DbcSpelleffectRecord> existing = db.DbcSpelleffect.Where(r => r.SpellID == id && r.DifficultyID == 0).ToList();
        foreach (DbcSpelleffectRecord row in existing.Where(r => s.Effects.All(e => e.EffectIndex != r.EffectIndex)))
        {
            db.DbcSpelleffect.Remove(row);
        }

        foreach (SpellEffectEntry entry in s.Effects)
        {
            DbcSpelleffectRecord? row = existing.FirstOrDefault(r => r.EffectIndex == entry.EffectIndex);
            if (row is null)
            {
                row = new DbcSpelleffectRecord { ID = NextId(db.DbcSpelleffect, r => r.ID), SpellID = id, DifficultyID = 0, EffectIndex = entry.EffectIndex };
                db.DbcSpelleffect.Add(row);
            }

            WriteEffect(entry, row);
        }
    }

    protected override void StageDeleteExtras(WorldDbContext db, Spell s)
    {
        int id = s.ID;
        db.DbcSpellname.RemoveRange(db.DbcSpellname.Where(r => r.ID == id));
        db.DbcSpellmisc.RemoveRange(db.DbcSpellmisc.Where(r => r.SpellID == id));
        db.DbcSpellcooldowns.RemoveRange(db.DbcSpellcooldowns.Where(r => r.SpellID == id));
        db.DbcSpellcategories.RemoveRange(db.DbcSpellcategories.Where(r => r.SpellID == id));
        db.DbcSpelllevels.RemoveRange(db.DbcSpelllevels.Where(r => r.SpellID == id));
        db.DbcSpellpower.RemoveRange(db.DbcSpellpower.Where(r => r.SpellID == id));
        db.DbcSpelleffect.RemoveRange(db.DbcSpelleffect.Where(r => r.SpellID == id));
    }

    private static int NextId<T>(DbSet<T> set, Expression<Func<T, int>> id) where T : class
    {
        int stored = set.AsNoTracking().OrderByDescending(id).Select(id).FirstOrDefault();
        Func<T, int> get = id.Compile();
        int tracked = set.Local.Select(get).DefaultIfEmpty(0).Max();
        return Math.Max(stored, tracked) + 1;
    }

    private static void Upsert<T>(DbSet<T> set, Expression<Func<T, int>> id, Expression<Func<T, bool>> match,
        Action<T> init, Action<T> write, bool hasData) where T : class, new()
    {
        T? row = set.FirstOrDefault(match);
        if (row is null)
        {
            if (!hasData)
            {
                return;
            }

            row = new T();
            init(row);
            ((MemberExpression)id.Body).Member.DeclaringType!.GetProperty(((MemberExpression)id.Body).Member.Name)!
                .SetValue(row, NextId(set, id));
            set.Add(row);
        }

        write(row);
    }

    private static void ReadMisc(DbcSpellmiscRecord m, Spell s)
    {
        int[] a =
        [
            m.Attributes1, m.Attributes2, m.Attributes3, m.Attributes4, m.Attributes5, m.Attributes6, m.Attributes7, m.Attributes8,
            m.Attributes9, m.Attributes10, m.Attributes11, m.Attributes12, m.Attributes13, m.Attributes14, m.Attributes15, m.Attributes16,
        ];
        a.CopyTo(s.Attributes, 0);
        s.SchoolMask = m.SchoolMask;
        s.CastingTimeIndex = m.CastingTimeIndex;
        s.DurationIndex = m.DurationIndex;
        s.RangeIndex = m.RangeIndex;
        s.Speed = m.Speed;
        s.SpellIconFileDataID = m.SpellIconFileDataID;
        s.ActiveIconFileDataID = m.ActiveIconFileDataID;
    }

    private static void WriteMisc(Spell s, DbcSpellmiscRecord m)
    {
        int[] a = s.Attributes;
        m.Attributes1 = a[0]; m.Attributes2 = a[1]; m.Attributes3 = a[2]; m.Attributes4 = a[3];
        m.Attributes5 = a[4]; m.Attributes6 = a[5]; m.Attributes7 = a[6]; m.Attributes8 = a[7];
        m.Attributes9 = a[8]; m.Attributes10 = a[9]; m.Attributes11 = a[10]; m.Attributes12 = a[11];
        m.Attributes13 = a[12]; m.Attributes14 = a[13]; m.Attributes15 = a[14]; m.Attributes16 = a[15];
        m.SchoolMask = s.SchoolMask;
        m.CastingTimeIndex = s.CastingTimeIndex;
        m.DurationIndex = s.DurationIndex;
        m.RangeIndex = s.RangeIndex;
        m.Speed = s.Speed;
        m.SpellIconFileDataID = s.SpellIconFileDataID;
        m.ActiveIconFileDataID = s.ActiveIconFileDataID;
    }

    private static void ReadCooldowns(DbcSpellcooldownsRecord c, Spell s)
    {
        s.RecoveryTime = c.RecoveryTime;
        s.CategoryRecoveryTime = c.CategoryRecoveryTime;
        s.StartRecoveryTime = c.StartRecoveryTime;
    }

    private static void WriteCooldowns(Spell s, DbcSpellcooldownsRecord c)
    {
        c.RecoveryTime = s.RecoveryTime;
        c.CategoryRecoveryTime = s.CategoryRecoveryTime;
        c.StartRecoveryTime = s.StartRecoveryTime;
    }

    private static void ReadCategories(DbcSpellcategoriesRecord k, Spell s)
    {
        s.Category = k.Category;
        s.DispelType = k.DispelType;
        s.Mechanic = k.Mechanic;
        s.StartRecoveryCategory = k.StartRecoveryCategory;
    }

    private static void WriteCategories(Spell s, DbcSpellcategoriesRecord k)
    {
        k.Category = s.Category;
        k.DispelType = s.DispelType;
        k.Mechanic = s.Mechanic;
        k.StartRecoveryCategory = s.StartRecoveryCategory;
    }

    private static void ReadLevels(DbcSpelllevelsRecord l, Spell s)
    {
        s.BaseLevel = l.BaseLevel;
        s.MaxLevel = l.MaxLevel;
        s.SpellLevel = l.SpellLevel;
    }

    private static void WriteLevels(Spell s, DbcSpelllevelsRecord l)
    {
        l.BaseLevel = s.BaseLevel;
        l.MaxLevel = s.MaxLevel;
        l.SpellLevel = s.SpellLevel;
    }

    private static void ReadPower(DbcSpellpowerRecord p, Spell s)
    {
        s.PowerType = p.PowerType;
        s.ManaCost = p.ManaCost;
        s.PowerCostPct = p.PowerCostPct;
    }

    private static void WritePower(Spell s, DbcSpellpowerRecord p)
    {
        p.PowerType = s.PowerType;
        p.ManaCost = s.ManaCost;
        p.PowerCostPct = s.PowerCostPct;
    }

    internal static SpellEffectEntry ToEffectEntry(DbcSpelleffectRecord r) => new()
    {
        EffectIndex = r.EffectIndex,
        Effect = r.Effect,
        EffectAura = r.EffectAura,
        EffectBasePoints = r.EffectBasePoints,
        EffectDieSides = r.EffectDieSides,
        EffectRealPointsPerLevel = r.EffectRealPointsPerLevel,
        EffectBonusCoefficient = r.EffectBonusCoefficient,
        EffectAmplitude = r.EffectAmplitude,
        EffectAuraPeriod = r.EffectAuraPeriod,
        ImplicitTarget1 = r.ImplicitTarget1,
        ImplicitTarget2 = r.ImplicitTarget2,
        EffectMechanic = r.EffectMechanic,
        EffectRadiusIndex1 = r.EffectRadiusIndex1,
        EffectRadiusIndex2 = r.EffectRadiusIndex2,
        EffectMiscValue1 = r.EffectMiscValue1,
        EffectMiscValue2 = r.EffectMiscValue2,
        EffectItemType = r.EffectItemType,
        EffectTriggerSpell = r.EffectTriggerSpell,
        EffectChainTargets = r.EffectChainTargets,
        EffectChainAmplitude = r.EffectChainAmplitude,
    };

    internal static void WriteEffect(SpellEffectEntry e, DbcSpelleffectRecord r)
    {
        r.EffectIndex = e.EffectIndex;
        r.Effect = e.Effect;
        r.EffectAura = e.EffectAura;
        r.EffectBasePoints = e.EffectBasePoints;
        r.EffectDieSides = e.EffectDieSides;
        r.EffectRealPointsPerLevel = e.EffectRealPointsPerLevel;
        r.EffectBonusCoefficient = e.EffectBonusCoefficient;
        r.EffectAmplitude = e.EffectAmplitude;
        r.EffectAuraPeriod = e.EffectAuraPeriod;
        r.ImplicitTarget1 = e.ImplicitTarget1;
        r.ImplicitTarget2 = e.ImplicitTarget2;
        r.EffectMechanic = e.EffectMechanic;
        r.EffectRadiusIndex1 = e.EffectRadiusIndex1;
        r.EffectRadiusIndex2 = e.EffectRadiusIndex2;
        r.EffectMiscValue1 = e.EffectMiscValue1;
        r.EffectMiscValue2 = e.EffectMiscValue2;
        r.EffectItemType = e.EffectItemType;
        r.EffectTriggerSpell = e.EffectTriggerSpell;
        r.EffectChainTargets = e.EffectChainTargets;
        r.EffectChainAmplitude = e.EffectChainAmplitude;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(Spell e)
    {
        yield return F.Text("Name", () => e.Name, v => e.Name = v, NameMaxLength);
        yield return F.Text("Name Subtext", () => e.NameSubtext, v => e.NameSubtext = v, NameMaxLength);
        yield return F.TextMultiline("Description", () => e.Description, v => e.Description = v, TextMaxLength);
        yield return F.TextMultiline("Aura Description", () => e.AuraDescription, v => e.AuraDescription = v, TextMaxLength);
        yield return F.Separator();
        yield return F.Flags<SpellSchoolMaskFlags>("School Mask", () => e.SchoolMask, v => e.SchoolMask = v);
        yield return F.Link("Casting Time", () => e.CastingTimeIndex, v => e.CastingTimeIndex = v, "Spell Cast Times");
        yield return F.Link("Duration", () => e.DurationIndex, v => e.DurationIndex = v, "Spell Duration");
        yield return F.Link("Range", () => e.RangeIndex, v => e.RangeIndex = v, "Spell Range");
        yield return F.Float("Speed", () => e.Speed, v => e.Speed = v);
        yield return F.Int("Spell Icon File ID", () => e.SpellIconFileDataID, v => e.SpellIconFileDataID = v);
        yield return F.Int("Active Icon File ID", () => e.ActiveIconFileDataID, v => e.ActiveIconFileDataID = v);
        yield return F.Separator();
        yield return F.Enum<SpellPowerKind>("Power Type", () => e.PowerType, v => e.PowerType = v);
        yield return F.Int("Mana Cost", () => e.ManaCost, v => e.ManaCost = v);
        yield return F.Float("Power Cost %", () => e.PowerCostPct, v => e.PowerCostPct = v);
        yield return F.Int("Recovery Time (ms)", () => e.RecoveryTime, v => e.RecoveryTime = v);
        yield return F.Int("Category Recovery Time (ms)", () => e.CategoryRecoveryTime, v => e.CategoryRecoveryTime = v);
        yield return F.Int("Start Recovery Time (ms)", () => e.StartRecoveryTime, v => e.StartRecoveryTime = v);
        yield return F.Int("Category", () => e.Category, v => e.Category = v);
        yield return F.Enum<SpellDispelKind>("Dispel Type", () => e.DispelType, v => e.DispelType = v);
        yield return F.Enum<SpellMechanicType>("Mechanic", () => e.Mechanic, v => e.Mechanic = v);
        yield return F.Int("Start Recovery Category", () => e.StartRecoveryCategory, v => e.StartRecoveryCategory = v);
        yield return F.Int("Base Level", () => e.BaseLevel, v => e.BaseLevel = v);
        yield return F.Int("Max Level", () => e.MaxLevel, v => e.MaxLevel = v);
        yield return F.Int("Spell Level", () => e.SpellLevel, v => e.SpellLevel = v);
        yield return F.Separator();
        yield return F.Flags<SpellAttr0Flags>("Attributes 1", () => e.Attributes[0], v => e.Attributes[0] = v);
        yield return F.Flags<SpellAttr1Flags>("Attributes 2", () => e.Attributes[1], v => e.Attributes[1] = v);
        yield return F.Flags<SpellAttr2Flags>("Attributes 3", () => e.Attributes[2], v => e.Attributes[2] = v);
        yield return F.Flags<SpellAttr3Flags>("Attributes 4", () => e.Attributes[3], v => e.Attributes[3] = v);
        yield return F.Flags<SpellAttr4Flags>("Attributes 5", () => e.Attributes[4], v => e.Attributes[4] = v);
        yield return F.Flags<SpellAttr5Flags>("Attributes 6", () => e.Attributes[5], v => e.Attributes[5] = v);
        yield return F.Flags<SpellAttr6Flags>("Attributes 7", () => e.Attributes[6], v => e.Attributes[6] = v);
        yield return F.Flags<SpellAttr7Flags>("Attributes 8", () => e.Attributes[7], v => e.Attributes[7] = v);
        yield return F.Flags<SpellAttr8Flags>("Attributes 9", () => e.Attributes[8], v => e.Attributes[8] = v);
        yield return F.Flags<SpellAttr9Flags>("Attributes 10", () => e.Attributes[9], v => e.Attributes[9] = v);
        yield return F.Flags<SpellAttr10Flags>("Attributes 11", () => e.Attributes[10], v => e.Attributes[10] = v);
        for (int i = 11; i < Spell.AttributeCount; i++)
        {
            int slot = i;
            yield return F.Int($"Attributes {slot + 1}", () => e.Attributes[slot], v => e.Attributes[slot] = v);
        }
    }

    public override void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        base.DrawFields(context, entity, tracker, navigate, fieldFilter);
        var s = (Spell)entity;
        ImGui.Separator();
        ImGui.Text("Effects");
        foreach (SpellEffectEntry effect in s.Effects.ToList())
        {
            int index = effect.EffectIndex;
            ImGui.PushID(index);
            ImGui.Text($"Effect {index}");
            ImGui.SameLine();
            if (ImGui.SmallButton("Remove"))
            {
                Apply(context, s.RemoveEffect(index));
            }

            EnumInt<SpellEffectType>(context, s, index, "Effect", effect.Effect, nameof(SpellEffectEntry.Effect));
            EnumInt<SpellAuraType>(context, s, index, "Aura", effect.EffectAura, nameof(SpellEffectEntry.EffectAura));
            Int(context, s, index, "Base Points", effect.EffectBasePoints, nameof(SpellEffectEntry.EffectBasePoints));
            Int(context, s, index, "Die Sides", effect.EffectDieSides, nameof(SpellEffectEntry.EffectDieSides));
            Float(context, s, index, "Points Per Level", effect.EffectRealPointsPerLevel, nameof(SpellEffectEntry.EffectRealPointsPerLevel));
            Float(context, s, index, "Bonus Coefficient", effect.EffectBonusCoefficient, nameof(SpellEffectEntry.EffectBonusCoefficient));
            Int(context, s, index, "Aura Period (ms)", effect.EffectAuraPeriod, nameof(SpellEffectEntry.EffectAuraPeriod));
            EnumInt<SpellImplicitTarget>(context, s, index, "Target 1", effect.ImplicitTarget1, nameof(SpellEffectEntry.ImplicitTarget1));
            EnumInt<SpellImplicitTarget>(context, s, index, "Target 2", effect.ImplicitTarget2, nameof(SpellEffectEntry.ImplicitTarget2));
            EnumInt<SpellMechanicType>(context, s, index, "Mechanic", effect.EffectMechanic, nameof(SpellEffectEntry.EffectMechanic));
            Int(context, s, index, "Radius 1", effect.EffectRadiusIndex1, nameof(SpellEffectEntry.EffectRadiusIndex1));
            Int(context, s, index, "Radius 2", effect.EffectRadiusIndex2, nameof(SpellEffectEntry.EffectRadiusIndex2));
            Int(context, s, index, "Misc Value 1", (int)effect.EffectMiscValue1, nameof(SpellEffectEntry.EffectMiscValue1));
            Int(context, s, index, "Misc Value 2", (int)effect.EffectMiscValue2, nameof(SpellEffectEntry.EffectMiscValue2));
            Int(context, s, index, "Item Type", effect.EffectItemType, nameof(SpellEffectEntry.EffectItemType));
            Int(context, s, index, "Trigger Spell", effect.EffectTriggerSpell, nameof(SpellEffectEntry.EffectTriggerSpell));
            Int(context, s, index, "Chain Targets", effect.EffectChainTargets, nameof(SpellEffectEntry.EffectChainTargets));
            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Effect"))
        {
            Apply(context, s.AddEffect(s.Effects.Count == 0 ? 0 : s.Effects.Max(x => x.EffectIndex) + 1));
        }
    }

    private static void Int(EditorContext context, Spell s, int index, string label, int current, string field)
    {
        int value = current;
        ImGui.SetNextItemWidth(140.0f);
        if (ImGui.InputInt(label, ref value) && value != current)
        {
            Apply(context, s.SetEffectField(index, field, value));
        }
    }

    private static void EnumInt<TEnum>(EditorContext context, Spell s, int index, string label, int current, string field)
        where TEnum : struct, System.Enum
    {
        if (CatalogEnumField.DrawEnumPicker(label, current, typeof(TEnum), out int picked) && picked != current)
        {
            Apply(context, s.SetEffectField(index, field, picked));
        }
    }

    private static void Float(EditorContext context, Spell s, int index, string label, float current, string field)
    {
        float value = current;
        ImGui.SetNextItemWidth(140.0f);
        if (ImGui.DragFloat(label, ref value, 0.01f) && !value.Equals(current))
        {
            Apply(context, s.SetEffectField(index, field, value));
        }
    }

    private static void Apply(EditorContext context, IEditCommand command)
    {
        command.Apply();
        context.EditSessions.Record(command);
    }
}
