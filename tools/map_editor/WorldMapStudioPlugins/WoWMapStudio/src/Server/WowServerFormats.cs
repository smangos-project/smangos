using System;

namespace WorldMapStudio.WoW;

/// <summary>
/// The extractor format versions the readers in this folder understand. Each mirrors a constant in the
/// server's C++ and is checked on every read: a mismatch refuses with <see cref="WowServerDataException"/>
/// instead of parsing, because a navmesh or terrain grid misread under the wrong layout still looks
/// plausible while being wrong.
/// </summary>
public static class WowServerFormats
{
    /// <summary><c>MAP_VERSION_MAGIC</c> in <c>src/game/Maps/GridMap.cpp</c>.</summary>
    public static ReadOnlySpan<byte> MapVersionMagic => "z1.4"u8;

    public static ReadOnlySpan<byte> MapMagic => "MAPS"u8;

    public static ReadOnlySpan<byte> MapAreaMagic => "AREA"u8;

    public static ReadOnlySpan<byte> MapHeightMagic => "MHGT"u8;

    public static ReadOnlySpan<byte> MapLiquidMagic => "MLIQ"u8;

    /// <summary><c>VMAP_MAGIC</c> in <c>src/game/vmap/VMapDefinitions.h</c>: the first eight bytes of every <c>.vmtile</c> and <c>.vmo</c>.</summary>
    public static ReadOnlySpan<byte> VmapMagic => "VMAP_7.0"u8;

    /// <summary><c>MMAP_MAGIC</c> in <c>src/game/Maps/MoveMapSharedDefines.h</c>.</summary>
    public const uint MmapMagic = 0x4d4d4150;

    /// <summary><c>MMAP_VERSION</c> in <c>src/game/Maps/MoveMapSharedDefines.h</c>.</summary>
    public const uint MmapVersion = 6;

    /// <summary><c>DT_NAVMESH_MAGIC</c> in <c>src/recastnavigation/Detour/Include/DetourNavMesh.h</c>.</summary>
    public const int DetourMagic = ('D' << 24) | ('N' << 16) | ('A' << 8) | 'V';

    /// <summary><c>DT_NAVMESH_VERSION</c> in the same header.</summary>
    public const int DetourVersion = 7;
}

/// <summary>Extracted server data that cannot be read: wrong version, wrong magic or a truncated file.</summary>
public sealed class WowServerDataException : Exception
{
    public WowServerDataException(string message) : base(message)
    {
    }

    /// <summary>A version field that does not match what this build reads.</summary>
    public static WowServerDataException VersionMismatch(string what, uint found, uint expected) =>
        new($"{what} is version {found}, but this build reads version {expected}. Re-extract the data, or update the reader.");

    /// <summary>A magic that names a version — <c>VMAP_6.0</c> where <c>VMAP_7.0</c> is read.</summary>
    public static WowServerDataException MagicMismatch(string what, string found, string expected) =>
        new($"{what} is '{found}', but this build reads '{expected}'. Re-extract the data, or update the reader.");

    /// <summary>A magic that does not match — usually not the kind of file it was named as.</summary>
    public static WowServerDataException BadMagic(string what) =>
        new($"{what} has the wrong magic. It is not a file this reader understands; re-extract the data.");

    public static WowServerDataException Truncated(string what) =>
        new($"{what} is truncated or its offsets are out of range. Re-extract the data.");
}
