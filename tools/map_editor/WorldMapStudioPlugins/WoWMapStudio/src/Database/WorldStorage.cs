using System;

namespace WorldMapStudio.WoW;

/// <summary>
/// The server's world tables, which live in the same database file as <see cref="EditorStorage"/>'s.
/// It owns none of them: <c>database_tool</c> creates them, and this storage only validates them.
/// </summary>
[Subsystem(nameof(DatabaseSystem))]
public sealed class WorldStorage : Storage
{
    public const string StorageName = "World";

    private readonly DatabaseSystem _database;

    public override string Name => StorageName;

    public WorldStorage(DatabaseSystem database)
    {
        _database = database;
    }

    public override bool OwnsLocation => false;

    public override StorageLocation Location => _database.EditorStorage.Location;

    public override bool OwnsTable(string table) => false;

    public override void EnsureSchema()
    {
    }
}
