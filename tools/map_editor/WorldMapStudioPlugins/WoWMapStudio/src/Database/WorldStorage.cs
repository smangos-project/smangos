using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace WorldMapStudio.WoW;

/// <summary>
/// The server's world tables, which live in the same database file as <see cref="EditorStorage"/>'s.
/// It owns none of them: <c>database_tool</c> creates them (and <c>generate-wms</c> writes their
/// records), and this storage only validates them. Hosts the catalog factories for those tables.
/// </summary>
[Subsystem(nameof(DatabaseSystem))]
public sealed partial class WorldStorage : Storage, ISubsystemHost
{
    public const string StorageName = "World";

    private readonly DatabaseSystem _database;

    public override string Name => StorageName;

    public WorldStorage(DatabaseSystem database)
    {
        _database = database;
        InitializeSubsystems();
    }

    public override bool OwnsLocation => false;

    // Read per call: storages are constructed in alphabetical order, so EditorStorage may not exist yet.
    public override StorageLocation Location => _database.EditorStorage.Location;

    public override bool OwnsTable(string table) => false;

    /// <summary>Reaches editor-side systems a catalog factory needs; subsystems only receive their parent.</summary>
    public EditorContext Context => _database.Context;

    /// <summary>Opens a short-lived context for one unit of work against this storage.</summary>
    public WorldDbContext CreateContext() => new(BuildOptions<WorldDbContext>(), TableConfigurations.ToList());

    public override void EnsureSchema()
    {
    }

    public override Schema? ExpectedSchema()
    {
        using WorldDbContext context = CreateContext();
        return ModelSchema.Extract(context);
    }

    public override Task CommitAsync(IReadOnlyList<IEntity> saves, IReadOnlyList<IEntity> deletes) =>
        CommitAsync(CreateContext, saves, deletes);
}
