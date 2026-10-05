using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Loads WoW <c>.m2</c> static bind-pose geometry (Legion+ chunked format, through <see cref="M2Reader"/>).</summary>
[Subsystem(nameof(AssetSystem))]
public sealed class M2ModelLoader : IModelLoader
{
    /// <summary>Prefix every geoset <see cref="ModelPart"/> this loader emits is named with, e.g.
    /// <c>Geoset:100</c> — the same name-prefix convention <c>WmoModelLoader.DoodadSetPartPrefix</c>
    /// uses for doodad sets, so a caller filtering geosets (<c>CreatureModelComponent</c>) parses this
    /// instead of restating the format.</summary>
    public const string GeosetPartPrefix = "Geoset:";

    public M2ModelLoader(AssetSystem assets)
    {
    }

    public string FormatId => M2Format.FormatId;

    public bool CanLoad(string path) => AssetPath.Extension(path).Equals(".m2", StringComparison.OrdinalIgnoreCase);

    public async Task<ModelAsset?> LoadModelAsync(AssetSystem assets, string path, WorkContext? work)
    {
        byte[]? bytes = await assets.ReadAssetBytesAsync(path).ConfigureAwait(false);
        if (bytes == null)
        {
            return null;
        }

        M2Document? document = M2Reader.Read(bytes, path);
        if (document == null || document.SkinFileDataIds.Length == 0)
        {
            return null;
        }

        string skinPath = SkinPath(path, document);
        byte[]? skinBytes = await assets.ReadAssetBytesAsync(skinPath).ConfigureAwait(false);
        if (skinBytes == null)
        {
            GD.PushWarning($"[WoW M2] '{path}' has no readable skin file '{skinPath}'.");
            return null;
        }

        M2Skin? skin = M2Reader.ReadSkin(skinBytes, skinPath);
        if (skin == null)
        {
            return null;
        }

        if (work != null)
        {
            await work.SwitchToMain();
        }

        List<ModelPart> parts = BuildParts(document, skin);
        if (parts.Count == 0)
        {
            return null;
        }

        List<ModelAttachmentPoint> attachmentPoints = document.Attachments
            .Select(attachment => new ModelAttachmentPoint(attachment.Id, attachment.Position)).ToList();
        return new ModelAsset(path, parts, document.Bounds, FormatId, attachmentPoints);
    }

    // The first (full detail) skin from the model's SFID chunk; falls back to the NN.skin naming
    // convention when the listfile doesn't know the id.
    private static string SkinPath(string path, M2Document document)
    {
        if (WowFileDataIds.Shared.PathOf(document.SkinFileDataIds[0]) is { } resolved)
        {
            return resolved;
        }

        string extension = AssetPath.Extension(path);
        return $"{path[..^extension.Length]}00.skin";
    }

    /// <summary>Groups batch indices by their submesh's geoset id (<see cref="M2SkinSubmesh.Id"/>),
    /// preserving first-seen order so a model with few distinct geosets (most creatures — typically
    /// just id 0) gets a short, stable part list rather than one entry per batch. Internal rather than
    /// private so <c>M2ModelLoaderTests</c> can drive the grouping directly against hand-built
    /// <see cref="M2Skin"/> data, without needing real `.m2`/`.skin` bytes or an
    /// <see cref="AssetSystem"/> the way exercising <see cref="LoadModelAsync"/> end-to-end would.</summary>
    internal static List<(ushort GeosetId, List<int> BatchIndices)> GroupBatchesByGeoset(M2Skin skin)
    {
        var order = new List<ushort>();
        var byGeoset = new Dictionary<ushort, List<int>>();

        for (int i = 0; i < skin.Batches.Length; i++)
        {
            M2SkinBatch batch = skin.Batches[i];
            if (batch.SkinSectionIndex >= skin.Submeshes.Length)
            {
                continue;
            }

            ushort geosetId = skin.Submeshes[batch.SkinSectionIndex].Id;
            if (!byGeoset.TryGetValue(geosetId, out List<int>? indices))
            {
                indices = [];
                byGeoset[geosetId] = indices;
                order.Add(geosetId);
            }

            indices.Add(i);
        }

        var result = new List<(ushort, List<int>)>(order.Count);
        foreach (ushort geosetId in order)
        {
            result.Add((geosetId, byGeoset[geosetId]));
        }

        return result;
    }

    // One ModelPart per distinct geoset id — the seam a caller filters geosets through (the same
    // Doodads:<name>-style name-prefix convention WmoModelLoader uses for doodad sets). A part is the
    // smallest unit ModelInstantiateOptions can independently show or hide, so independently
    // filterable geosets each need their own part. All default-visible, so a caller with no
    // PartFilter renders every geoset.
    private static List<ModelPart> BuildParts(M2Document document, M2Skin skin)
    {
        var parts = new List<ModelPart>();
        foreach ((ushort geosetId, List<int> batchIndices) in GroupBatchesByGeoset(skin))
        {
            if (BuildGeosetSurface(document, skin, batchIndices) is { } surface)
            {
                parts.Add(new ModelPart($"{GeosetPartPrefix}{geosetId}", Transform3D.Identity, [surface], []));
            }
        }

        return parts;
    }

    private static ModelSurface? BuildGeosetSurface(M2Document document, M2Skin skin, List<int> batchIndices)
    {
        var mesh = new ArrayMesh();
        var materials = new List<MeshMaterial>();

        foreach (int i in batchIndices)
        {
            M2SkinBatch batch = skin.Batches[i];
            if (BatchIsRestHidden(document.ColorRestAlpha, batch))
            {
                continue;
            }

            if (!TryAddSubmeshSurface(mesh, document, skin, skin.Submeshes[batch.SkinSectionIndex]))
            {
                continue;
            }

            // Fallback tint index is the batch's own position in the whole model's batch array, not
            // its position within this geoset group — so a plain creature's fallback colours cycle
            // the same palette globally.
            materials.Add(BuildMaterial(document, batch, i));
        }

        return materials.Count == 0 ? null : new ModelSurface("Mesh", mesh, materials);
    }

    /// <summary>A batch bound to a <c>ModelColor</c> block whose opacity is ~0 at rest
    /// (<see cref="M2Document.ColorRestAlpha"/>) is one that is skipped whenever it is not
    /// mid-animation — the blink eyelid and eye-shine meshes on a character model, and assorted
    /// rest-hidden creature overlays. This loader renders one static pose with no animation clock, so
    /// it drops the batch instead of rendering the eyelid permanently shut. Internal so
    /// <c>M2ModelLoaderTests</c> can drive the rule directly.</summary>
    internal static bool BatchIsRestHidden(float[] colorRestAlpha, M2SkinBatch batch) =>
        batch.ColorIndex >= 0
        && batch.ColorIndex < colorRestAlpha.Length
        && colorRestAlpha[batch.ColorIndex] < RestAlphaEpsilon;

    // One 8-bit level of opacity; below this a batch contributes nothing a viewer could see. Tracks
    // resolve to an exact 0 here, not a small positive value.
    private const float RestAlphaEpsilon = 1f / 255f;

    // Adds one skin batch's submesh as a surface of the shared model mesh; returns false (adding
    // nothing) for a submesh whose geometry is malformed.
    private static bool TryAddSubmeshSurface(ArrayMesh mesh, M2Document document, M2Skin skin, M2SkinSubmesh submesh)
    {
        if (submesh.IndexCount == 0 || submesh.IndexCount % 3 != 0)
        {
            return false;
        }

        long indexStart = submesh.IndexStart;
        if (indexStart + submesh.IndexCount > skin.Indices.Length)
        {
            return false;
        }

        var localToBuild = new Dictionary<ushort, int>();
        var positions = new List<Vector3>();
        var normals = new List<Vector3>();
        var uvs = new List<Vector2>();
        var indices = new int[submesh.IndexCount];

        for (int i = 0; i < submesh.IndexCount; i++)
        {
            ushort localVertex = skin.Indices[indexStart + i];
            if (localVertex >= skin.VertexLookup.Length)
            {
                return false;
            }

            ushort globalVertex = skin.VertexLookup[localVertex];
            if (globalVertex >= document.Vertices.Length)
            {
                return false;
            }

            if (!localToBuild.TryGetValue(globalVertex, out int buildIndex))
            {
                buildIndex = positions.Count;
                localToBuild[globalVertex] = buildIndex;
                M2Vertex vertex = document.Vertices[globalVertex];
                positions.Add(vertex.Position);
                normals.Add(vertex.Normal);
                uvs.Add(vertex.Uv);
            }

            indices[i] = buildIndex;
        }

        if (positions.Count == 0)
        {
            return false;
        }

        // WoW triangles are clockwise-front-facing (D3D convention); the coordinate conversion is a
        // proper rotation and does not flip this, so rewind explicitly for Godot's CCW front face.
        for (int i = 0; i + 2 < indices.Length; i += 3)
        {
            (indices[i + 1], indices[i + 2]) = (indices[i + 2], indices[i + 1]);
        }

        var arrays = new Godot.Collections.Array();
        arrays.Resize((int)Mesh.ArrayType.Max);
        arrays[(int)Mesh.ArrayType.Vertex] = positions.ToArray();
        arrays[(int)Mesh.ArrayType.Normal] = normals.ToArray();
        arrays[(int)Mesh.ArrayType.TexUV] = uvs.ToArray();
        arrays[(int)Mesh.ArrayType.Index] = indices;

        mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, arrays);
        return true;
    }

    private static MeshMaterial BuildMaterial(M2Document document, M2SkinBatch batch, int fallbackIndex)
    {
        (string texturePath, uint textureType) = ResolveTexture(document, batch.TextureComboIndex);
        M2MaterialDef? material = batch.MaterialIndex < document.Materials.Length ? document.Materials[batch.MaterialIndex] : null;

        return M2Material.Describe(
            texture: texturePath,
            tint: texturePath.Length == 0 ? ModelAsset.FallbackColor(fallbackIndex) : Colors.White,
            blend: WowBlendModes.FromRaw(material?.Blend ?? 0),
            twoSided: material?.Flags.TwoSided ?? false,
            unlit: material?.Flags.Unlit ?? false,
            depthWrite: material?.Flags.DepthWrite ?? true,
            textureSlot: textureType);
    }

    // Returns the type alongside the path (rather than a second lookup) because a replaceable slot's
    // texture has no name to resolve at all — the type is the only thing distinguishing "this batch
    // wants creature skin slot 1" from "this batch has no texture" once the name comes back empty.
    private static (string Path, uint Type) ResolveTexture(M2Document document, ushort textureComboIndex)
    {
        if (textureComboIndex >= document.TextureLookup.Length)
        {
            return ("", 0);
        }

        ushort textureIndex = document.TextureLookup[textureComboIndex];
        if (textureIndex >= document.Textures.Length)
        {
            return ("", 0);
        }

        M2TextureDef texture = document.Textures[textureIndex];
        string name = texture.Name.Length > 0 ? texture.Name : WowFileDataIds.Shared.PathOf(texture.FileDataId) ?? "";
        return (name, texture.Type);
    }
}
