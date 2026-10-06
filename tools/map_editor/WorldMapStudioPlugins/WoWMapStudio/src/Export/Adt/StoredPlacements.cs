using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Reads a project map's model placements out of the editor database.</summary>
public static class StoredPlacements
{
    public static async Task<List<EditorPlacement>> LoadAsync(EditorStorage storage, AssetSystem assets, int mapId)
    {
        List<(MapEntityRecord Entity, SceneModelRendererComponentRecord Model)> rows;
        using (IDisposable reader = await storage.Lock.ReaderAsync().ConfigureAwait(false))
        {
            await using EditorDbContext context = storage.CreateContext();
            rows = (await context.MapEntities.AsNoTracking()
                .Where(entity => entity.MapId == mapId)
                .Join(context.Set<SceneModelRendererComponentRecord>().AsNoTracking(),
                    entity => entity.Id, model => model.EntityId, (entity, model) => new { entity, model })
                .ToListAsync().ConfigureAwait(false))
                .Select(row => (row.entity, row.model)).ToList();
        }

        var m2Bounds = new Dictionary<string, Aabb?>(StringComparer.OrdinalIgnoreCase);
        var wmoRoots = new Dictionary<string, WmoRoot?>(StringComparer.OrdinalIgnoreCase);
        var result = new List<EditorPlacement>();
        foreach ((MapEntityRecord e, SceneModelRendererComponentRecord m) in rows.OrderBy(r => r.Entity.Id))
        {
            string path = m.ModelPath;
            bool wmo = path.EndsWith(".wmo", StringComparison.OrdinalIgnoreCase);
            Aabb? bounds;
            ushort set = 0;
            if (wmo)
            {
                if (!wmoRoots.TryGetValue(path, out WmoRoot? root))
                {
                    byte[]? bytes = await assets.ReadAssetBytesAsync(path).ConfigureAwait(false);
                    wmoRoots[path] = root = bytes != null ? WmoRootReader.Read(bytes, path) : null;
                }

                bounds = root?.Bounds;
                if (root != null && !string.IsNullOrEmpty(m.FormatState))
                {
                    for (int i = 1; i < root.DoodadSets.Length; i++)
                    {
                        string name = root.DoodadSets[i].Name.Length > 0 ? root.DoodadSets[i].Name : $"Set {i}";
                        if (name == m.FormatState)
                        {
                            set = (ushort)i;
                        }
                    }
                }
            }
            else if (!m2Bounds.TryGetValue(path, out bounds))
            {
                byte[]? bytes = await assets.ReadAssetBytesAsync(path).ConfigureAwait(false);
                m2Bounds[path] = bounds = bytes != null ? M2Reader.Read(bytes, path)?.Bounds : null;
            }

            var basis = new Basis(new Quaternion((float)e.RotX, (float)e.RotY, (float)e.RotZ, (float)e.RotW))
                .Scaled(new Vector3((float)e.ScaleX, (float)e.ScaleY, (float)e.ScaleZ));
            var transform = new Transform3D(basis, new Vector3((float)e.PosX, (float)e.PosY, (float)e.PosZ));
            result.Add(new EditorPlacement(e.Id, path, transform, bounds ?? default, set));
        }

        return result;
    }
}
