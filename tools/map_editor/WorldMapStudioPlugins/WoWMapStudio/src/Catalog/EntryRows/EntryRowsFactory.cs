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

/// <summary>An entity holding every row of a table that shares one `entry`.</summary>
public interface IEntryRowTable<TRow> : ISavedCatalogEntity
{
    int ID { get; }
    string Kind { get; }
    IReadOnlyList<TRow> Rows { get; }
    void ReplaceRows(IEnumerable<TRow> rows);
}

/// <summary>
/// A table with many rows per `entry` (loot, vendor, trainer lists) as a catalog keyed by `entry`: the
/// entity is the entry's whole row list, diffed against the database on save. A subclass supplies the
/// row mapping, its key and the row widgets.
/// </summary>
public abstract class EntryRowsFactory<TRecord, TTable, TRow> : ILazyCatalogEntityFactory, ICatalogBrowser
    where TRecord : class, new()
    where TTable : CatalogEntity, IEntryRowTable<TRow>
{
    private const int SearchLimit = 50;

    private static readonly PropertyInfo EntryProp = typeof(TRecord).GetProperty("Entry") ?? typeof(TRecord).GetProperty("MenuId") ?? Prop("Id");

    private bool _createOpenRequested;
    private bool _createActive;
    private int _createId;
    private string? _createError;

    protected EntryRowsFactory(WorldStorage storage)
    {
        Storage = storage;
    }

    protected WorldStorage Storage { get; }

    public Type EntityType => typeof(TTable);

    public abstract string CatalogName { get; }

    /// <summary>The kind carried by this catalog's entities; one factory per kind.</summary>
    protected string Kind => CatalogName;

    public bool Handles(IEntity entity) => entity is TTable t && t.Kind == Kind;

    public virtual void Configure(ModelBuilder model)
    {
    }

    private static PropertyInfo Prop(string name) => typeof(TRecord).GetProperty(name)!;

    internal TTable CallToEntity(int entry, IEnumerable<TRecord> records)
    {
        TTable table = NewTable(entry);
        table.IsSaved = true;
        table.ReplaceRows(Order(records.Select(ToRow)));
        return table;
    }

    protected abstract TTable NewTable(int entry);

    protected abstract TRow ToRow(TRecord record);

    /// <summary>Copies the non-key columns of <paramref name="row"/> into <paramref name="record"/>.</summary>
    protected abstract void WriteRow(TRow row, TRecord record);

    /// <summary>Copies the key columns (other than `entry`) of <paramref name="row"/> into a new record.</summary>
    protected abstract void WriteKey(TRow row, TRecord record);

    /// <summary>What identifies a row within its entry.</summary>
    protected abstract object KeyOf(TRow row);

    protected virtual IEnumerable<TRow> Order(IEnumerable<TRow> rows) => rows;

    protected abstract string DuplicateMessage(int entry, TRow row);

    protected abstract void DrawRows(EditorContext context, TTable table);

    /// <summary>Removes, adds and updates records to turn <paramref name="existing"/> into
    /// <paramref name="wanted"/>, matching rows by <see cref="KeyOf"/>.</summary>
    internal void CallDiff(int entry, List<TRecord> existing, IReadOnlyList<TRow> wanted,
        Action<TRecord> remove, Action<TRecord> add)
    {
        var seen = new HashSet<object>();
        foreach (TRow row in wanted)
        {
            if (!seen.Add(KeyOf(row)))
            {
                throw new InvalidOperationException(DuplicateMessage(entry, row));
            }
        }

        Dictionary<object, TRecord> byKey = existing.ToDictionary(r => KeyOf(ToRow(r)));
        foreach (KeyValuePair<object, TRecord> pair in byKey.Where(p => !seen.Contains(p.Key)))
        {
            remove(pair.Value);
        }

        foreach (TRow row in wanted)
        {
            if (byKey.TryGetValue(KeyOf(row), out TRecord? record))
            {
                WriteRow(row, record);
                continue;
            }

            record = new TRecord();
            EntryProp.SetValue(record, entry);
            WriteKey(row, record);
            WriteRow(row, record);
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

    private TTable? Open(EditorContext context, int id) =>
        context.Catalog.OfType<TTable>().FirstOrDefault(t => t.ID == id && t.Kind == Kind);

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

        TTable table = CallToEntity(id, rows);
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

        TTable table = NewTable(id);
        var command = new CreateCatalogEntityCommand(context.Catalog, table);
        command.Apply();
        context.EditSessions.Record(command);
        return table;
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var table = (TTable)entity;
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
        var table = (TTable)entity;
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
        Action<string, string> navigate, string fieldFilter) => DrawRows(context, (TTable)entity);

    protected static void Apply(EditorContext context, IEditCommand command)
    {
        command.Apply();
        context.EditSessions.Record(command);
    }
}
