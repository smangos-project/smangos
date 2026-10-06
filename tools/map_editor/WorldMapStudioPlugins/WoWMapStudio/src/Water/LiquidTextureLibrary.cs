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

    // Procedural-water fallback — used when the catalog names no template
    // (procedural-water rows) or the named frames don't load.
    private const string FallbackTemplate = @"XTextures\river\lake_a.%d.blp";

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

        string template = WowLiquidCatalog.Find(context, liquidTypeId)?.TextureTemplate is { Length: > 0 } configured
            ? configured
            : FallbackTemplate;

        Texture2DArray? array = BuildTextureArray(context.Assets, template) ?? BuildTextureArray(context.Assets, FallbackTemplate);
        if (array == null)
        {
            // Loud rather than silent: an unbound sampler renders as flat white, which reads as "the
            // tint is broken" rather than "no client is configured" unless something says so.
            GD.PushWarning($"[WoW Liquid] No animation frames for liquid type {liquidTypeId}: tried '{FrameFile(template, 1)}' and '{FrameFile(FallbackTemplate, 1)}'. Is a client configured as an asset source?");
        }

        _arrays[liquidTypeId] = array;
        return array;
    }

    /// <summary>Drops attempted-load memoization for a specific liquid type — e.g. after configuring a
    /// new client, so a previously "not found" result is retried instead of staying cached as null.</summary>
    public void Invalidate(int liquidTypeId) => _arrays.Remove(liquidTypeId);

    /// <summary>One frame's path from a <c>dbc_liquidtype.Texture1</c> <c>printf</c> template — the
    /// <c>%d</c> is the (non-zero-padded) frame number, matching how the client reconstructs frames.</summary>
    private static string FrameFile(string template, int frame) =>
        template.Contains("%d") ? template.Replace("%d", frame.ToString()) : $"{template}{frame}";

    private static Texture2DArray? BuildTextureArray(AssetSystem assets, string template)
    {
        var frames = new Godot.Collections.Array<Image>();
        Image? first = null;

        for (int frame = 1; frame <= MaxFrames; frame++)
        {
            Image? image = LoadImage(assets, FrameFile(template, frame));
            if (image == null)
            {
                break;
            }

            first ??= image;

            // Guards Texture2DArray.CreateFromImages, which requires every layer to share format and
            // size: a frame that disagrees with the first is dropped in favour of repeating the first,
            // rather than failing the whole array.
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
