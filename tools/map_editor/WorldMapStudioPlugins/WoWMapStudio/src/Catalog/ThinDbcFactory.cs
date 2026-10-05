using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;
using Vector2 = System.Numerics.Vector2;
using Vector4 = System.Numerics.Vector4;

namespace WorldMapStudio.WoW;

/// <summary>One row of a `dbc_*` table: an int `ID`, edited and saved in place, never null-keyed.</summary>
public interface IDbcCatalogEntity : ISavedCatalogEntity
{
    int ID { get; set; }
}

/// <summary>
/// The whole <see cref="ILazyCatalogEntityFactory"/> + <see cref="ICatalogBrowser"/> surface for a
/// `dbc_*` table that needs nothing bespoke — id search, a create popup, open-by-id, in-place staging,
/// and a declarative field sheet — so a plain table is a ~30-line subclass supplying only its
/// <see cref="ToEntity"/>/<see cref="Write"/> pair and its <see cref="Fields"/>.
///
/// Graduating is incremental: override <see cref="DisplayText"/> for a computed display text,
/// <see cref="TextFilter"/> for name search, <see cref="DrawFields"/> for links or grouping; or drop
/// the base and implement <see cref="ICatalogBrowser"/> directly (what <c>ItemFactory</c> does) losing
/// nothing — the base is a convenience, not a framework.
/// </summary>
public abstract class ThinDbcFactory<TEntity, TRecord> : ILazyCatalogEntityFactory, ICatalogBrowser
    where TEntity : CatalogEntity, IDbcCatalogEntity, new()
    where TRecord : class, new()
{
    protected const int SearchLimit = 50;

    private readonly PropertyInfo _idProperty;
    private readonly Expression<Func<TRecord, int>> _idSelector;
    private readonly Expression<Func<TRecord, int?>> _nullableIdSelector;
    private readonly Func<TRecord, int> _idOf;

    private bool _createOpenRequested;
    private bool _createActive;
    private int _createId;
    private string? _createError;

    protected ThinDbcFactory(WorldStorage storage)
    {
        Storage = storage;
        _idProperty = typeof(TRecord).GetProperty(KeyPropertyName)
            ?? throw new InvalidOperationException($"{typeof(TRecord).Name} has no {KeyPropertyName} column.");
        _idSelector = MakeIdSelector();
        _nullableIdSelector = MakeNullableIdSelector();
        _idOf = _idSelector.Compile();
    }

    protected WorldStorage Storage { get; }

    /// <summary>The record's key column, reflected by the constructor — "ID" for every table but one
    /// keyed differently (e.g. `dbc_spellspecialuniteffect.EnumID`).</summary>
    protected virtual string KeyPropertyName => "ID";

    public Type EntityType => typeof(TEntity);

    public abstract string CatalogName { get; }

    public bool Handles(IEntity entity) => entity is TEntity;

    // The table is already mapped by its generated ITableConfiguration.
    public void Configure(ModelBuilder model)
    {
    }

    /// <summary>Row → entity, with <see cref="ISavedCatalogEntity.IsSaved"/> set true.</summary>
    protected abstract TEntity ToEntity(TRecord record);

    /// <summary>Entity → row, field by field, leaving columns this catalog doesn't model untouched.</summary>
    protected abstract void Write(TEntity entity, TRecord record);

    /// <summary>Loads what a catalog folds in alongside the `dbc_*` row — a server-side row keyed by the
    /// same id, child lists — into a freshly fetched entity, before it joins the registry. No-op by
    /// default.</summary>
    protected virtual Task LoadExtrasAsync(WorldDbContext db, TEntity entity) => Task.CompletedTask;

    /// <summary>Stages the folded-in rows for an entity being saved, alongside the `dbc_*` row's own
    /// insert or update. No-op by default.</summary>
    protected virtual void StageExtras(WorldDbContext db, TEntity entity)
    {
    }

    /// <summary>Stages removal of the folded-in rows when the entity's `dbc_*` row is deleted. No-op by
    /// default.</summary>
    protected virtual void StageDeleteExtras(WorldDbContext db, TEntity entity)
    {
    }

    /// <summary>The field rows, in order — walked through a <see cref="CatalogFieldSheet"/>.</summary>
    protected abstract IEnumerable<CatalogFieldSpec> Fields(TEntity entity);

    /// <summary>Display text for a row, e.g. "1500 ms" — shown after the id in a search result and
    /// wherever else a reference to this row is drawn. Empty = nothing better than the id.</summary>
    protected virtual string DisplayText(TRecord record) => string.Empty;

    /// <summary>Predicate for a non-numeric search term, or null for an id-only catalog.</summary>
    protected virtual Expression<Func<TRecord, bool>>? TextFilter(string term) => null;

    internal enum FilterKind
    {
        All,
        ById,
        ByText,
        None,
    }

    /// <summary>How <see cref="SearchAsync"/> reads a filter string: everything, one id, a text
    /// predicate, or (a non-numeric term on an id-only catalog) nothing.</summary>
    internal FilterKind ClassifyFilter(string filter, out int id)
    {
        id = 0;
        if (string.IsNullOrWhiteSpace(filter))
        {
            return FilterKind.All;
        }

        return int.TryParse(filter, out id)
            ? FilterKind.ById
            : TextFilter(filter) is not null ? FilterKind.ByText : FilterKind.None;
    }

    public virtual async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext db = Storage.CreateContext();

        IQueryable<TRecord> query = db.Set<TRecord>().AsNoTracking();
        switch (ClassifyFilter(filter, out int id))
        {
            case FilterKind.ById:
                query = query.Where(IdEquals(id));
                break;
            case FilterKind.ByText:
                query = query.Where(TextFilter(filter)!);
                break;
            case FilterKind.None:
                return [];
        }

        List<TRecord> rows = await query.OrderBy(_idSelector).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(RowResult).ToList();
    }

    private CatalogSearchResult RowResult(TRecord record) => new(_idOf(record).ToString(), DisplayText(record));

    public virtual async Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        var remaining = new List<int>();

        foreach (string key in keys)
        {
            if (!int.TryParse(key, out int id))
            {
                continue;
            }

            if (context.Catalog.OfType<TEntity>().FirstOrDefault(e => e.ID == id) is { } open)
            {
                var scratch = new TRecord();
                Write(open, scratch);
                result[key] = DisplayText(scratch);
            }
            else
            {
                remaining.Add(id);
            }
        }

        if (remaining.Count > 0)
        {
            await using WorldDbContext db = Storage.CreateContext();
            List<TRecord> rows = await db.Set<TRecord>().AsNoTracking()
                .Where(IdIn(remaining)).ToListAsync().ConfigureAwait(false);
            foreach (TRecord record in rows)
            {
                result[_idOf(record).ToString()] = DisplayText(record);
            }
        }

        return result;
    }

    public async Task<CatalogEntity?> OpenAsync(EditorContext context, string key)
    {
        if (!int.TryParse(key, out int id))
        {
            return null;
        }

        if (context.Catalog.OfType<TEntity>().FirstOrDefault(e => e.ID == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = Storage.CreateContext();
        TRecord? record = await db.Set<TRecord>().AsNoTracking()
            .FirstOrDefaultAsync(IdEquals(id)).ConfigureAwait(false);
        if (record is null)
        {
            return null;
        }

        TEntity entity = ToEntity(record);
        await LoadExtrasAsync(db, entity).ConfigureAwait(false);
        OnEntityLoaded(context, entity);
        context.Catalog.Add(entity);
        return entity;
    }

    /// <summary>Extension point run once right after an entity is constructed — fetched fresh in
    /// <see cref="OpenAsync"/>, or newly made in <see cref="Create"/> — before it joins the registry.
    /// For a subclass to wire per-entity state that isn't a plain field (e.g. a reference back to
    /// <paramref name="context"/>'s registry, the way <c>SpellVisualKit.SetAttachIdAllocator</c>'s
    /// caller wires that). No-op by default.</summary>
    protected virtual void OnEntityLoaded(EditorContext context, TEntity entity)
    {
    }

    public virtual void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (TEntity)entity;
        var sheet = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);
        foreach (CatalogFieldSpec spec in Fields(typed))
        {
            spec.Draw(sheet);
        }
    }

    public async Task<string?> SuggestKeyAsync()
    {
        await using WorldDbContext db = Storage.CreateContext();
        int? max = await db.Set<TRecord>().MaxAsync(_nullableIdSelector).ConfigureAwait(false);
        return ((max ?? 0) + 1).ToString();
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
        if (ImGui.Button($"New {CatalogName}"))
        {
            _createId = BlockingWork.Run(async () =>
            {
                await using WorldDbContext db = Storage.CreateContext();
                int? max = await db.Set<TRecord>().MaxAsync(_nullableIdSelector).ConfigureAwait(false);
                return (max ?? 0) + 1;
            });
            _createError = null;
            _createOpenRequested = true;
        }

        DrawCreatePopup(context, onCreated);
    }

    private void DrawCreatePopup(EditorContext context, Action<CatalogEntity> onCreated)
    {
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
            ImGui.InputInt("ID", ref _createId);

            if (_createError is not null)
            {
                ImGuiEx.TextColored(CommonColors.Error, _createError);
            }

            if (ImGui.Button("Create", new Vector2(120.0f, 0.0f)) && TryCommitCreate(context, onCreated))
            {
                open = false;
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

    private bool TryCommitCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
        CatalogEntity entity;
        try
        {
            entity = Create(context, _createId.ToString());
        }
        catch (InvalidOperationException error)
        {
            _createError = error.Message;
            return false;
        }

        onCreated(entity);
        return true;
    }

    public CatalogEntity Create(EditorContext context, string key)
    {
        if (!int.TryParse(key, out int id))
        {
            throw new InvalidOperationException($"'{key}' is not a valid {CatalogName} id.");
        }

        GuardCreatable(context.Catalog, id);

        bool existsInDb = BlockingWork.Run(async () =>
        {
            await using WorldDbContext db = Storage.CreateContext();
            return await db.Set<TRecord>().AsNoTracking().AnyAsync(IdEquals(id)).ConfigureAwait(false);
        });

        if (existsInDb)
        {
            throw new InvalidOperationException($"{CatalogName} {id} already exists in the database.");
        }

        var entity = new TEntity { ID = id };
        OnEntityLoaded(context, entity);
        var command = new CreateCatalogEntityCommand(context.Catalog, entity);
        command.Apply();
        context.EditSessions.Record(command);
        return entity;
    }

    /// <summary>Throws if an entity for <paramref name="id"/> is already open — the registry half of
    /// <see cref="Create"/>'s guards, split out so it stands alone without a live database.</summary>
    internal void GuardCreatable(CatalogEntityRegistry catalog, int id)
    {
        if (catalog.OfType<TEntity>().Any(e => e.ID == id))
        {
            throw new InvalidOperationException($"{CatalogName} {id} is already open.");
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var typed = (TEntity)entity;

        Action onCommitted = SavedEntityStaging.Stage(typed,
            insert: () =>
            {
                var record = new TRecord();
                _idProperty.SetValue(record, typed.ID);
                Write(typed, record);
                db.Set<TRecord>().Add(record);
            },
            update: () =>
            {
                // Fetched, not freshly constructed, so columns this editor doesn't surface survive.
                TRecord record = db.Set<TRecord>().Find(typed.ID)
                    ?? throw new InvalidOperationException(
                        $"{CatalogName} row {typed.ID} was removed outside the editor.");
                Write(typed, record);
            });

        StageExtras(db, typed);
        return onCommitted;
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var typed = (TEntity)entity;

        SavedEntityStaging.StageDelete(typed, delete: () =>
        {
            StageDeleteExtras(db, typed);
            var stub = new TRecord();
            _idProperty.SetValue(stub, typed.ID);
            db.Set<TRecord>().Remove(stub);
        });
    }

    private Expression<Func<TRecord, bool>> IdEquals(int id)
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        return Expression.Lambda<Func<TRecord, bool>>(
            Expression.Equal(Expression.Property(p, _idProperty), Expression.Constant(id)), p);
    }

    private static readonly MethodInfo ListContains =
        typeof(List<int>).GetMethod(nameof(List<int>.Contains), [typeof(int)])!;

    private Expression<Func<TRecord, bool>> IdIn(List<int> ids)
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        Expression call = Expression.Call(Expression.Constant(ids), ListContains, Expression.Property(p, _idProperty));
        return Expression.Lambda<Func<TRecord, bool>>(call, p);
    }

    private Expression<Func<TRecord, int>> MakeIdSelector()
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        return Expression.Lambda<Func<TRecord, int>>(Expression.Property(p, _idProperty), p);
    }

    private Expression<Func<TRecord, int?>> MakeNullableIdSelector()
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        return Expression.Lambda<Func<TRecord, int?>>(
            Expression.Convert(Expression.Property(p, _idProperty), typeof(int?)), p);
    }
}
