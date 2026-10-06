using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Resolves template entries to appearances, sharing one in-flight query per entry.</summary>
public sealed class SpawnAppearances(WorldStorage storage)
{
    private readonly object _lock = new();
    private readonly Dictionary<uint, Task<CreatureAppearance?>> _creatures = [];
    private readonly Dictionary<uint, Task<CreatureAppearance?>> _gameObjects = [];

    public Task<CreatureAppearance?> Creature(uint entry) => Cached(_creatures, entry, ResolveCreatureAsync);

    public Task<CreatureAppearance?> GameObject(uint entry) => Cached(_gameObjects, entry, ResolveGameObjectAsync);

    private Task<CreatureAppearance?> Cached(Dictionary<uint, Task<CreatureAppearance?>> cache, uint entry,
        System.Func<uint, Task<CreatureAppearance?>> resolve)
    {
        lock (_lock)
        {
            if (!cache.TryGetValue(entry, out Task<CreatureAppearance?>? task))
            {
                cache[entry] = task = Task.Run(() => resolve(entry));
            }

            return task;
        }
    }

    private async Task<CreatureAppearance?> ResolveCreatureAsync(uint entry)
    {
        await using WorldDbContext db = storage.CreateContext();
        CreatureTemplateRecord? template = await db.CreatureTemplate.AsNoTracking()
            .FirstOrDefaultAsync(row => row.Entry == (int)entry).ConfigureAwait(false);
        if (template == null || template.DisplayId1 <= 0)
        {
            return null;
        }

        CreatureAppearance? appearance = await CreatureAppearanceResolver.ResolveAsync(storage, template.DisplayId1)
            .ConfigureAwait(false);
        return appearance != null && template.DisplayScale1 > 0
            ? appearance with { Scale = appearance.Scale * template.DisplayScale1 }
            : appearance;
    }

    private async Task<CreatureAppearance?> ResolveGameObjectAsync(uint entry)
    {
        await using WorldDbContext db = storage.CreateContext();
        GameobjectTemplateRecord? template = await db.GameobjectTemplate.AsNoTracking()
            .FirstOrDefaultAsync(row => row.Entry == (int)entry).ConfigureAwait(false);
        if (template == null)
        {
            return null;
        }

        DbcGameobjectdisplayinfoRecord? display = await db.DbcGameobjectdisplayinfo.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == template.DisplayId).ConfigureAwait(false);
        if (display == null)
        {
            return null;
        }

        CreatureAppearance? appearance = CreatureAppearanceResolver.Build(WowFileDataIds.Shared, display.FileDataID, 1f, []);
        return appearance != null && template.Size > 0 ? appearance with { Scale = template.Size } : appearance;
    }
}
