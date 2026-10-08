using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;
using Vector2 = System.Numerics.Vector2;

namespace WorldMapStudio.WoW;

/// <summary>
/// A `*_loot_template` table as a catalog keyed by `entry`: the entity is the entry's whole row list,
/// replaced on save. All nine loot tables share their columns, so one base serves them and each
/// subclass only names its table.
/// </summary>
public abstract class LootFactory<TRecord> : ILazyCatalogEntityFactory, ICatalogBrowser
    where TRecord : class, new()
{
    private const int SearchLimit = 50;

    private static readonly PropertyInfo EntryProp = Prop("Entry");
    private static readonly PropertyInfo ItemProp = Prop("Item");
    private static readonly PropertyInfo ChanceProp = Prop("ChanceOrQuestChance");
    private static readonly PropertyInfo GroupProp = Prop("Groupid");
    private static readonly PropertyInfo MinProp = Prop("MincountOrRef");
    private static readonly PropertyInfo MaxProp = Prop("Maxcount");
    private static readonly PropertyInfo ConditionProp = Prop("ConditionId");

    private bool _createOpenRequested;
    private bool _createActive;
    private int _createId;
    private string? _createError;

    protected LootFactory(WorldStorage storage)
    {
        Storage = storage;
    }

    protected WorldStorage Storage { get; }

    public Type EntityType => typeof(LootTable);

    public abstract string CatalogName { get; }

    /// <summary>Short kind shown in the entity's display name, e.g. "Creature".</summary>
    protected abstract string Kind { get; }

    public bool Handles(IEntity entity) => entity is LootTable t && t.Kind == Kind;

    public void Configure(ModelBuilder model)
    {
    }

    private static PropertyInfo Prop(string name) => typeof(TRecord).GetProperty(name)!;

    private static LootRow ToRow(TRecord r) => new()
    {
        Item = (int)ItemProp.GetValue(r)!,
        Chance = (float)ChanceProp.GetValue(r)!,
        GroupId = (int)GroupProp.GetValue(r)!,
        MinCountOrRef = (int)MinProp.GetValue(r)!,
        MaxCount = (int)MaxProp.GetValue(r)!,
        ConditionId = (int)ConditionProp.GetValue(r)!,
    };

    private static void Write(LootRow row, TRecord r)
    {
        ChanceProp.SetValue(r, row.Chance);
        GroupProp.SetValue(r, row.GroupId);
        MinProp.SetValue(r, row.MinCountOrRef);
        MaxProp.SetValue(r, row.MaxCount);
        ConditionProp.SetValue(r, row.ConditionId);
    }

    internal LootTable CallToEntity(int entry, IEnumerable<TRecord> records)
    {
        var table = new LootTable(Kind) { ID = entry, IsSaved = true };
        table.ReplaceRows(records.Select(ToRow).OrderBy(r => r.GroupId).ThenBy(r => r.Item));
        return table;
    }

    /// <summary>Rows to remove, add and update to turn <paramref name="existing"/> into
    /// <paramref name="wanted"/>; rows are matched by item (and group, where the table keys on it).</summary>
    internal void CallDiff(int entry, List<TRecord> existing, IReadOnlyList<LootRow> wanted,
        Action<TRecord> remove, Action<TRecord> add)
    {
        bool keyedByGroup = typeof(TRecord).GetCustomAttributes<PrimaryKeyAttribute>().Single()
            .PropertyNames.Contains("Groupid");
        (int, int) KeyOf(int item, int group) => (item, keyedByGroup ? group : 0);

        var seen = new HashSet<(int, int)>();
        foreach (LootRow row in wanted)
        {
            if (!seen.Add(KeyOf(row.Item, row.GroupId)))
            {
                throw new InvalidOperationException(
                    $"{Kind} loot {entry} lists item {row.Item} twice" + (keyedByGroup ? $" in group {row.GroupId}." : "."));
            }
        }

        Dictionary<(int, int), TRecord> byKey = existing.ToDictionary(
            r => KeyOf((int)ItemProp.GetValue(r)!, (int)GroupProp.GetValue(r)!));
        foreach (KeyValuePair<(int, int), TRecord> pair in byKey.Where(p => !seen.Contains(p.Key)))
        {
            remove(pair.Value);
        }

        foreach (LootRow row in wanted)
        {
            if (byKey.TryGetValue(KeyOf(row.Item, row.GroupId), out TRecord? record))
            {
                Write(row, record);
                continue;
            }

            record = new TRecord();
            EntryProp.SetValue(record, entry);
            ItemProp.SetValue(record, row.Item);
            Write(row, record);
            add(record);
        }
    }

    private static Expression<Func<TRecord, bool>> EntryEquals(int entry)
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        return Expression.Lambda<Func<TRecord, bool>>(
            Expression.Equal(Expression.Property(p, EntryProp), Expression.Constant(entry)), p);
    }

    private static Expression<Func<TRecord, int>> EntrySelector()
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        return Expression.Lambda<Func<TRecord, int>>(Expression.Property(p, EntryProp), p);
    }

    private static Expression<Func<TRecord, int?>> NullableEntrySelector()
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        return Expression.Lambda<Func<TRecord, int?>>(
            Expression.Convert(Expression.Property(p, EntryProp), typeof(int?)), p);
    }

    private static readonly MethodInfo ListContains =
        typeof(List<int>).GetMethod(nameof(List<int>.Contains), [typeof(int)])!;

    private static Expression<Func<TRecord, bool>> EntryIn(List<int> entries)
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        return Expression.Lambda<Func<TRecord, bool>>(
            Expression.Call(Expression.Constant(entries), ListContains, Expression.Property(p, EntryProp)), p);
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext db = Storage.CreateContext();
        IQueryable<TRecord> query = db.Set<TRecord>().AsNoTracking();
        if (!string.IsNullOrWhiteSpace(filter))
        {
            if (!int.TryParse(filter, out int id))
            {
                return [];
            }

            query = query.Where(EntryEquals(id));
        }

        List<int> entries = await query.Select(EntrySelector()).Distinct().OrderBy(e => e).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);
        return (await CountsAsync(db, entries).ConfigureAwait(false))
            .Select(p => new CatalogSearchResult(p.Key.ToString(), RowCountText(p.Value))).ToList();
    }

    private static string RowCountText(int count) => count == 1 ? "1 row" : $"{count} rows";

    private static async Task<SortedDictionary<int, int>> CountsAsync(WorldDbContext db, List<int> entries)
    {
        List<int> all = entries.Count == 0
            ? []
            : await db.Set<TRecord>().AsNoTracking().Where(EntryIn(entries)).Select(EntrySelector())
                .ToListAsync().ConfigureAwait(false);
        var counts = new SortedDictionary<int, int>();
        foreach (int e in all)
        {
            counts[e] = counts.GetValueOrDefault(e) + 1;
        }

        return counts;
    }

    public async Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        var remaining = new List<int>();
        foreach (string key in keys)
        {
            if (!int.TryParse(key, out int id))
            {
                continue;
            }

            if (Open(context, id) is { } open)
            {
                result[key] = RowCountText(open.Rows.Count);
            }
            else
            {
                remaining.Add(id);
            }
        }

        if (remaining.Count > 0)
        {
            await using WorldDbContext db = Storage.CreateContext();
            foreach ((int entry, int count) in await CountsAsync(db, remaining).ConfigureAwait(false))
            {
                result[entry.ToString()] = RowCountText(count);
            }
        }

        return result;
    }

    private LootTable? Open(EditorContext context, int id) =>
        context.Catalog.OfType<LootTable>().FirstOrDefault(t => t.ID == id && t.Kind == Kind);

    public async Task<CatalogEntity?> OpenAsync(EditorContext context, string key)
    {
        if (!int.TryParse(key, out int id))
        {
            return null;
        }

        if (Open(context, id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = Storage.CreateContext();
        List<TRecord> rows = await db.Set<TRecord>().AsNoTracking().Where(EntryEquals(id))
            .ToListAsync().ConfigureAwait(false);
        if (rows.Count == 0)
        {
            return null;
        }

        LootTable table = CallToEntity(id, rows);
        context.Catalog.Add(table);
        return table;
    }

    public async Task<string?> SuggestKeyAsync()
    {
        await using WorldDbContext db = Storage.CreateContext();
        int? max = await db.Set<TRecord>().MaxAsync(NullableEntrySelector()).ConfigureAwait(false);
        return ((max ?? 0) + 1).ToString();
    }

    public CatalogEntity Create(EditorContext context, string key)
    {
        if (!int.TryParse(key, out int id))
        {
            throw new InvalidOperationException($"'{key}' is not a valid {CatalogName} entry.");
        }

        if (Open(context, id) is not null)
        {
            throw new InvalidOperationException($"{CatalogName} {id} is already open.");
        }

        bool exists = BlockingWork.Run(async () =>
        {
            await using WorldDbContext db = Storage.CreateContext();
            return await db.Set<TRecord>().AsNoTracking().AnyAsync(EntryEquals(id)).ConfigureAwait(false);
        });
        if (exists)
        {
            throw new InvalidOperationException($"{CatalogName} {id} already exists in the database.");
        }

        var table = new LootTable(Kind) { ID = id };
        var command = new CreateCatalogEntityCommand(context.Catalog, table);
        command.Apply();
        context.EditSessions.Record(command);
        return table;
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var table = (LootTable)entity;
        void Sync()
        {
            List<TRecord> existing = db.Set<TRecord>().Where(EntryEquals(table.ID)).ToList();
            CallDiff(table.ID, existing, table.Rows, r => db.Set<TRecord>().Remove(r), r => db.Set<TRecord>().Add(r));
        }

        return SavedEntityStaging.Stage(table, Sync, Sync);
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var table = (LootTable)entity;
        SavedEntityStaging.StageDelete(table, () =>
            db.Set<TRecord>().RemoveRange(db.Set<TRecord>().Where(EntryEquals(table.ID)).ToList()));
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
        if (ImGui.Button($"New {CatalogName}"))
        {
            _createId = int.Parse(BlockingWork.Run(SuggestKeyAsync) ?? "1");
            _createError = null;
            _createOpenRequested = true;
        }

        string popupId = $"Create {CatalogName}";
        if (_createOpenRequested)
        {
            ImGui.OpenPopup(popupId);
            _createOpenRequested = false;
            _createActive = true;
        }

        if (!_createActive)
        {
            return;
        }

        bool open = true;
        ImGuiEx.PopupModal(popupId, true, ref open, ImGuiWindowFlags.AlwaysAutoResize, () =>
        {
            ImGui.InputInt("Entry", ref _createId);
            if (_createError is not null)
            {
                ImGuiEx.TextColored(CommonColors.Error, _createError);
            }

            if (ImGui.Button("Create", new Vector2(120.0f, 0.0f)))
            {
                try
                {
                    onCreated(Create(context, _createId.ToString()));
                    open = false;
                }
                catch (InvalidOperationException error)
                {
                    _createError = error.Message;
                }
            }

            ImGui.SameLine();
            if (ImGui.Button("Cancel", new Vector2(120.0f, 0.0f)))
            {
                open = false;
            }
        });

        if (!open)
        {
            _createActive = false;
        }
    }

    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var table = (LootTable)entity;
        for (int i = 0; i < table.Rows.Count; i++)
        {
            LootRow row = table.Rows[i];
            int index = i;
            ImGui.PushID(index);
            ImGui.Text($"Row {index}");
            ImGui.SameLine();
            if (ImGui.SmallButton("Remove"))
            {
                Apply(context, table.RemoveRow(index));
                ImGui.PopID();
                break;
            }

            Int(context, table, index, "Item", row.Item, nameof(LootRow.Item));
            Float(context, table, index, "Chance", row.Chance, nameof(LootRow.Chance));
            Int(context, table, index, "Group", row.GroupId, nameof(LootRow.GroupId));
            Int(context, table, index, "Min Count / Ref", row.MinCountOrRef, nameof(LootRow.MinCountOrRef));
            Int(context, table, index, "Max Count", row.MaxCount, nameof(LootRow.MaxCount));
            Int(context, table, index, "Condition", row.ConditionId, nameof(LootRow.ConditionId));
            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Row"))
        {
            Apply(context, table.AddRow(0));
        }
    }

    private static void Int(EditorContext context, LootTable t, int index, string label, int current, string field)
    {
        int value = current;
        ImGui.SetNextItemWidth(140.0f);
        if (ImGui.InputInt(label, ref value) && value != current)
        {
            Apply(context, t.SetRowField(index, field, value));
        }
    }

    private static void Float(EditorContext context, LootTable t, int index, string label, float current, string field)
    {
        float value = current;
        ImGui.SetNextItemWidth(140.0f);
        if (ImGui.DragFloat(label, ref value, 0.1f) && !value.Equals(current))
        {
            Apply(context, t.SetRowField(index, field, value));
        }
    }

    private static void Apply(EditorContext context, IEditCommand command)
    {
        command.Apply();
        context.EditSessions.Record(command);
    }
}
