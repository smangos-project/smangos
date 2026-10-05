using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Loads WoW <c>.wmo</c> static group geometry and doodad sets (see <see cref="WmoRootReader"/>).</summary>
[Subsystem(nameof(AssetSystem))]
public sealed class WmoModelLoader : IModelLoader
{
    /// <summary>
    /// Name prefix identifying a <see cref="ModelPart"/> as one WMO doodad set (consumed by the
    /// doodad-set picker in <c>ModelRendererComponent.Wmo.cs</c>); the part name after the prefix is
    /// the set's own display name.
    /// </summary>
    public const string DoodadSetPartPrefix = "Doodads:";

    public WmoModelLoader(AssetSystem assets)
    {
    }

    public float Priority => -5f;

    public string FormatId => WmoFormat.FormatId;

    public bool CanLoad(string path) => AssetPath.Extension(path).Equals(".wmo", StringComparison.OrdinalIgnoreCase);

    /// <summary>Excludes group files (<c>..._NNN.wmo</c>) from listings; they are never standalone models.</summary>
    public bool CanList(string path) => CanLoad(path) && !IsGroupFileName(path);

    public async Task<ModelAsset?> LoadModelAsync(AssetSystem assets, string path, WorkContext? work)
    {
        byte[]? rootBytes = await assets.ReadAssetBytesAsync(path).ConfigureAwait(false);
        if (rootBytes == null)
        {
            return null;
        }

        WmoRoot? root = WmoRootReader.Read(rootBytes, path);
        if (root == null || root.GroupCount == 0)
        {
            return null;
        }

        string extension = AssetPath.Extension(path);
        string basePath = path[..^extension.Length];
        var parts = new List<ModelPart>();

        for (int i = 0; i < root.GroupCount; i++)
        {
            string groupPath = $"{basePath}_{i:000}{extension}";
            byte[]? groupBytes = await assets.ReadAssetBytesAsync(groupPath).ConfigureAwait(false);
            if (groupBytes == null)
            {
                GD.PushWarning($"[WoW WMO] '{path}' is missing group file '{groupPath}'.");
                continue;
            }

            WmoGroup? group = WmoGroupReader.Read(groupBytes, root.Lighting);
            if (group == null)
            {
                GD.PushWarning($"[WoW WMO] '{path}' has a malformed group file '{groupPath}'.");
                continue;
            }

            if (work != null)
            {
                await work.SwitchToMain();
            }

            List<ModelSurface> surfaces = BuildSurfaces(root, group, i);

            if (work != null)
            {
                await work.SwitchToBackground();
            }

            if (surfaces.Count > 0)
            {
                parts.Add(new ModelPart($"Group{i}", Transform3D.Identity, surfaces, []));
            }
        }

        // Set 0 is the WMO's always-visible global doodad set; every set is emitted so the doodad-set
        // picker can offer them, but only set 0 defaults to visible (ModelAsset.Instantiate falls back
        // to ModelPart.DefaultVisible when nothing overrides it via ConfigurePartFilter).
        for (int i = 0; i < root.DoodadSets.Length; i++)
        {
            if (BuildDoodadSetPart(root.DoodadSets[i], i, defaultVisible: i == 0) is { } doodads)
            {
                parts.Add(doodads);
            }
        }

        return parts.Count == 0 ? null : new ModelAsset(path, parts, root.Bounds, FormatId);
    }

    private static ModelPart? BuildDoodadSetPart(WmoDoodadSet set, int index, bool defaultVisible)
    {
        var references = new List<ModelReference>();
        foreach (WmoDoodadPlacement placement in set.Placements)
        {
            if (placement.Path.Length > 0)
            {
                references.Add(new ModelReference(placement.Path, placement.Path, placement.Transform));
            }
        }

        if (references.Count == 0)
        {
            return null;
        }

        string displayName = set.Name.Length > 0 ? set.Name : $"Set {index}";
        return new ModelPart($"{DoodadSetPartPrefix}{displayName}", Transform3D.Identity, [], references, defaultVisible);
    }

    private static bool IsGroupFileName(string path)
    {
        string extension = AssetPath.Extension(path);
        string name = AssetPath.FileName(path);
        string stem = name[..^extension.Length];
        if (stem.Length < 4 || stem[^4] != '_')
        {
            return false;
        }

        for (int i = stem.Length - 3; i < stem.Length; i++)
        {
            if (!char.IsAsciiDigit(stem[i]))
            {
                return false;
            }
        }

        return true;
    }

    // One shared ArrayMesh per group, one mesh surface per batch, rather than a separate ArrayMesh (and
    // therefore a separate MeshInstance3D) per batch: a group typically holds anywhere from a handful
    // to a few dozen batches, and a large WMO can have well over a hundred groups — building and
    // instantiating one node per batch multiplied that into thousands of MeshInstance3Ds for geometry
    // that renders identically either way, since a per-surface material override gives a multi-surface
    // mesh the same per-batch material control a separate MeshInstance3D per batch had.
    private static List<ModelSurface> BuildSurfaces(WmoRoot root, WmoGroup group, int groupIndex)
    {
        var mesh = new ArrayMesh();
        var materials = new List<MeshMaterial>();

        for (int i = 0; i < group.Batches.Length; i++)
        {
            if (!TryAddBatchSurface(mesh, group, group.Batches[i]))
            {
                continue;
            }

            materials.Add(BuildMaterial(root, group, group.Batches[i], (groupIndex * 1000) + i));
        }

        return materials.Count == 0 ? [] : [new ModelSurface($"Group{groupIndex}", mesh, materials)];
    }

    // Adds one batch as a surface of the shared group mesh; returns false (adding nothing) for a batch
    // whose geometry is malformed.
    private static bool TryAddBatchSurface(ArrayMesh mesh, WmoGroup group, WmoBatch batch)
    {
        if (batch.IndexCount == 0 || batch.IndexCount % 3 != 0 || batch.VertexEnd < batch.VertexStart)
        {
            return false;
        }

        long indexStart = batch.IndexStart;
        if (indexStart + batch.IndexCount > group.Indices.Length)
        {
            return false;
        }

        int vertexStart = batch.VertexStart;
        int vertexCount = batch.VertexEnd - batch.VertexStart + 1;
        if (vertexStart + vertexCount > group.Vertices.Length)
        {
            return false;
        }

        bool hasColors = group.VertexColors != null && group.VertexColors.Length == group.Vertices.Length;
        bool hasUvs = group.Uvs.Length == group.Vertices.Length;
        bool hasNormals = group.Normals.Length == group.Vertices.Length;

        var positions = new Vector3[vertexCount];
        var normals = new Vector3[vertexCount];
        var uvs = new Vector2[vertexCount];
        Color[]? colors = hasColors ? new Color[vertexCount] : null;

        for (int i = 0; i < vertexCount; i++)
        {
            int source = vertexStart + i;
            positions[i] = group.Vertices[source];
            normals[i] = hasNormals ? group.Normals[source] : Vector3.Up;
            uvs[i] = hasUvs ? group.Uvs[source] : Vector2.Zero;
            if (colors != null)
            {
                colors[i] = group.VertexColors![source];
            }
        }

        var indices = new int[batch.IndexCount];
        for (int i = 0; i < batch.IndexCount; i++)
        {
            ushort globalIndex = group.Indices[indexStart + i];
            if (globalIndex < vertexStart || globalIndex >= vertexStart + vertexCount)
            {
                return false;
            }

            indices[i] = globalIndex - vertexStart;
        }

        // WoW triangles are clockwise-front-facing (D3D convention); the coordinate conversion is a
        // proper rotation and does not flip this, so rewind explicitly for Godot's CCW front face.
        for (int i = 0; i + 2 < indices.Length; i += 3)
        {
            (indices[i + 1], indices[i + 2]) = (indices[i + 2], indices[i + 1]);
        }

        var arrays = new Godot.Collections.Array();
        arrays.Resize((int)Mesh.ArrayType.Max);
        arrays[(int)Mesh.ArrayType.Vertex] = positions;
        arrays[(int)Mesh.ArrayType.Normal] = normals;
        arrays[(int)Mesh.ArrayType.TexUV] = uvs;
        if (colors != null)
        {
            arrays[(int)Mesh.ArrayType.Color] = colors;
        }

        arrays[(int)Mesh.ArrayType.Index] = indices;

        mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, arrays);
        return true;
    }

    private static MeshMaterial BuildMaterial(WmoRoot root, WmoGroup group, WmoBatch batch, int fallbackIndex)
    {
        WmoMaterialDef? material = batch.MaterialIndex < root.Materials.Length ? root.Materials[batch.MaterialIndex] : null;
        string texturePath = material?.TexturePath ?? "";
        bool hasColors = group.VertexColors != null && group.VertexColors.Length == group.Vertices.Length;

        return WmoMaterial.Describe(
            texture: texturePath,
            tint: texturePath.Length == 0 ? ModelAsset.FallbackColor(fallbackIndex) : Colors.White,
            blend: WowBlendModes.FromRaw(material?.BlendMode ?? 0),
            twoSided: material?.Flags.TwoSided ?? false,
            unlit: material?.Flags.Unlit ?? false,
            repeatTexture: material?.Flags.RepeatTexture ?? true,
            vertexColor: hasColors);
    }
}
