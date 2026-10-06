using System;
using Godot;
using WoWLib.Formats.M2.Chunked;
using WoWLib.Formats.M2.Root;
using WoWLib.Formats.M2.Skin;

namespace WorldMapStudio.WoW;

/// <summary>Reads Legion+ chunked <c>.m2</c> files and their <c>.skin</c> files through wowlib into the
/// loader's Godot-space <see cref="M2Document"/> / <see cref="M2Skin"/>.</summary>
internal static class M2Reader
{
    private const uint ChunkedMagic = 0x3132444D; // "MD21"

    public static M2Document? Read(byte[] bytes, string path)
    {
        try
        {
            return ReadCore(bytes);
        }
        catch (Exception e)
        {
            GD.PushWarning($"[WoW M2] Failed to read '{path}': {e.Message}");
            return null;
        }
    }

    public static M2Skin? ReadSkin(byte[] bytes, string path)
    {
        try
        {
            using var file = new SkinCataPlus();
            file.Read(bytes);
            var profile = file.Profile;
            var submeshes = new M2SkinSubmesh[profile.Submeshes.Count];
            for (int i = 0; i < submeshes.Length; i++)
            {
                var s = profile.Submeshes[i];
                submeshes[i] = new M2SkinSubmesh(s.SkinSectionId, s.VertexStart, s.VertexCount, s.IndexStart + ((uint)s.Level << 16), s.IndexCount);
            }

            var batches = new M2SkinBatch[profile.Batches.Count];
            for (int i = 0; i < batches.Length; i++)
            {
                var b = profile.Batches[i];
                batches[i] = new M2SkinBatch(b.SkinSectionIndex, b.MaterialIndex, b.TextureComboIndex, b.ColorIndex == ushort.MaxValue ? (short)-1 : (short)b.ColorIndex);
            }

            return new M2Skin
            {
                VertexLookup = profile.Vertices.ToArray(),
                Indices = profile.Indices.ToArray(),
                Submeshes = submeshes,
                Batches = batches,
            };
        }
        catch (Exception e)
        {
            GD.PushWarning($"[WoW M2] Failed to read skin '{path}': {e.Message}");
            return null;
        }
    }

    private static M2Document? ReadCore(byte[] bytes)
    {
        if (bytes.Length < 8 || BitConverter.ToUInt32(bytes, 0) != ChunkedMagic)
        {
            return null;
        }

        byte[]? image = null;
        foreach ((uint id, int start, int size) in WowChunkReader.Chunks(bytes, 0, bytes.Length))
        {
            if (id == ChunkedMagic)
            {
                image = bytes.AsSpan(start, size).ToArray();
                break;
            }
        }

        if (image == null)
        {
            return null;
        }

        using M2ChunkedFile chunked = M2ChunkedFile.ForVersion(WoWLib.Versions.Global.ClassicEra);
        chunked.Read(bytes);
        uint[] skinFdids = chunked.SkinFdids.ToArray();
        uint[] textureFdids = chunked.TextureFdids.ToArray();

        using var root = new M2RootLegionPlus();
        root.Read(image);

        var vertices = new M2Vertex[root.Vertices.Count];
        for (int i = 0; i < vertices.Length; i++)
        {
            var v = root.Vertices[i];
            var p = v.Pos;
            var n = v.Normal;
            var uv = v.TexCoords[0];
            vertices[i] = new M2Vertex(
                WowCoordinates.ToGodot(new Vector3(p.X, p.Y, p.Z)),
                WowCoordinates.ToGodot(new Vector3(n.X, n.Y, n.Z)),
                new Vector2(uv.X, uv.Y));
        }

        var textures = new M2TextureDef[root.Textures.Count];
        for (int i = 0; i < textures.Length; i++)
        {
            var t = root.Textures[i];
            textures[i] = new M2TextureDef(t.Type, t.Type == 0 ? t.Filename ?? "" : "", i < textureFdids.Length ? textureFdids[i] : 0);
        }

        var materials = new M2MaterialDef[root.Materials.Count];
        for (int i = 0; i < materials.Length; i++)
        {
            var m = root.Materials[i];
            ushort flags = (ushort)m.Flags;
            materials[i] = new M2MaterialDef(
                new M2MaterialFlags((flags & 0x1) != 0, (flags & 0x4) != 0, (flags & 0x10) == 0),
                m.BlendingMode);
        }

        var colorAlpha = new float[root.Colors.Count];
        for (int i = 0; i < colorAlpha.Length; i++)
        {
            colorAlpha[i] = RestAlpha(root.Colors[i].Alpha);
        }

        var attachments = new M2Attachment[root.Attachments.Count];
        for (int i = 0; i < attachments.Length; i++)
        {
            var a = root.Attachments[i];
            var p = a.Position;
            attachments[i] = new M2Attachment(a.Id, WowCoordinates.ToGodot(new Vector3(p.X, p.Y, p.Z)));
        }

        var min = root.BoundingBox.Min;
        var max = root.BoundingBox.Max;
        return new M2Document
        {
            SkinFileDataIds = skinFdids,
            Vertices = vertices,
            Textures = textures,
            Materials = materials,
            TextureLookup = root.TextureLookupTable.ToArray(),
            ColorRestAlpha = colorAlpha,
            Attachments = attachments,
            Bounds = WowCoordinates.TransformBounds(new Vector3(min.X, min.Y, min.Z), new Vector3(max.X, max.Y, max.Z)),
        };
    }

    // First keyframe of the first animation, 0..1; 1 for a track with no animations (always on).
    private static float RestAlpha(WoWLib.Formats.M2.Root.Record.M2TrackFixed16WotlkPlus track)
    {
        if (track.Values.Count == 0)
        {
            return 1f;
        }

        var keys = track.Values[0];
        if (keys.Count == 0)
        {
            return 0f;
        }

        return Math.Clamp(keys[0].AsFloat, 0f, 1f);
    }
}
