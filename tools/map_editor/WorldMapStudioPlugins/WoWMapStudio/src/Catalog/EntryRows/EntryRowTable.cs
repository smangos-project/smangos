using System;
using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>Base for an entity holding all rows of one `entry`, edited as a whole list so undo restores
/// the list.</summary>
public abstract class EntryRowTable<TRow> : CatalogEntity, IEntryRowTable<TRow>
{
    private readonly List<TRow> _rows = [];

    protected EntryRowTable(string kind)
    {
        Kind = kind;
    }

    public string Kind { get; }

    [ScriptProperty]
    public int ID { get; set; }

    public bool IsSaved { get; set; }

    public IReadOnlyList<TRow> Rows => _rows;

    public override string DisplayName => $"{Kind} {ID}";

    public void ReplaceRows(IEnumerable<TRow> rows)
    {
        _rows.Clear();
        _rows.AddRange(rows);
    }

    protected void CheckIndex(int index)
    {
        if (index < 0 || index >= _rows.Count)
        {
            throw new InvalidOperationException($"Row {index} does not exist on {DisplayName}.");
        }
    }

    protected IEditCommand AppendRow(TRow row) => ReplaceRowsCommand(_rows.Append(row).ToList());

    protected IEditCommand RemoveRowAt(int index)
    {
        CheckIndex(index);
        List<TRow> after = _rows.ToList();
        after.RemoveAt(index);
        return ReplaceRowsCommand(after);
    }

    protected IEditCommand UpdateRow(int index, Func<TRow, TRow> change)
    {
        CheckIndex(index);
        List<TRow> after = _rows.ToList();
        after[index] = change(after[index]);
        return ReplaceRowsCommand(after);
    }

    private IEditCommand ReplaceRowsCommand(List<TRow> after) =>
        new SetFieldCommand<List<TRow>>(this, "rows", ReplaceRows, _rows.ToList(), after);
}
