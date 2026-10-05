using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Finds or creates everything one project map needs to hold imported ADT tiles: the map itself (from
/// a user-supplied name), its ADT landscape settings, the <c>adt_height</c> and <c>adt_alpha</c>
/// channels, the terrain and 4 texture-slot layers, the height-only material, the map-wide height and
/// alpha <see cref="PaintImage"/>s with their feeding <c>ImageComponent</c> placements, and — on
/// demand — one material per <c>(texture, slot)</c> pair. Idempotent: a second import into the same
/// map reuses what the first created, and each image's storage kind is settled by whichever import
/// made it.
/// </summary>
public sealed class AdtImportCatalog
{
    /// <summary>The <c>adt_height</c> channel resolution — texels per chunk edge, four per cell so every
    /// corner and cell-centre vertex reads back exactly (see <see cref="CellCentreTexels"/>).</summary>
    public const int HeightResolution = HeightMapper.TexelEdge;

    /// <summary>The <c>adt_alpha</c> channel resolution — 1:1 with an MCAL texel.</summary>
    public const int AlphaResolution = TextureMapper.AlphaEdge;

    /// <summary>The <c>adt_holes</c> channel resolution — 1:1 with MCNK's 4×4 hole grid.</summary>
    public const int HoleResolution = HoleMapper.HoleEdge;

    /// <summary>The <c>adt_vcolor</c> / <c>adt_vlight</c> channel resolution — vertex-sampled, so the
    /// same layout as heights.</summary>
    public const int VertexResolution = VertexColorMapper.TexelEdge;

    /// <summary>The <c>adt_area</c> / <c>adt_flags</c> channel resolution — one texel per MCNK,
    /// carrying an MCNK header scalar the terrain-attribute writes read.</summary>
    public const int ChunkResolution = 1;

    /// <summary>Height texels per ADT tile edge — <see cref="HeightResolution"/> across 16 chunks.</summary>
    public const int HeightPxPerTile = HeightResolution * WowTileAddressing.ChunksPerTileEdge;

    /// <summary><c>adt_area</c> / <c>adt_flags</c> texels per ADT tile edge — one per MCNK.</summary>
    public const int ChunkPxPerTile = ChunkResolution * WowTileAddressing.ChunksPerTileEdge;

    /// <summary>Alpha texels per ADT tile edge.</summary>
    public const int AlphaPxPerTile = AlphaResolution * WowTileAddressing.ChunksPerTileEdge;

    /// <summary>Hole texels per ADT tile edge.</summary>
    public const int HolePxPerTile = HoleResolution * WowTileAddressing.ChunksPerTileEdge;

    /// <summary>Vertex-colour / light texels per ADT tile edge.</summary>
    public const int VertexPxPerTile = VertexResolution * WowTileAddressing.ChunksPerTileEdge;

    public const string HeightChannelName = "adt_height";
    public const string AlphaChannelName = "adt_alpha";
    private const string HoleChannelName = "adt_holes";
    public const string VColorChannelName = "adt_vcolor";
    public const string VLightChannelName = "adt_vlight";
    private const string AreaChannelName = "adt_area";
    private const string FlagsChannelName = "adt_flags";
    private const string HeightLayerName = "ADT Terrain";
    private const string HeightMaterialName = "ADT Height";
    private const string HoleLayerName = "ADT Holes";
    private const string VColorLayerName = "ADT VColor";
    private const string VLightLayerName = "ADT VLight";
    private const string ChunkLayerName = "ADT Chunk Data";
    private const string HeightFunctionId = "builtin.height.channel_offset";
    private const string AlphaFunctionId = "builtin.alpha.channel_mask";
    private const string HoleFunctionId = "builtin.hole.channel_mask";
    private const string VColorFunctionId = "builtin.vertex_color.channel_paint";
    private const string VLightFunctionId = "builtin.vertex_light.channel_paint";
    private const string AttributeChannelValueId = "builtin.attribute.channel_value";
    private static readonly string[] SlotLayerNames = ["ADT Base", "ADT Layer 1", "ADT Layer 2", "ADT Layer 3"];

    /// <summary>Which <c>adt_alpha</c> component carries each non-base slot's coverage.</summary>
    private static readonly string[] SlotChannelSwizzle = ["", "r", "g", "b"];

    public required MapId Map { get; init; }

    public required LandscapeSettings Settings { get; init; }

    public required int HeightChannelId { get; init; }

    public required int AlphaChannelId { get; init; }

    public required int HoleChannelId { get; init; }

    public required int HeightLayerId { get; init; }

    public required int HeightMaterialId { get; init; }

    public required int HoleLayerId { get; init; }

    public required int HoleMaterialId { get; init; }

    public required int AreaChannelId { get; init; }

    public required int FlagsChannelId { get; init; }

    public required int ChunkLayerId { get; init; }

    public required int ChunkMaterialId { get; init; }

    public required int AreaImageId { get; init; }

    public required int FlagsImageId { get; init; }

    public required int ChunkImageChunkSize { get; init; }

    public required int VertexColorLayerId { get; init; }

    public required int VertexColorMaterialId { get; init; }

    public required int VertexLightLayerId { get; init; }

    public required int VertexLightMaterialId { get; init; }

    public required int HeightImageId { get; init; }

    public required int HeightImageChunkSize { get; init; }

    public required int AlphaImageId { get; init; }

    public required int AlphaImageChunkSize { get; init; }

    public required int HoleImageId { get; init; }

    public required int HoleImageChunkSize { get; init; }

    public required int VertexColorImageId { get; init; }

    public required int VertexLightImageId { get; init; }

    public required int VertexImageChunkSize { get; init; }

    /// <summary>The 4 texture-slot layer ids, indexed by MCNK slot.</summary>
    public required int[] SlotLayerIds { get; init; }

    public static async Task<AdtImportCatalog> EnsureAsync(
        BatchContext context,
        WorkContext work,
        string mapName,
        string sourceMap,
        PaintImageStorageKind storageKind)
    {
        await work.SwitchToMain();

        EditorContext editor = context.Editor;
        Map map = FindOrCreateMap(editor, mapName);
        await BindToSourceAsync(context, editor, map, sourceMap).ConfigureAwait(true);
        LandscapeSettings settings = await EnsureSettingsAsync(editor, map.Id).ConfigureAwait(true);

        // Import writes settings directly rather than through LandscapeSystem.Enable, so seed the
        // profile's terrain attributes (wow.area, wow.mcnk_flags) here too — otherwise the material
        // writes below bind keys the map never declares and nothing evaluates them. Idempotent.
        editor.Landscape.SeedAttributes(map.Id, new WowAdtProfile(editor.Landscape));

        var toSave = new List<IEntity>();

        LandscapeChannel heightChannel = FindOrCreate(editor, map.Id, HeightChannelName,
            () => new LandscapeChannel { Map = map.Id, Name = HeightChannelName, Resolution = HeightResolution, BitDepth = 32, Components = 1 }, toSave);

        LandscapeChannel alphaChannel = FindOrCreate(editor, map.Id, AlphaChannelName,
            () => new LandscapeChannel { Map = map.Id, Name = AlphaChannelName, Resolution = AlphaResolution, BitDepth = 8, Components = 4 }, toSave);

        LandscapeChannel holeChannel = FindOrCreate(editor, map.Id, HoleChannelName,
            () => new LandscapeChannel { Map = map.Id, Name = HoleChannelName, Resolution = HoleResolution, BitDepth = 8, Components = 1 }, toSave);

        LandscapeChannel vColorChannel = FindOrCreate(editor, map.Id, VColorChannelName,
            () => new LandscapeChannel { Map = map.Id, Name = VColorChannelName, Resolution = VertexResolution, BitDepth = 8, Components = 4 }, toSave);

        LandscapeChannel vLightChannel = FindOrCreate(editor, map.Id, VLightChannelName,
            () => new LandscapeChannel { Map = map.Id, Name = VLightChannelName, Resolution = VertexResolution, BitDepth = 8, Components = 4 }, toSave);

        foreach ((LandscapeChannel channel, int expected) in new[]
        {
            (heightChannel, HeightResolution), (vColorChannel, VertexResolution), (vLightChannel, VertexResolution),
        })
        {
            if (channel.Resolution != expected)
            {
                throw new InvalidOperationException(
                    $"Map '{mapName}' was imported with {channel.Resolution} texels per chunk in '{channel.Name}', " +
                    $"but cell-centre vertices need {expected}. Import into a fresh map.");
            }
        }

        // One scalar 32-bit texel per MCNK each. Two separate channels rather than one wide one: an
        // ImageComponent feeding a multi-component channel goes through a colour path that clamps to
        // [0,1] (it is built for alpha), which would crush an area id — a scalar channel takes the
        // raw value.
        LandscapeChannel areaChannel = FindOrCreate(editor, map.Id, AreaChannelName,
            () => new LandscapeChannel { Map = map.Id, Name = AreaChannelName, Resolution = ChunkResolution, BitDepth = 32, Components = 1 }, toSave);

        LandscapeChannel flagsChannel = FindOrCreate(editor, map.Id, FlagsChannelName,
            () => new LandscapeChannel { Map = map.Id, Name = FlagsChannelName, Resolution = ChunkResolution, BitDepth = 32, Components = 1 }, toSave);

        LandscapeLayer heightLayer = FindOrCreate(editor, map.Id, HeightLayerName,
            () => new LandscapeLayer { Map = map.Id, Name = HeightLayerName, DrawOrder = 0, IsBase = false }, toSave);

        LandscapeLayer holeLayer = FindOrCreate(editor, map.Id, HoleLayerName,
            () => new LandscapeLayer { Map = map.Id, Name = HoleLayerName, DrawOrder = 5, IsBase = false }, toSave);

        LandscapeLayer vColorLayer = FindOrCreate(editor, map.Id, VColorLayerName,
            () => new LandscapeLayer { Map = map.Id, Name = VColorLayerName, DrawOrder = 6, IsBase = false }, toSave);

        LandscapeLayer vLightLayer = FindOrCreate(editor, map.Id, VLightLayerName,
            () => new LandscapeLayer { Map = map.Id, Name = VLightLayerName, DrawOrder = 7, IsBase = false }, toSave);

        LandscapeLayer chunkLayer = FindOrCreate(editor, map.Id, ChunkLayerName,
            () => new LandscapeLayer { Map = map.Id, Name = ChunkLayerName, DrawOrder = 8, IsBase = false }, toSave);

        var slotLayers = new int[SlotLayerNames.Length];
        for (int slot = 0; slot < SlotLayerNames.Length; slot++)
        {
            int drawOrder = slot + 1;
            bool isBase = slot == 0;
            LandscapeLayer layer = FindOrCreate(editor, map.Id, SlotLayerNames[slot],
                () => new LandscapeLayer { Map = map.Id, Name = SlotLayerNames[slot], DrawOrder = drawOrder, IsBase = isBase }, toSave);
            slotLayers[slot] = layer.RecordId ?? 0;
        }

        var heightParameters = new LandscapeParameterValues();
        heightParameters.Set(ChannelHeightOffset.Mask, HeightChannelName);
        heightParameters.Set(ChannelHeightOffset.Amount, 1.0f);
        LandscapeMaterial heightMaterial = FindOrCreate(editor, map.Id, HeightMaterialName,
            () => new LandscapeMaterial
            {
                Map = map.Id,
                Name = HeightMaterialName,
                HeightFunction = HeightFunctionId,
                HeightParameters = heightParameters.Serialize(),
            }, toSave);

        var holeParameters = new LandscapeParameterValues();
        holeParameters.Set(ChannelMaskHole.Mask, HoleChannelName);
        holeParameters.Set(ChannelMaskHole.Threshold, 0.5f);
        LandscapeMaterial holeMaterial = FindOrCreate(editor, map.Id, HoleLayerName,
            () => new LandscapeMaterial
            {
                Map = map.Id,
                Name = HoleLayerName,
                HoleFunction = HoleFunctionId,
                HoleParameters = holeParameters.Serialize(),
            }, toSave);

        (PaintImage heightImage, int heightChunkSize) = FindOrCreateImage(editor, $"ADT Heights ({mapName})",
            HeightPxPerTile, 1, PaintImagePixelFormat.Float32, storageKind, context.ProjectFolder, "heights", mapName, "{x}_{y}.exr", toSave);

        (PaintImage alphaImage, int alphaChunkSize) = FindOrCreateImage(editor, $"ADT Alpha ({mapName})",
            AlphaPxPerTile, 4, PaintImagePixelFormat.Byte, storageKind, context.ProjectFolder, "alpha", mapName, "{x}_{y}.png", toSave);

        var vColorParameters = new LandscapeParameterValues();
        vColorParameters.Set(ChannelVertexColorPaint.Source, VColorChannelName);
        vColorParameters.Set(ChannelVertexColorPaint.Strength, 1.0f);
        vColorParameters.Set(ChannelVertexColorPaint.Exponent, 2.0f);
        vColorParameters.Set(ChannelVertexColorPaint.Scale, 4.0f);
        LandscapeMaterial vColorMaterial = FindOrCreate(editor, map.Id, VColorLayerName,
            () => new LandscapeMaterial
            {
                Map = map.Id,
                Name = VColorLayerName,
                VertexColorFunction = VColorFunctionId,
                VertexColorParameters = vColorParameters.Serialize(),
            }, toSave);

        // FindOrCreate returns an existing material untouched, so a map imported before the raw-MCCV
        // fix would keep Exponent/Scale at their function defaults (1, 1) and render the now-raw bytes
        // as colour at roughly half brightness. Re-apply just these two parameters rather than leave a
        // re-imported map stuck decoding nothing.
        LandscapeParameterValues existingVColorParameters = LandscapeParameterValues.Parse(vColorMaterial.VertexColorParameters);
        existingVColorParameters.Set(ChannelVertexColorPaint.Exponent, 2.0f);
        existingVColorParameters.Set(ChannelVertexColorPaint.Scale, 4.0f);
        string repairedVColorParameters = existingVColorParameters.Serialize();
        if (vColorMaterial.VertexColorParameters != repairedVColorParameters)
        {
            vColorMaterial.VertexColorParameters = repairedVColorParameters;
            toSave.Add(vColorMaterial);
        }

        var vLightParameters = new LandscapeParameterValues();
        vLightParameters.Set(ChannelVertexLightPaint.Source, VLightChannelName);
        vLightParameters.Set(ChannelVertexLightPaint.Amount, 1.0f);
        LandscapeMaterial vLightMaterial = FindOrCreate(editor, map.Id, VLightLayerName,
            () => new LandscapeMaterial
            {
                Map = map.Id,
                Name = VLightLayerName,
                VertexLightFunction = VLightFunctionId,
                VertexLightParameters = vLightParameters.Serialize(),
            }, toSave);

        (PaintImage holeImage, int holeChunkSize) = FindOrCreateImage(editor, $"ADT Holes ({mapName})",
            HolePxPerTile, 1, PaintImagePixelFormat.Byte, storageKind, context.ProjectFolder, "holes", mapName, "{x}_{y}.png", toSave);

        // A function-less material: it carries only terrain-attribute writes. One reads adt_area into
        // wow.area, the other adt_flags into wow.mcnk_flags — both through the nearest-texel
        // channel_value function, since a filtered id is a wrong id.
        LandscapeMaterial chunkMaterial = FindOrCreate(editor, map.Id, ChunkLayerName,
            () => new LandscapeMaterial { Map = map.Id, Name = ChunkLayerName }, toSave);
        int chunkMaterialId = chunkMaterial.RecordId ?? 0;
        FindOrCreateAttributeWrite(editor, map.Id, chunkMaterialId, WowAdtProfile.AreaAttributeKey,
            AreaChannelName, toSave);
        FindOrCreateAttributeWrite(editor, map.Id, chunkMaterialId, WowAdtProfile.FlagsAttributeKey,
            FlagsChannelName, toSave);

        (PaintImage areaImage, int chunkImageChunkSize) = FindOrCreateImage(editor, $"ADT Area ({mapName})",
            ChunkPxPerTile, 1, PaintImagePixelFormat.Float32, storageKind, context.ProjectFolder, "area", mapName, "{x}_{y}.exr", toSave);

        (PaintImage flagsImage, int _) = FindOrCreateImage(editor, $"ADT MCNK Flags ({mapName})",
            ChunkPxPerTile, 1, PaintImagePixelFormat.Float32, storageKind, context.ProjectFolder, "mcnkflags", mapName, "{x}_{y}.exr", toSave);

        (PaintImage vColorImage, int vertexChunkSize) = FindOrCreateImage(editor, $"ADT VColor ({mapName})",
            VertexPxPerTile, 4, PaintImagePixelFormat.Byte, storageKind, context.ProjectFolder, "vcolor", mapName, "{x}_{y}.png", toSave);

        (PaintImage vLightImage, int _) = FindOrCreateImage(editor, $"ADT VLight ({mapName})",
            VertexPxPerTile, 4, PaintImagePixelFormat.Byte, storageKind, context.ProjectFolder, "vlight", mapName, "{x}_{y}.png", toSave);

        EditorStorage storage = editor.Database.EditorStorage;
        if (toSave.Count > 0)
        {
            await storage.CommitAsync(toSave, []).ConfigureAwait(true);
        }

        float footprint = WowTileAddressing.TilesPerMapEdge * WowTileAddressing.ChunksPerTileEdge * settings.ChunkWorldSize;
        await EnsurePlacementEntityAsync(editor, storage, map.Id, "ADT Heights", heightImage, HeightChannelName, ImageWriteMode.Replace, footprint).ConfigureAwait(true);
        await EnsurePlacementEntityAsync(editor, storage, map.Id, "ADT Alpha", alphaImage, AlphaChannelName, ImageWriteMode.Max, footprint).ConfigureAwait(true);
        await EnsurePlacementEntityAsync(editor, storage, map.Id, "ADT Holes", holeImage, HoleChannelName, ImageWriteMode.Max, footprint).ConfigureAwait(true);
        await EnsurePlacementEntityAsync(editor, storage, map.Id, "ADT VColor", vColorImage, VColorChannelName, ImageWriteMode.Max, footprint).ConfigureAwait(true);
        await EnsurePlacementEntityAsync(editor, storage, map.Id, "ADT VLight", vLightImage, VLightChannelName, ImageWriteMode.Max, footprint).ConfigureAwait(true);
        await EnsurePlacementEntityAsync(editor, storage, map.Id, "ADT Area", areaImage, AreaChannelName, ImageWriteMode.Replace, footprint).ConfigureAwait(true);
        await EnsurePlacementEntityAsync(editor, storage, map.Id, "ADT MCNK Flags", flagsImage, FlagsChannelName, ImageWriteMode.Replace, footprint).ConfigureAwait(true);

        var result = new AdtImportCatalog
        {
            Map = map.Id,
            Settings = settings,
            HeightChannelId = heightChannel.RecordId ?? 0,
            AlphaChannelId = alphaChannel.RecordId ?? 0,
            HoleChannelId = holeChannel.RecordId ?? 0,
            HeightLayerId = heightLayer.RecordId ?? 0,
            HeightMaterialId = heightMaterial.RecordId ?? 0,
            HoleLayerId = holeLayer.RecordId ?? 0,
            HoleMaterialId = holeMaterial.RecordId ?? 0,
            AreaChannelId = areaChannel.RecordId ?? 0,
            FlagsChannelId = flagsChannel.RecordId ?? 0,
            ChunkLayerId = chunkLayer.RecordId ?? 0,
            ChunkMaterialId = chunkMaterialId,
            AreaImageId = areaImage.RecordId ?? 0,
            FlagsImageId = flagsImage.RecordId ?? 0,
            ChunkImageChunkSize = chunkImageChunkSize,
            VertexColorLayerId = vColorLayer.RecordId ?? 0,
            VertexColorMaterialId = vColorMaterial.RecordId ?? 0,
            VertexLightLayerId = vLightLayer.RecordId ?? 0,
            VertexLightMaterialId = vLightMaterial.RecordId ?? 0,
            HeightImageId = heightImage.RecordId ?? 0,
            HeightImageChunkSize = heightChunkSize,
            AlphaImageId = alphaImage.RecordId ?? 0,
            AlphaImageChunkSize = alphaChunkSize,
            HoleImageId = holeImage.RecordId ?? 0,
            HoleImageChunkSize = holeChunkSize,
            VertexColorImageId = vColorImage.RecordId ?? 0,
            VertexLightImageId = vLightImage.RecordId ?? 0,
            VertexImageChunkSize = vertexChunkSize,
            SlotLayerIds = slotLayers,
        };

        await work.SwitchToBackground();
        return result;
    }

    /// <summary>The name of the material that puts <paramref name="texture"/> in MCNK slot <paramref name="slot"/>.</summary>
    public static string SlotMaterialName(string texture, int slot) => $"ADT {slot}: {texture}";

    /// <summary>Which <c>(texture, slot)</c> a material name from <see cref="SlotMaterialName"/> stands for.</summary>
    public static bool TryParseSlotMaterialName(string name, out string texture, out int slot)
    {
        texture = "";
        slot = -1;
        if (!name.StartsWith("ADT ", StringComparison.Ordinal))
        {
            return false;
        }

        int colon = name.IndexOf(": ", StringComparison.Ordinal);
        if (colon < 5 || !int.TryParse(name.AsSpan(4, colon - 4), out slot) || slot is < 0 or >= AdtTileComponent.SlotCount)
        {
            return false;
        }

        texture = name[(colon + 2)..];
        return true;
    }

    /// <summary>The alpha function and parameters a slot's material carries: none for the opaque base, the
    /// identity <c>channel_mask</c> over <c>adt_alpha</c>'s R/G/B for slots 1-3.</summary>
    public static (string Function, string Parameters) SlotAlpha(int slot)
    {
        if (slot is not (> 0 and < 4))
        {
            return ("", "");
        }

        var alpha = new LandscapeParameterValues();
        // ":" is the swizzle separator — a channel name can legally contain ".".
        alpha.Set(ChannelMaskAlpha.Mask, $"{AlphaChannelName}:{SlotChannelSwizzle[slot]}");
        alpha.Set(ChannelMaskAlpha.Threshold, 0.5f);
        alpha.Set(ChannelMaskAlpha.Softness, 1.0f);
        return (AlphaFunctionId, alpha.Serialize());
    }

    /// <summary>
    /// One <see cref="LandscapeMaterial"/> per <c>(texturePath, slot)</c> pair — the base slot opaque,
    /// slots 1-3 bound to <c>adt_alpha</c>'s R/G/B through the identity <c>channel_mask</c> remap
    /// (threshold 0.5, softness 1.0). Called per tile as its MTEX is read; after the first few tiles it
    /// finds everything and creates nothing. Returns a lookup for every requested pair.
    /// </summary>
    public static async Task<IReadOnlyDictionary<(string Texture, int Slot), int>> EnsureSlotMaterialsAsync(
        BatchContext context,
        WorkContext work,
        MapId map,
        IReadOnlyCollection<(string Texture, int Slot, int EffectId)> requests)
    {
        EditorContext editor = context.Editor;

        // One scan of the loaded catalog instead of a FirstOrDefault per requested pair — after the
        // first few tiles every pair already exists, so this dictionary build is the only catalog scan
        // this call makes.
        var byName = new Dictionary<string, LandscapeMaterial>(StringComparer.Ordinal);
        foreach (LandscapeMaterial material in editor.Catalog.OfType<LandscapeMaterial>())
        {
            if (material.Map.Equals(map))
            {
                byName[material.DisplayName] = material;
            }
        }

        // Collapse to one entry per (texture, slot); a texture with different effect ids across chunks
        // keeps the first non-zero one it was seen with.
        var byPair = requests
            .GroupBy(request => (request.Texture, request.Slot))
            .Select(group => (group.Key.Texture, group.Key.Slot,
                EffectId: group.Select(request => request.EffectId).FirstOrDefault(id => id != 0)));

        var toCreate = new List<(string Texture, int Slot, int EffectId, string Name, string AlphaFunction, string AlphaParameters)>();
        var toRepair = new List<(LandscapeMaterial Material, string TexturePath, string AlphaFunction, string AlphaParameters, int EffectId)>();
        var result = new Dictionary<(string, int), int>();

        foreach ((string texture, int slot, int effectId) in byPair)
        {
            string name = SlotMaterialName(texture, slot);
            (string wantedAlphaFunction, string wantedAlphaParameters) = SlotAlpha(slot);

            if (!byName.TryGetValue(name, out LandscapeMaterial? existing))
            {
                toCreate.Add((texture, slot, effectId, name, wantedAlphaFunction, wantedAlphaParameters));
                continue;
            }

            if (existing.AlphaFunction != wantedAlphaFunction || existing.AlphaParameters != wantedAlphaParameters
                || existing.TexturePath != texture || (effectId != 0 && existing.SurfaceEffectId != effectId))
            {
                // Repair a material from an earlier importer version (e.g. one with a stale channel
                // binding) rather than leave it broken. A zero effect id never overwrites a set one.
                toRepair.Add((existing, texture, wantedAlphaFunction, wantedAlphaParameters, effectId));
            }

            result[(texture, slot)] = existing.RecordId ?? 0;
        }

        if (toCreate.Count == 0 && toRepair.Count == 0)
        {
            return result;
        }

        await work.SwitchToMain();

        var toSave = new List<IEntity>();
        foreach ((string texture, int slot, int effectId, string name, string wantedAlphaFunction, string wantedAlphaParameters) in toCreate)
        {
            var created = new LandscapeMaterial
            {
                Map = map,
                Name = name,
                TexturePath = texture,
                AlphaFunction = wantedAlphaFunction,
                AlphaParameters = wantedAlphaParameters,
                SurfaceEffectId = effectId,
            };
            editor.Catalog.AssignId(created);
            editor.Catalog.Add(created);
            toSave.Add(created);
            result[(texture, slot)] = created.RecordId ?? 0;
        }

        foreach ((LandscapeMaterial existing, string texturePath, string wantedAlphaFunction, string wantedAlphaParameters, int effectId) in toRepair)
        {
            existing.TexturePath = texturePath;
            existing.AlphaFunction = wantedAlphaFunction;
            existing.AlphaParameters = wantedAlphaParameters;
            if (effectId != 0)
            {
                existing.SurfaceEffectId = effectId;
            }

            toSave.Add(existing);
        }

        await editor.Database.EditorStorage.CommitAsync(toSave, []).ConfigureAwait(true);
        await work.SwitchToBackground();
        return result;
    }

    private static Map FindOrCreateMap(EditorContext editor, string mapName)
    {
        Map? existing = editor.Maps.Maps.FirstOrDefault(map =>
            string.Equals(map.DisplayName, mapName, StringComparison.OrdinalIgnoreCase));
        if (existing != null)
        {
            return existing;
        }

        Map? created = editor.Maps.Create(editor.Maps.NextFreeId(), mapName, out string? error);
        if (created == null)
        {
            throw new InvalidOperationException($"Could not create map '{mapName}': {error}");
        }

        return created;
    }

    /// <summary>
    /// Binds the target project map to the WoW map <paramref name="sourceMap"/> (the client folder
    /// imported from) resolves to, if it isn't bound already — so a freshly imported map has WoW data
    /// (spawns, lights, area table, …) without an extra manual step. Leaves an existing binding to a
    /// different WoW map alone rather than overwriting it; logs either way.
    /// </summary>
    private static async Task BindToSourceAsync(BatchContext context, EditorContext editor, Map map, string sourceMap)
    {
        DbcMap? source = editor.Catalog.OfType<DbcMap>().FirstOrDefault(candidate =>
            string.Equals(candidate.Directory, sourceMap, StringComparison.OrdinalIgnoreCase));
        if (source == null)
        {
            context.Log($"[WoW] '{sourceMap}' has no matching dbc_map row — target map '{map.DisplayName}' was not bound to a WoW map id.");
            return;
        }

        WowMapBindings bindings = WowMapBindings.For(editor);
        int? existing = bindings.Resolve(map.Id);
        if (existing == null)
        {
            await bindings.SetAsync(map.Id, source.MapId).ConfigureAwait(true);
            context.Log($"[WoW] Bound target map '{map.DisplayName}' (id {map.Id.Value}) to WoW map {source.MapId} ('{sourceMap}').");
        }
        else if (existing != source.MapId)
        {
            context.Log($"[WoW] Target map '{map.DisplayName}' is already bound to WoW map {existing}, not {source.MapId} ('{sourceMap}') — leaving it alone.");
        }
    }

    private static async Task<LandscapeSettings> EnsureSettingsAsync(EditorContext editor, MapId map)
    {
        if (editor.Landscape.LoadSettingsFor(map) is { } existing)
        {
            return existing;
        }

        LandscapeSettings settings = new WowAdtProfile(editor.Landscape).CreateSettings();
        if (editor.Database.Storages.SelectMany(storage => storage.LandscapeSettingsSources).FirstOrDefault(source => source.CanEdit)
            is not { } sink)
        {
            throw new InvalidOperationException("No storage accepts landscape settings.");
        }

        await sink.SaveAsync(map, settings).ConfigureAwait(true);
        return settings;
    }

    private static TEntity FindOrCreate<TEntity>(
        EditorContext editor,
        MapId map,
        string name,
        Func<TEntity> create,
        List<IEntity> toSave)
        where TEntity : CatalogEntity, IKeyedCatalogEntity, ILandscapeCatalogEntity
    {
        TEntity? existing = editor.Catalog.OfType<TEntity>()
            .FirstOrDefault(entity => entity.Map.Equals(map) && string.Equals(entity.DisplayName, name, StringComparison.Ordinal));
        if (existing != null)
        {
            return existing;
        }

        TEntity created = create();
        editor.Catalog.AssignId(created);
        editor.Catalog.Add(created);
        toSave.Add(created);
        return created;
    }

    /// <summary>Finds (by owning material and target attribute key) or creates one attribute write
    /// bound to <paramref name="channelBinding"/> through <c>builtin.attribute.channel_value</c>,
    /// repairing a stale binding from an earlier importer version rather than leaving it broken.</summary>
    private static void FindOrCreateAttributeWrite(
        EditorContext editor,
        MapId map,
        int materialId,
        string attributeKey,
        string channelBinding,
        List<IEntity> toSave)
    {
        var parameters = new LandscapeParameterValues();
        parameters.Set(AttributeChannelValue.Source, channelBinding);
        string wantedParameters = parameters.Serialize();

        LandscapeMaterialAttributeWrite? existing = editor.Catalog.OfType<LandscapeMaterialAttributeWrite>()
            .FirstOrDefault(write => write.Map.Equals(map) && write.MaterialId == materialId
                && write.Binding.Attribute == attributeKey);

        if (existing == null)
        {
            var created = new LandscapeMaterialAttributeWrite
            {
                Map = map,
                MaterialId = materialId,
                Attribute = attributeKey,
                Function = AttributeChannelValueId,
                Parameters = wantedParameters,
            };
            editor.Catalog.AssignId(created);
            editor.Catalog.Add(created);
            toSave.Add(created);
            return;
        }

        if (existing.Function != AttributeChannelValueId || existing.Parameters != wantedParameters)
        {
            existing.Function = AttributeChannelValueId;
            existing.Parameters = wantedParameters;
            toSave.Add(existing);
        }
    }

    /// <summary>The single map-wide <c>ImageComponent</c> placement feeding one channel. Placed at the
    /// world origin with a footprint of the whole 64×64-tile map, so image pixel <c>i</c> lines up with
    /// channel texel <c>i</c> with no offset.</summary>
    private static async Task EnsurePlacementEntityAsync(
        EditorContext editor,
        EditorStorage storage,
        MapId map,
        string entityName,
        PaintImage image,
        string channel,
        ImageWriteMode writeMode,
        float footprint)
    {
        int imageId = image.RecordId ?? 0;

        using (IDisposable read = await storage.Lock.ReaderAsync().ConfigureAwait(true))
        await using (EditorDbContext db = storage.CreateContext())
        {
            // Scoped to this map, not just the image: FindOrCreateImage matches by name alone, so two
            // different project maps that share a display name end up sharing one PaintImage. Without
            // the map check here, the second map's import would see the first map's placement and skip
            // creating its own — leaving that map with no placement at all, so its landscape always
            // samples zero for this channel no matter how much real pixel data actually got written.
            if (await db.Set<SceneImageComponentRecord>().AsNoTracking()
                    .Where(record => record.ImageId == imageId)
                    .Join(db.MapEntities.AsNoTracking(), record => record.EntityId, entity => entity.Id, (record, entity) => entity.MapId)
                    .AnyAsync(mapId => mapId == map.Value).ConfigureAwait(true))
            {
                return;
            }
        }

        var entity = new MapSceneEntity { Name = entityName, Map = map };
        entity.AddComponent(new ImageComponent(editor.Images)
        {
            ImageId = imageId,
            Channel = channel,
            WriteMode = writeMode,
            WorldSizeX = footprint,
            WorldSizeZ = footprint,
        });

        await storage.CommitAsync([entity], []).ConfigureAwait(true);
    }

    private static (PaintImage Image, int ChunkSize) FindOrCreateImage(
        EditorContext editor,
        string imageName,
        int pxPerTile,
        int components,
        PaintImagePixelFormat format,
        PaintImageStorageKind storageKind,
        string projectFolder,
        string diskSubdir,
        string mapName,
        string diskPattern,
        List<IEntity> toSave)
    {
        PaintImage? existing = editor.Catalog.OfType<PaintImage>()
            .FirstOrDefault(image => string.Equals(image.Name, imageName, StringComparison.Ordinal));
        if (existing != null)
        {
            return (existing, existing.ChunkSize);
        }

        int side = pxPerTile * WowTileAddressing.TilesPerMapEdge;

        var image = new PaintImage { Name = imageName };
        image.ConfigureNew(side, side, pxPerTile, components, format);
        if (storageKind == PaintImageStorageKind.Disk)
        {
            image.ConfigureDiskSource(System.IO.Path.Combine(projectFolder, "adt-import", diskSubdir, mapName), diskPattern);
        }

        editor.Catalog.AssignId(image);
        editor.Catalog.Add(image);
        toSave.Add(image);
        return (image, pxPerTile);
    }
}
