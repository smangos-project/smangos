using System;

namespace WorldMapStudio.WoW;

/// <summary>One row of `gossip_menu`: a text shown for a menu under a condition.</summary>
public readonly record struct GossipTextRow
{
    public int TextId { get; init; }
    public int ScriptId { get; init; }
    public int ConditionId { get; init; }
}

/// <summary>One row of `gossip_menu_option`.</summary>
public readonly record struct GossipOptionRow
{
    public int Id { get; init; }
    public int OptionIcon { get; init; }
    public string OptionText { get; init; }
    public int OptionBroadcastText { get; init; }
    public int OptionId { get; init; }
    public int NpcOptionNpcflag { get; init; }
    public int ActionMenuId { get; init; }
    public int ActionPoiId { get; init; }
    public int ActionScriptId { get; init; }
    public int BoxCoded { get; init; }
    public string BoxText { get; init; }
    public int BoxBroadcastText { get; init; }
    public int ConditionId { get; init; }
}

/// <summary>The texts a gossip menu can show.</summary>
public sealed class GossipTextTable : EntryRowTable<GossipTextRow>
{
    public GossipTextTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand AddRow(int textId) => AppendRow(new GossipTextRow { TextId = textId });

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row => field switch
    {
        nameof(GossipTextRow.TextId) => row with { TextId = (int)value },
        nameof(GossipTextRow.ScriptId) => row with { ScriptId = (int)value },
        nameof(GossipTextRow.ConditionId) => row with { ConditionId = (int)value },
        _ => throw new InvalidOperationException($"Unknown gossip text field '{field}'."),
    });
}

/// <summary>The options of one gossip menu.</summary>
public sealed class GossipOptionTable : EntryRowTable<GossipOptionRow>
{
    public GossipOptionTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand AddRow(int id) => AppendRow(new GossipOptionRow { Id = id, OptionText = string.Empty, BoxText = string.Empty });

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row => field switch
    {
        nameof(GossipOptionRow.Id) => row with { Id = (int)value },
        nameof(GossipOptionRow.OptionIcon) => row with { OptionIcon = (int)value },
        nameof(GossipOptionRow.OptionBroadcastText) => row with { OptionBroadcastText = (int)value },
        nameof(GossipOptionRow.OptionId) => row with { OptionId = (int)value },
        nameof(GossipOptionRow.NpcOptionNpcflag) => row with { NpcOptionNpcflag = (int)value },
        nameof(GossipOptionRow.ActionMenuId) => row with { ActionMenuId = (int)value },
        nameof(GossipOptionRow.ActionPoiId) => row with { ActionPoiId = (int)value },
        nameof(GossipOptionRow.ActionScriptId) => row with { ActionScriptId = (int)value },
        nameof(GossipOptionRow.BoxCoded) => row with { BoxCoded = (int)value },
        nameof(GossipOptionRow.BoxBroadcastText) => row with { BoxBroadcastText = (int)value },
        nameof(GossipOptionRow.ConditionId) => row with { ConditionId = (int)value },
        _ => throw new InvalidOperationException($"Unknown gossip option field '{field}'."),
    });

    /// <summary>Sets the option or box text of a row.</summary>
    [ScriptFunction]
    public IEditCommand SetRowText(int index, string field, string value) => UpdateRow(index, row => field switch
    {
        nameof(GossipOptionRow.OptionText) => row with { OptionText = value },
        nameof(GossipOptionRow.BoxText) => row with { BoxText = value },
        _ => throw new InvalidOperationException($"Unknown gossip option text field '{field}'."),
    });
}
