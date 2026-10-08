using System;
using System.Collections.Generic;
using Godot;
using WoWLib.Formats.Common;
using WoWLib.Formats.M2.Chunked;
using WoWLib.Formats.M2.Root;
using WoWLib.Formats.M2.Root.Record;
using WoWLib.Formats.M2.Skin;

using WowM2Vertex = WoWLib.Formats.M2.Root.Record.M2Vertex;
using WowM2Material = WoWLib.Formats.M2.Root.Record.M2Material;

namespace WorldMapStudio.WoW;

/// <summary>One material's worth of geometry: one texture, one material and one skin batch.</summary>
public readonly record struct ModelSurfaceInput(
    IReadOnlyList<Vector3> Positions,
    IReadOnlyList<Vector3> Normals,
    IReadOnlyList<Vector2> Uvs,
    IReadOnlyList<int> Indices,
    uint TextureFileDataId,
    string Blend,
    bool TwoSided,
    bool Unlit,
    bool DepthWrite);

/// <summary>Writes a static procedural model as a 1.15 chunked <c>.m2</c> plus one <c>.skin</c>.</summary>
public static class M2ModelWriter
{
    private const uint FormatVersion = 274;
    private const uint GlobalFlagBits = 0x202080;

    // The client needs a non-zero skin profile count in the header even though SFID lists the skins.
    private const int SkinProfileCountOffset = 68;
    private const int SequenceArrayOffset = 28;
    private const int SequenceBoundsOffset = 32;

    public static (byte[] M2Bytes, byte[] SkinBytes) Write(string name, IReadOnlyList<ModelSurfaceInput> surfaces, uint skinFileDataId = 0)
    {
        using var root = new M2RootLegionPlus { FormatVersion = FormatVersion, Name = name, GlobalFlags = (GlobalFlags)GlobalFlagBits };
        using var skin = new SkinCataPlus();
        using M2ChunkedFile chunked = M2ChunkedFile.ForVersion(WoWLib.Versions.Global.ClassicEra);

        var wowPositions = new List<Vector3>();
        int indexTotal = 0;
        foreach (ModelSurfaceInput s in surfaces)
        {
            foreach (Vector3 p in s.Positions)
            {
                wowPositions.Add(WowCoordinates.FromGodot(p));
            }

            indexTotal += s.Indices.Count;
        }

        if (wowPositions.Count > ushort.MaxValue)
        {
            throw new InvalidOperationException($"M2 '{name}' has {wowPositions.Count} vertices; the skin format allows 65535");
        }

        (Vector3 min, Vector3 max, float radius) = Bounds(wowPositions);
        var box = new CAaBox(new C3Vector(min.X, min.Y, min.Z), new C3Vector(max.X, max.Y, max.Z));
        root.BoundingBox = box;
        root.BoundingSphereRadius = radius;
        root.CollisionBox = new CAaBox(box);
        root.CollisionSphereRadius = radius;

        AddCollision(root, wowPositions, surfaces);

        root.GlobalLoops.Add(new M2Loop());
        root.Sequences.Add(new M2SequenceWodPlus
        {
            Duration = 1000,
            Flags = (SequenceFlags)0x820,
            Frequency = 0x7fff,
            BlendTimeIn = 150,
            BlendTimeOut = 0,
            VariationNext = -1,
            AliasNext = 0,
        });
        root.Bones.Add(new M2CompBoneWotlkPlus { KeyBoneId = -1, ParentBone = -1 });
        root.BoneLookupTable.Add(0);
        root.TextureWeights.Add(new M2TextureWeightWotlkPlus());
        root.TransparencyLookupTable.Add(0);
        root.TextureTransformsLookupTable.Add(-1);

        var profile = skin.Profile;
        profile.BoneCountMax = 0;
        int vertexStart = 0;
        int indexStart = 0;
        for (int i = 0; i < surfaces.Count; i++)
        {
            ModelSurfaceInput s = surfaces[i];
            for (int v = 0; v < s.Positions.Count; v++)
            {
                Vector3 pos = wowPositions[vertexStart + v];
                Vector3 n = WowCoordinates.FromGodot(v < s.Normals.Count ? s.Normals[v] : Vector3.Up);
                Vector2 uv = v < s.Uvs.Count ? s.Uvs[v] : Vector2.Zero;
                using var vertex = new WowM2Vertex
                {
                    Pos = new C3Vector(pos.X, pos.Y, pos.Z),
                    Normal = new C3Vector(n.X, n.Y, n.Z),
                };
                vertex.BoneWeights[0] = 255;
                vertex.TexCoords[0] = new C2Vector(uv.X, uv.Y);
                vertex.TexCoords[1] = new C2Vector(uv.X, uv.Y);
                root.Vertices.Add(vertex);
                profile.Vertices.Add((ushort)(vertexStart + v));
                profile.Bones.Add(vertex.BoneIndices);
            }

            foreach (int index in s.Indices)
            {
                profile.Indices.Add((ushort)(index + vertexStart));
            }

            root.Textures.Add(new M2Texture { Type = 0, Flags = 3 });
            chunked.TextureFdids.Add(s.TextureFileDataId);
            root.TextureLookupTable.Add((ushort)i);
            root.Materials.Add(new WowM2Material { Flags = MaterialFlagsFor(s), BlendingMode = BlendValue(s.Blend) });

            profile.Submeshes.Add(new M2SkinSectionTbcPlus
            {
                SkinSectionId = 0,
                Level = (ushort)(indexStart >> 16),
                VertexStart = (ushort)vertexStart,
                VertexCount = (ushort)s.Positions.Count,
                IndexStart = (ushort)(indexStart & 0xFFFF),
                IndexCount = (ushort)s.Indices.Count,
                BoneCount = 1,
                BoneInfluences = 0,
                CenterPosition = new C3Vector((min.X + max.X) / 2, (min.Y + max.Y) / 2, (min.Z + max.Z) / 2),
                SortCenterPosition = new C3Vector((min.X + max.X) / 2, (min.Y + max.Y) / 2, (min.Z + max.Z) / 2),
                SortRadius = radius,
            });
            profile.Batches.Add(new M2Batch(16, 0, 0, (ushort)i, 0, ushort.MaxValue, (ushort)i, 0, 1, (ushort)i, 0, 0, 0));

            vertexStart += s.Positions.Count;
            indexStart += s.Indices.Count;
        }

        if (skinFileDataId != 0)
        {
            chunked.SkinFdids.Add(skinFileDataId);
        }

        byte[] image = root.Write();
        BitConverter.GetBytes(1u).CopyTo(image, SkinProfileCountOffset);
        int sequenceStart = BitConverter.ToInt32(image, SequenceArrayOffset + 4) + SequenceBoundsOffset;
        float[] bounds = { min.X, min.Y, min.Z, max.X, max.Y, max.Z, radius };
        for (int i = 0; i < bounds.Length; i++)
        {
            BitConverter.GetBytes(bounds[i]).CopyTo(image, sequenceStart + i * 4);
        }

        byte[] rest = chunked.Write();
        var m2 = new byte[8 + image.Length + rest.Length - 8];
        BitConverter.GetBytes(0x3132444Du).CopyTo(m2, 0);
        BitConverter.GetBytes((uint)image.Length).CopyTo(m2, 4);
        image.CopyTo(m2, 8);
        Array.Copy(rest, 8, m2, 8 + image.Length, rest.Length - 8);
        return (m2, skin.Write());
    }

    /// <summary>Every surface collides: the render mesh doubles as the bounding mesh the server extractor reads.</summary>
    private static void AddCollision(M2RootLegionPlus root, List<Vector3> wowPositions, IReadOnlyList<ModelSurfaceInput> surfaces)
    {
        foreach (Vector3 p in wowPositions)
        {
            root.CollisionVertices.Add(new C3Vector(p.X, p.Y, p.Z));
        }

        int vertexStart = 0;
        foreach (ModelSurfaceInput s in surfaces)
        {
            for (int i = 0; i + 2 < s.Indices.Count; i += 3)
            {
                int a = vertexStart + s.Indices[i], b = vertexStart + s.Indices[i + 1], c = vertexStart + s.Indices[i + 2];
                Vector3 n = (wowPositions[b] - wowPositions[a]).Cross(wowPositions[c] - wowPositions[a]).Normalized();
                for (int k = 0; k < 3; k++)
                {
                    root.CollisionNormals.Add(new C3Vector(n.X, n.Y, n.Z));
                }

                root.CollisionTriangles.Add((ushort)a);
                root.CollisionTriangles.Add((ushort)b);
                root.CollisionTriangles.Add((ushort)c);
            }

            vertexStart += s.Positions.Count;
        }
    }

    private static MaterialFlags MaterialFlagsFor(ModelSurfaceInput s)
    {
        ushort bits = 0;
        if (s.Unlit) bits |= 0x1;
        if (s.TwoSided) bits |= 0x4;
        if (!s.DepthWrite) bits |= 0x10;
        return (MaterialFlags)bits;
    }

    private static ushort BlendValue(string blend) => blend switch
    {
        "alpha_key" => 1,
        "alpha" => 2,
        "no_add_alpha" => 3,
        "add" => 4,
        "mod" => 5,
        "mod2x" => 6,
        _ => 0,
    };

    private static (Vector3 Min, Vector3 Max, float Radius) Bounds(IReadOnlyList<Vector3> wow)
    {
        if (wow.Count == 0)
        {
            return (Vector3.Zero, Vector3.Zero, 0f);
        }

        Vector3 min = wow[0];
        Vector3 max = min;
        float radius = 0f;
        foreach (Vector3 p in wow)
        {
            min = new Vector3(Mathf.Min(min.X, p.X), Mathf.Min(min.Y, p.Y), Mathf.Min(min.Z, p.Z));
            max = new Vector3(Mathf.Max(max.X, p.X), Mathf.Max(max.Y, p.Y), Mathf.Max(max.Z, p.Z));
            radius = Mathf.Max(radius, p.Length());
        }

        return (min, max, radius);
    }
}
