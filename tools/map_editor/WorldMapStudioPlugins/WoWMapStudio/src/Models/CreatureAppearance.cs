using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>What a creature display id renders as: a model path plus the texture bound to each
/// replaceable M2 slot (see <see cref="M2TextureSlot"/>).</summary>
public sealed record CreatureAppearance(string ModelPath, float Scale, IReadOnlyDictionary<uint, string> SlotTextures)
{
    /// <summary>Instantiate options that bind <see cref="SlotTextures"/> onto the model's replaceable slots.</summary>
    public ModelInstantiateOptions ToInstantiateOptions() => new()
    {
        MaterialOverride = (_, _, material) =>
            material.TypeId == M2Material.TypeId
            && SlotTextures.TryGetValue((uint)material.Values.GetInt(M2Material.TextureSlot), out string? texture)
                ? material.With(M2Material.Texture, texture)
                : material,
    };
}

/// <summary>Resolves `dbc_creaturedisplayinfo` rows to a <see cref="CreatureAppearance"/> through
/// `dbc_creaturemodeldata` and the listfile.</summary>
public static class CreatureAppearanceResolver
{
    private static readonly uint[] VariationSlots = [M2TextureSlot.CreatureSkin1, M2TextureSlot.CreatureSkin2, M2TextureSlot.CreatureSkin3];

    public static async Task<CreatureAppearance?> ResolveAsync(WorldStorage storage, int displayId, WowFileDataIds? ids = null)
    {
        await using WorldDbContext db = storage.CreateContext();
        DbcCreaturedisplayinfoRecord? display = await db.DbcCreaturedisplayinfo.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == displayId).ConfigureAwait(false);
        if (display == null)
        {
            return null;
        }

        DbcCreaturemodeldataRecord? model = await db.DbcCreaturemodeldata.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == display.ModelID).ConfigureAwait(false);
        if (model == null)
        {
            return null;
        }

        return Build(ids ?? WowFileDataIds.Shared, model.FileDataID, display.CreatureModelScale,
            [display.TextureVariationFileDataID1, display.TextureVariationFileDataID2, display.TextureVariationFileDataID3]);
    }

    internal static CreatureAppearance? Build(WowFileDataIds ids, int modelFileDataId, float scale, int[] variationFileDataIds)
    {
        if (modelFileDataId <= 0 || ids.PathOf((uint)modelFileDataId) is not { } modelPath)
        {
            return null;
        }

        var textures = new Dictionary<uint, string>();
        for (int i = 0; i < variationFileDataIds.Length && i < VariationSlots.Length; i++)
        {
            if (variationFileDataIds[i] > 0 && ids.PathOf((uint)variationFileDataIds[i]) is { } path)
            {
                textures[VariationSlots[i]] = path;
            }
        }

        return new CreatureAppearance(modelPath, scale > 0 ? scale : 1f, textures);
    }
}
