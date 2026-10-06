using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Resolves a liquid type id to an animated <see cref="Texture2DArray"/>. Lazy per liquid
/// type rather than uploading every row up front — this editor loads assets on demand.
///
/// The frame-filename template comes from the <see cref="LiquidType"/> catalog
/// (<see cref="WowLiquidCatalog"/>), not a <c>.dbc</c> file: the database is this project's DBC
/// authority. One instance is owned per consumer (see <see cref="WowLiquidMaterialType"/>).
/// </summary>
public sealed class LiquidTextureLibrary
{
    private const int MaxFrames = 30;

    // Procedural-water fallback — used when the catalog names no frames or they don't load.
    private const string FallbackTemplate = "XTextures/river/lake_a.{0}.blp";

    private readonly Dictionary<int, Texture2DArray?> _arrays = new();

    /// <summary>The animated frame array for <paramref name="liquidTypeId"/>, built and cached on first
    /// use. Null if the id isn't a known liquid type, or its template (and the procedural-water
    /// fallback) both fail to load — an absent client, most likely.</summary>
    public Texture2DArray? GetTextureArray(EditorContext context, int liquidTypeId)
    {
        if (_arrays.TryGetValue(liquidTypeId, out Texture2DArray? cached))
        {
            return cached;
        }

        string[] framePaths = WowLiquidCatalog.Find(context, liquidTypeId)?.FrameFileDataIds
            .Select(id => WowFileDataIds.For(context).PathOf((uint)id))
            .OfType<string>()
            .Take(MaxFrames)
            .ToArray() ?? [];

        Texture2DArray? array = (framePaths.Length > 0 ? BuildTextureArray(context.Assets, framePaths) : null)
            ?? BuildTextureArray(context.Assets, FallbackFrames());
        if (array == null)
        {
            // Loud rather than silent: an unbound sampler renders as flat white, which reads as "the
            // tint is broken" rather than "no client is configured" unless something says so.
            GD.PushWarning($"[WoW Liquid] No animation frames for liquid type {liquidTypeId}. Is a client configured as an asset source?");
        }

        _arrays[liquidTypeId] = array;
        return array;
    }

    /// <summary>Drops attempted-load memoization for a specific liquid type — e.g. after configuring a
    /// new client, so a previously "not found" result is retried instead of staying cached as null.</summary>
    public void Invalidate(int liquidTypeId) => _arrays.Remove(liquidTypeId);

    private static string[] FallbackFrames() =>
        Enumerable.Range(1, MaxFrames).Select(frame => string.Format(FallbackTemplate, frame)).ToArray();

    private static Texture2DArray? BuildTextureArray(AssetSystem assets, IReadOnlyList<string> paths)
    {
        var frames = new Godot.Collections.Array<Image>();
        Image? first = null;

        foreach (string path in paths)
        {
            Image? image = LoadImage(assets, path);
            if (image == null)
            {
                break;
            }

            first ??= image;

            // Texture2DArray.CreateFromImages requires every layer to share format and size: a frame
            // that disagrees with the first repeats the first instead of failing the whole array.
            frames.Add(image.GetFormat() == first.GetFormat() && image.GetWidth() == first.GetWidth() && image.GetHeight() == first.GetHeight()
                ? image
                : first);
        }

        if (frames.Count == 0)
        {
            return null;
        }

        var array = new Texture2DArray();
        array.CreateFromImages(frames);
        return array;
    }

    private static Image? LoadImage(AssetSystem assets, string path)
    {
        foreach (ITextureLoader loader in assets.TextureLoaders.Where(loader => loader.CanLoad(path)))
        {
            if (loader.LoadTextureImageAsync(assets, path).GetAwaiter().GetResult() is { } image)
            {
                return image;
            }
        }

        return null;
    }
}
