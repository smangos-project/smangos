using System;
using System.Threading.Tasks;
using Godot;
using WoWLib.Formats.BLP;
using GodotImage = Godot.Image;
using WowImage = WoWLib.Formats.BLP.Image;

namespace WorldMapStudio.WoW;

/// <summary>Loads WoW BLP2 texture files into Godot RGBA images through wowlib.</summary>
[Subsystem(nameof(AssetSystem))]
public sealed class BLPTextureLoader : ITextureLoader
{
    public BLPTextureLoader(AssetSystem assets)
    {
    }

    public bool CanLoad(string path) => AssetPath.Extension(path).Equals(".blp", StringComparison.OrdinalIgnoreCase);

    public async Task<GodotImage?> LoadTextureImageAsync(AssetSystem assets, string path)
    {
        byte[]? bytes = await assets.ReadAssetBytesAsync(path).ConfigureAwait(false);
        if (bytes == null)
        {
            return null;
        }

        try
        {
            return Decode(bytes);
        }
        catch (Exception e)
        {
            GD.PushWarning($"[WoW BLP] Failed to decode '{path}': {e.Message}");
            return null;
        }
    }

    /// <summary>Decodes mip 0 of a BLP2 file to an RGBA8 image.</summary>
    public static GodotImage Decode(byte[] bytes)
    {
        using var blp = new BLP();
        blp.Read(bytes);
        using WowImage decoded = blp.Decode();
        return GodotImage.CreateFromData((int)decoded.Width, (int)decoded.Height, false, GodotImage.Format.Rgba8, decoded.Pixels.ToArray());
    }
}
