using Godot;

namespace WorldMapStudio.WoW;

internal readonly record struct M2Vertex(Vector3 Position, Vector3 Normal, Vector2 Uv);

/// <summary>A texture slot. <see cref="Name"/> is empty for replaceable slots (type != 0, e.g. creature
/// skin) and for fileless slots; <see cref="FileDataId"/> is the model's TXID entry for the slot.</summary>
internal sealed record M2TextureDef(uint Type, string Name, uint FileDataId);

internal readonly record struct M2MaterialFlags(bool Unlit, bool TwoSided, bool DepthWrite);

internal sealed record M2MaterialDef(M2MaterialFlags Flags, ushort Blend);

/// <summary>An attachment point id (<see cref="M2AttachmentSlot"/>) and its bind-pose position in model space.</summary>
internal readonly record struct M2Attachment(uint Id, Vector3 Position);

internal sealed class M2Document
{
    public required uint[] SkinFileDataIds { get; init; }

    public required M2Vertex[] Vertices { get; init; }

    public required M2TextureDef[] Textures { get; init; }

    public required M2MaterialDef[] Materials { get; init; }

    /// <summary>Batches index into this to find a texture; entries point into <see cref="Textures"/>.</summary>
    public required ushort[] TextureLookup { get; init; }

    /// <summary>Per <c>ModelColor</c> block, its opacity at rest (first keyframe of the first
    /// animation). A batch bound to a block whose rest opacity is ~0 (blink eyelids, eye shine) is
    /// dropped, since only a static pose is rendered.</summary>
    public required float[] ColorRestAlpha { get; init; }

    public required M2Attachment[] Attachments { get; init; }

    public required Aabb Bounds { get; init; }
}

/// <summary><see cref="Id"/> is the geoset id creature/character geoset selection keys off.</summary>
internal readonly record struct M2SkinSubmesh(ushort Id, ushort VertexStart, ushort VertexCount, uint IndexStart, ushort IndexCount);

/// <summary><see cref="ColorIndex"/> is -1 for a batch with no colour/alpha animation.</summary>
internal readonly record struct M2SkinBatch(
    ushort SkinSectionIndex, ushort MaterialIndex, ushort TextureComboIndex, short ColorIndex = -1);

internal sealed class M2Skin
{
    /// <summary>Local-vertex to global-vertex lookup (into <see cref="M2Document.Vertices"/>).</summary>
    public required ushort[] VertexLookup { get; init; }

    /// <summary>Triangle list of local vertex indices (into <see cref="VertexLookup"/>).</summary>
    public required ushort[] Indices { get; init; }

    public required M2SkinSubmesh[] Submeshes { get; init; }

    public required M2SkinBatch[] Batches { get; init; }
}
