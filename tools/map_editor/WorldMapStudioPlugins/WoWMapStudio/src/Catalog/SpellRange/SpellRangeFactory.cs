using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>`dbc_spellrange` as a browsable catalog. Id search with a computed "0–40 yd" label
/// (its own `DisplayName_lang` when set).</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class SpellRangeFactory : ThinDbcFactory<SpellRange, DbcSpellrangeRecord>
{
    private const uint NameMaxLength = 128;

    public SpellRangeFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Spell Range";

    protected override string DisplayText(DbcSpellrangeRecord record) =>
        record.DisplayNameLang.Length > 0
            ? record.DisplayNameLang
            : $"{record.RangeMin1:0.#}–{record.RangeMax1:0.#} yd";

    protected override SpellRange ToEntity(DbcSpellrangeRecord record) => new()
    {
        ID = record.ID,
        RangeMin1 = record.RangeMin1,
        RangeMin2 = record.RangeMin2,
        RangeMax1 = record.RangeMax1,
        RangeMax2 = record.RangeMax2,
        Flags = record.Flags,
        DisplayNameLang = record.DisplayNameLang,
        DisplayNameShortLang = record.DisplayNameShortLang,
        IsSaved = true,
    };

    protected override void Write(SpellRange entity, DbcSpellrangeRecord record)
    {
        record.RangeMin1 = entity.RangeMin1;
        record.RangeMin2 = entity.RangeMin2;
        record.RangeMax1 = entity.RangeMax1;
        record.RangeMax2 = entity.RangeMax2;
        record.Flags = entity.Flags;
        record.DisplayNameLang = entity.DisplayNameLang;
        record.DisplayNameShortLang = entity.DisplayNameShortLang;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(SpellRange e) =>
    [
        F.Text("Display Name", () => e.DisplayNameLang, v => e.DisplayNameLang = v, NameMaxLength),
        F.Text("Display Name Short", () => e.DisplayNameShortLang, v => e.DisplayNameShortLang = v, NameMaxLength),
        F.Flags<SpellRangeFlags>("Flags", () => e.Flags, v => e.Flags = v),
        F.Separator(),
        F.Float("Range Min (hostile)", () => e.RangeMin1, v => e.RangeMin1 = v),
        F.Float("Range Min (friendly)", () => e.RangeMin2, v => e.RangeMin2 = v),
        F.Float("Range Max (hostile)", () => e.RangeMax1, v => e.RangeMax1 = v),
        F.Float("Range Max (friendly)", () => e.RangeMax2, v => e.RangeMax2 = v),
    ];
}
