using System;
using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>
/// The <c>(texture, slot)</c> materials of one map: which material puts a texture in an MCNK slot, and
/// which texture a material stands for. Rows are the importer's own (<see cref="AdtImportCatalog"/>), so
/// imported and tool-created materials are indistinguishable. Creating one is a catalog write outside the
/// edit session, like the importer's: a material nothing references cannot change a chunk, and the edit
/// that starts using it is what stamps the chunks.
/// </summary>
public sealed class AdtSlotMaterials
{
    private readonly EditorContext _editor;
    private readonly MapId _map;
    private readonly Action<IReadOnlyList<IEntity>> _persist;
    private readonly Dictionary<(string Key, int Slot), LandscapeMaterial> _byPair = [];
    private readonly Dictionary<int, LandscapeMaterial> _byId = [];

    /// <param name="persist">Stores newly created materials. Null writes them to the editor database.</param>
    public AdtSlotMaterials(EditorContext editor, MapId map, Action<IReadOnlyList<IEntity>>? persist = null)
    {
        _editor = editor;
        _map = map;
        _persist = persist ?? PersistToDatabase;
        Refresh();
    }

    /// <summary>Rereads the map's slot materials from the catalog.</summary>
    public void Refresh()
    {
        _byPair.Clear();
        _byId.Clear();
        foreach (LandscapeMaterial material in _editor.Catalog.OfType<LandscapeMaterial>())
        {
            if (material.Map.Equals(_map) &&
                AdtImportCatalog.TryParseSlotMaterialName(material.DisplayName, out _, out int slot) &&
                material.TexturePath.Length > 0)
            {
                _byPair[(AdtTexturePath.Key(material.TexturePath), slot)] = material;
                if (material.RecordId is int id)
                {
                    _byId[id] = material;
                }
            }
        }
    }

    public int? MaterialFor(string texture, int slot) =>
        _byPair.TryGetValue((AdtTexturePath.Key(texture), slot), out LandscapeMaterial? material) ? material.RecordId : null;

    public string? TextureOf(int? materialId) =>
        materialId is int id && _byId.TryGetValue(id, out LandscapeMaterial? material) ? material.TexturePath : null;

    public int? EffectIdOf(int? materialId) =>
        materialId is int id && _byId.TryGetValue(id, out LandscapeMaterial? material) ? material.SurfaceEffectId : null;

    /// <summary>Every distinct texture the map's slot materials use.</summary>
    public IReadOnlyList<string> Textures() => _byPair.Values
        .Select(material => material.TexturePath)
        .Distinct(StringComparer.OrdinalIgnoreCase)
        .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
        .ToList();

    /// <summary>Creates whichever of the texture's four slot materials are missing. A new one copies the
    /// ground-effect id another slot of the same texture already carries.</summary>
    public void EnsureTexture(string texture)
    {
        EnsurePairs(Enumerable.Range(0, AdtTileComponent.SlotCount).Select(slot => (texture, slot)));
    }

    /// <summary>Creates whichever of the given pairs are missing, in one catalog write.</summary>
    public void EnsurePairs(IEnumerable<(string Texture, int Slot)> pairs)
    {
        var created = new List<IEntity>();
        foreach ((string texture, int slot) in pairs.Distinct())
        {
            if (MaterialFor(texture, slot) != null)
            {
                continue;
            }

            (string function, string parameters) = AdtImportCatalog.SlotAlpha(slot);
            int effect = Enumerable.Range(0, AdtTileComponent.SlotCount)
                .Select(other => _byPair.GetValueOrDefault((AdtTexturePath.Key(texture), other))?.SurfaceEffectId ?? 0)
                .FirstOrDefault(id => id != 0);

            var material = new LandscapeMaterial
            {
                Map = _map,
                Name = AdtImportCatalog.SlotMaterialName(TextureId(texture), slot),
                TexturePath = texture,
                AlphaFunction = function,
                AlphaParameters = parameters,
                SurfaceEffectId = effect,
            };
            _editor.Catalog.AssignId(material);
            _editor.Catalog.Add(material);
            _byPair[(AdtTexturePath.Key(texture), slot)] = material;
            if (material.RecordId is int id)
            {
                _byId[id] = material;
            }

            created.Add(material);
        }

        if (created.Count > 0)
        {
            _persist(created);
        }
    }

    private static uint TextureId(string path) =>
        WowFileDataIds.Shared.IdOf(path)
        ?? (path.StartsWith("unknown/", StringComparison.Ordinal) && uint.TryParse(System.IO.Path.GetFileNameWithoutExtension(path), out uint raw) ? raw : WowFileDataIds.Shared.Allocate(path));

    private void PersistToDatabase(IReadOnlyList<IEntity> created)
    {
        var toSave = created.ToList();
        BlockingWork.Run(() => _editor.Database.EditorStorage.CommitAsync(toSave, []));
    }
}
