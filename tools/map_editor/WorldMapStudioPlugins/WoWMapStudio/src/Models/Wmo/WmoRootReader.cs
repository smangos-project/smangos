using System;
using System.Text;
using Godot;
using static WorldMapStudio.WoW.WowBinaryReader;
using static WorldMapStudio.WoW.WowChunkReader;

namespace WorldMapStudio.WoW;

internal readonly record struct WmoMaterialFlags(bool Unlit, bool TwoSided, bool RepeatTexture);

/// <summary>Root-level lighting inputs a group needs to rebase its MOCV (see <see cref="WmoVertexColorFix"/>).</summary>
internal readonly record struct WmoRootLighting(Color AmbientColor, bool SkipBaseColor, bool LightenInteriors);

internal sealed record WmoMaterialDef(WmoMaterialFlags Flags, uint BlendMode, string TexturePath);

internal readonly record struct WmoDoodadPlacement(string Path, Transform3D Transform);

internal sealed record WmoDoodadSet(string Name, WmoDoodadPlacement[] Placements);

internal sealed class WmoRoot
{
    public required int GroupCount { get; init; }

    public required Aabb Bounds { get; init; }

    public required WmoMaterialDef[] Materials { get; init; }

    public required WmoDoodadSet[] DoodadSets { get; init; }

    public required WmoRootLighting Lighting { get; init; }
}

/// <summary>
/// Reads the root <c>.wmo</c> file: MOHD (group count, bounds), MOMT (materials) and MODS/MODD (doodad
/// sets). Texture and doodad names come from the MOTX/MODN blobs when present; files without them store
/// FileDataIDs instead (MOMT texture fields, MODI). Everything else (portals, lights, fog) is skipped.
/// </summary>
internal static class WmoRootReader
{
    private const int MohdSize = 64;
    private const int MomtStride = 64;
    private const int ModsStride = 32;
    private const int ModdStride = 40;

    public static WmoRoot? Read(byte[] bytes, string path)
    {
        int offset = 0;
        if (!TryReadChunkHeader(bytes, ref offset, out uint id, out uint size) || id != ChunkId("MVER"))
        {
            return null;
        }

        if (size < 4 || ReadU32(bytes, offset) != 17)
        {
            GD.PushWarning($"[WoW WMO] '{path}' is not version 17; not supported.");
            return null;
        }

        offset += (int)size;
        if (!TryReadChunkHeader(bytes, ref offset, out id, out size) || id != ChunkId("MOHD"))
        {
            // Not a root file — most likely a group file (starts MVER, MOGP) opened directly.
            return null;
        }

        if (size < MohdSize)
        {
            return null;
        }

        int mohdStart = offset;
        int groupCount = (int)ReadU32(bytes, mohdStart + 4);
        Color ambientColor = ReadBgra(bytes, mohdStart + 28);
        ushort mohdFlags = ReadU16(bytes, mohdStart + 60);
        var lighting = new WmoRootLighting(ambientColor, SkipBaseColor: (mohdFlags & 0x2) != 0, LightenInteriors: (mohdFlags & 0x8) != 0);
        Vector3 boundsMin = ReadVector3(bytes, mohdStart + 36);
        Vector3 boundsMax = ReadVector3(bytes, mohdStart + 48);
        offset += (int)size;

        int motxOffset = -1;
        uint motxSize = 0;
        int modnOffset = -1;
        uint modnSize = 0;
        int momtOffset = -1;
        uint momtSize = 0;
        int moddOffset = -1;
        uint moddSize = 0;
        uint[]? doodadFdids = null;
        (string Name, int Start, int Count)[] doodadSetRanges = [];

        while (TryReadChunkHeader(bytes, ref offset, out id, out size))
        {
            if (id == ChunkId("MOTX"))
            {
                motxOffset = offset;
                motxSize = size;
            }
            else if (id == ChunkId("MOMT"))
            {
                momtOffset = offset;
                momtSize = size;
            }
            else if (id == ChunkId("MODS"))
            {
                doodadSetRanges = ReadDoodadSetRanges(bytes, offset, size);
            }
            else if (id == ChunkId("MODN"))
            {
                modnOffset = offset;
                modnSize = size;
            }
            else if (id == ChunkId("MODD"))
            {
                moddOffset = offset;
                moddSize = size;
            }
            else if (id == ChunkId("MODI"))
            {
                doodadFdids = new uint[size / 4];
                for (int i = 0; i < doodadFdids.Length; i++)
                {
                    doodadFdids[i] = ReadU32(bytes, offset + (i * 4));
                }
            }

            offset += (int)size;
        }

        WmoMaterialDef[] materials = momtOffset < 0 ? [] : ReadMaterials(bytes, momtOffset, momtSize, motxOffset, motxSize);
        WmoDoodadPlacement[] placements = moddOffset < 0
            ? []
            : ReadDoodadPlacements(bytes, moddOffset, moddSize, modnOffset, modnSize, doodadFdids);

        return new WmoRoot
        {
            GroupCount = groupCount,
            Bounds = WowCoordinates.TransformBounds(boundsMin, boundsMax),
            Materials = materials,
            DoodadSets = ResolveDoodadSets(doodadSetRanges, placements),
            Lighting = lighting,
        };
    }

    /// <summary>Reads a CArgb/CImVector (on-disk byte order B, G, R, A) as a 0..1 <see cref="Color"/>.</summary>
    private static Color ReadBgra(byte[] bytes, int at) =>
        new(bytes[at + 2] / 255f, bytes[at + 1] / 255f, bytes[at] / 255f, bytes[at + 3] / 255f);

    private static WmoDoodadSet[] ResolveDoodadSets((string Name, int Start, int Count)[] ranges, WmoDoodadPlacement[] placements)
    {
        var result = new WmoDoodadSet[ranges.Length];
        for (int i = 0; i < ranges.Length; i++)
        {
            (string name, int start, int count) = ranges[i];
            if (start < 0 || count < 0 || start + count > placements.Length)
            {
                result[i] = new WmoDoodadSet(name, []);
                continue;
            }

            result[i] = new WmoDoodadSet(name, placements[start..(start + count)]);
        }

        return result;
    }

    private static (string Name, int Start, int Count)[] ReadDoodadSetRanges(byte[] bytes, int offset, uint size)
    {
        int count = (int)(size / ModsStride);
        var result = new (string, int, int)[count];
        for (int i = 0; i < count; i++)
        {
            int at = offset + (i * ModsStride);
            string name = ReadNameString(bytes, (uint)at, 20);
            int start = (int)ReadU32(bytes, at + 20);
            int placementCount = (int)ReadU32(bytes, at + 24);
            result[i] = (name, start, placementCount);
        }

        return result;
    }

    private static WmoDoodadPlacement[] ReadDoodadPlacements(
        byte[] bytes, int offset, uint size, int modnOffset, uint modnSize, uint[]? doodadFdids)
    {
        int count = (int)(size / ModdStride);
        var result = new WmoDoodadPlacement[count];
        for (int i = 0; i < count; i++)
        {
            int at = offset + (i * ModdStride);
            uint nameIndex = ReadU32(bytes, at) & 0xFFFFFF;
            string path = doodadFdids != null
                ? ResolveFdid(nameIndex < doodadFdids.Length ? doodadFdids[nameIndex] : 0)
                : ResolveBlobString(bytes, modnOffset, modnSize, nameIndex);
            path = NormalizeDoodadPath(path);

            Vector3 position = WowCoordinates.ToGodot(ReadVector3(bytes, at + 4));
            var rawRotation = new Quaternion(
                ReadSingle(bytes, at + 16),
                ReadSingle(bytes, at + 20),
                ReadSingle(bytes, at + 24),
                ReadSingle(bytes, at + 28));
            Quaternion rotation = WowCoordinates.ToGodot(rawRotation);
            float scale = ReadSingle(bytes, at + 32);

            Basis basis = new Basis(rotation).Scaled(Vector3.One * scale);
            result[i] = new WmoDoodadPlacement(path, new Transform3D(basis, position));
        }

        return result;
    }

    /// <summary>Many real WMOs still reference doodads with a legacy .mdx/.mdl extension; WoW's own model data is .m2.</summary>
    private static string NormalizeDoodadPath(string path)
    {
        if (path.EndsWith(".mdx", StringComparison.OrdinalIgnoreCase) || path.EndsWith(".mdl", StringComparison.OrdinalIgnoreCase))
        {
            return $"{path[..^4]}.m2";
        }

        return path;
    }

    private static WmoMaterialDef[] ReadMaterials(byte[] bytes, int offset, uint size, int motxOffset, uint motxSize)
    {
        int count = (int)(size / MomtStride);
        var result = new WmoMaterialDef[count];
        for (int i = 0; i < count; i++)
        {
            int at = offset + (i * MomtStride);
            uint flagBits = ReadU32(bytes, at);
            uint blendMode = ReadU32(bytes, at + 8);
            uint textureOffset1 = ReadU32(bytes, at + 12);

            var flags = new WmoMaterialFlags(
                Unlit: (flagBits & 0x1) != 0,
                TwoSided: (flagBits & 0x4) != 0,
                RepeatTexture: (flagBits & 0x40) == 0);

            string texture = motxOffset < 0
                ? ResolveFdid(textureOffset1)
                : ResolveBlobString(bytes, motxOffset, motxSize, textureOffset1);
            result[i] = new WmoMaterialDef(flags, blendMode, texture);
        }

        return result;
    }

    private static string ResolveFdid(uint fdid) =>
        fdid != 0 && WowFileDataIds.Shared.PathOf(fdid) is { } path ? AssetPath.Normalize(path) : "";

    /// <summary>Resolves a byte-offset, NUL-terminated path out of a name blob (MOTX or MODN).</summary>
    private static string ResolveBlobString(byte[] bytes, int blobOffset, uint blobSize, uint relativeOffset)
    {
        if (blobOffset < 0 || relativeOffset >= blobSize)
        {
            return "";
        }

        int start = blobOffset + (int)relativeOffset;
        int end = start;
        int blobEnd = blobOffset + (int)blobSize;
        while (end < blobEnd && bytes[end] != 0)
        {
            end++;
        }

        return end > start ? AssetPath.Normalize(Encoding.UTF8.GetString(bytes, start, end - start)) : "";
    }
}
