using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Reads which texture each chunk slot of an imported tile currently uses out of the editor database.</summary>
public static class StoredSlotTextures
{
    public static async Task<Dictionary<(int Col, int Row), IReadOnlyList<uint?>>> LoadAsync(
        EditorStorage storage,
        int mapId,
        int fileX,
        int fileY,
        System.Func<string, uint> fdidOfPath)
    {
        List<SceneAdtTileChunkRecord> chunks;
        Dictionary<int, string> materialPaths;
        using (System.IDisposable reader = await storage.Lock.ReaderAsync().ConfigureAwait(false))
        {
            await using EditorDbContext db = storage.CreateContext();
            chunks = await db.Set<SceneAdtTileComponentRecord>().AsNoTracking()
                .Where(component => component.FileX == fileX && component.FileY == fileY)
                .Join(db.MapEntities.AsNoTracking().Where(entity => entity.MapId == mapId), component => component.EntityId, entity => entity.Id, (component, _) => component.EntityId)
                .Join(db.Set<SceneAdtTileChunkRecord>().AsNoTracking(), id => id, chunk => chunk.EntityId, (_, chunk) => chunk)
                .ToListAsync().ConfigureAwait(false);
            materialPaths = await db.Set<LandscapeMaterialRecord>().AsNoTracking()
                .Where(material => material.MapId == mapId)
                .ToDictionaryAsync(material => material.Id, material => material.TexturePath).ConfigureAwait(false);
        }

        var result = new Dictionary<(int, int), IReadOnlyList<uint?>>();
        foreach (SceneAdtTileChunkRecord chunk in chunks)
        {
            var slots = new uint?[AdtTileComponent.SlotCount];
            foreach (string record in chunk.Slots.Split('|', System.StringSplitOptions.RemoveEmptyEntries))
            {
                string[] parts = record.Split(':');
                if (parts.Length >= 3 && int.TryParse(parts[0], CultureInfo.InvariantCulture, out int slot) && slot is >= 0 and < AdtTileComponent.SlotCount
                    && int.TryParse(parts[2], CultureInfo.InvariantCulture, out int material) && materialPaths.TryGetValue(material, out string? path))
                {
                    slots[slot] = fdidOfPath(path);
                }
            }

            result[(chunk.LocalCol, chunk.LocalRow)] = slots;
        }

        return result;
    }
}
