using System;
using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>
/// One row of a <see cref="ThinDbcFactory{TEntity,TRecord}"/>'s field sheet: a label plus how to draw
/// it against a <see cref="CatalogFieldSheet"/>. Built with <see cref="F"/> so a table's <c>Fields</c>
/// override reads as a flat declaration; the base walks the list.
/// </summary>
public sealed class CatalogFieldSpec
{
    private readonly Action<CatalogFieldSheet> _draw;

    internal CatalogFieldSpec(string? label, Action<CatalogFieldSheet> draw)
    {
        Label = label;
        _draw = draw;
    }

    /// <summary>The field label, or null for a structural entry (separator, group wrapper).</summary>
    internal string? Label { get; }

    internal void Draw(CatalogFieldSheet sheet) => _draw(sheet);
}

/// <summary>Field-spec constructors — one per <see cref="CatalogFieldSheet"/> widget.</summary>
public static class F
{
    public static CatalogFieldSpec Int(string label, Func<int> get, Action<int> set) =>
        new(label, sheet => sheet.Int(label, get(), set));

    public static CatalogFieldSpec UInt(string label, Func<uint> get, Action<uint> set) =>
        new(label, sheet => sheet.UInt(label, get(), set));

    public static CatalogFieldSpec Float(string label, Func<float> get, Action<float> set) =>
        new(label, sheet => sheet.Float(label, get(), set));

    public static CatalogFieldSpec Byte(string label, Func<byte> get, Action<byte> set) =>
        new(label, sheet => sheet.Byte(label, get(), set));

    public static CatalogFieldSpec Text(string label, Func<string> get, Action<string> set, uint maxLength = 255) =>
        new(label, sheet => sheet.Text(label, get(), maxLength, set));

    public static CatalogFieldSpec TextMultiline(string label, Func<string> get, Action<string> set, uint maxLength = 4096) =>
        new(label, sheet => sheet.TextMultiline(label, get(), maxLength, set));

    public static CatalogFieldSpec Link(string label, Func<int> get, Action<int> set, string targetCatalogName) =>
        new(label, sheet => sheet.Link(label, get(), set, targetCatalogName));

    /// <summary>A link whose "no reference" reading isn't the plain <c>== 0</c> default — e.g. a
    /// `dbc_spellvisualkit` effect slot, where both <c>0</c> and <c>-1</c> mean none.</summary>
    public static CatalogFieldSpec Link(string label, Func<int> get, Action<int> set, string targetCatalogName,
        Func<int, bool> isEmpty) =>
        new(label, sheet => sheet.Link(label, get(), set, targetCatalogName, isEmpty));

    public static CatalogFieldSpec Enum<TEnum>(string label, Func<int> get, Action<int> set)
        where TEnum : struct, System.Enum =>
        new(label, sheet => sheet.Enum(label, get(), set, typeof(TEnum)));

    public static CatalogFieldSpec Flags<TEnum>(string label, Func<int> get, Action<int> set)
        where TEnum : struct, System.Enum =>
        new(label, sheet => sheet.Flags(label, get(), set, typeof(TEnum)));

    /// <summary>A bitmask over options read at draw time — e.g. race or class bits whose names live in
    /// a catalog — rather than an enum type.</summary>
    public static CatalogFieldSpec Flags(string label, Func<int> get, Action<int> set,
        Func<IReadOnlyList<(string Label, int Bit)>> options) =>
        new(label, sheet => sheet.Flags(label, get(), set, options()));

    public static CatalogFieldSpec Custom(string label, Action body) =>
        new(label, sheet => sheet.Custom(label, body));

    public static CatalogFieldSpec Separator() =>
        new(null, sheet => sheet.Separator());

    public static CatalogFieldSpec Group(string title, params CatalogFieldSpec[] children) =>
        new(title, sheet => sheet.Group(title, () =>
        {
            foreach (CatalogFieldSpec child in children)
            {
                child.Draw(sheet);
            }
        }));
}
