using System;
using System.Collections.Generic;
using Godot;
using WoWLib.Formats.Common;
using WoWLib.Formats.WMO.Group.Chunks;
using WoWLib.Formats.WMO.Root.Chunks;

using WowWmoBatch = WoWLib.Formats.WMO.Group.Chunks.WMOBatchLegionPlus;

namespace WorldMapStudio.WoW;

/// <summary>Writes a static procedural model as a 1.15 root <c>.wmo</c> plus one exterior group file.</summary>
public static class WmoModelWriter
{
    private const float BoxPad = 0.028f;
    private const PolyFlags SolidPoly = PolyFlags.Render | PolyFlags.CullObjects;

    /// <param name="groupFileDataId">Id written into the root's group file list (GFID).</param>
    public static (byte[] RootBytes, byte[] GroupBytes) Write(IReadOnlyList<ModelSurfaceInput> surfaces, uint groupFileDataId)
    {
        var positions = new List<Vector3>();
        var normals = new List<Vector3>();
        var uvs = new List<Vector2>();
        var indices = new List<int>();
        var batchRanges = new List<(int IndexStart, int IndexCount, int VertexMin, int VertexMax)>();
        foreach (ModelSurfaceInput s in surfaces)
        {
            int vertexStart = positions.Count;
            for (int v = 0; v < s.Positions.Count; v++)
            {
                positions.Add(WowCoordinates.FromGodot(s.Positions[v]));
                normals.Add(WowCoordinates.FromGodot(v < s.Normals.Count ? s.Normals[v] : Vector3.Up));
                uvs.Add(v < s.Uvs.Count ? s.Uvs[v] : Vector2.Zero);
            }

            batchRanges.Add((indices.Count, s.Indices.Count, vertexStart, vertexStart + s.Positions.Count - 1));
            foreach (int index in s.Indices)
            {
                indices.Add(index + vertexStart);
            }
        }

        if (positions.Count > ushort.MaxValue)
        {
            throw new InvalidOperationException($"WMO group has {positions.Count} vertices; a group allows 65535");
        }

        (Vector3 min, Vector3 max) = Bounds(positions);
        min -= Vector3.One * BoxPad;
        max += Vector3.One * BoxPad;
        var box = new CAaBox(new C3Vector(min.X, min.Y, min.Z), new C3Vector(max.X, max.Y, max.Z));

        using var root = new WoWLib.Formats.WMO.Root.WMORootLegion();
        root.Header.NTextures = (uint)surfaces.Count;
        root.Header.BoundingBox = new CAaBox(box);
        root.Header.AmbientColor = new CArgb();
        root.GroupNames.Add("");
        int nameOffset = (int)root.GroupNames.Add("Exterior");
        root.GroupInfos.Add(new SmoGroupInfo(GroupFlags.HasBsp | GroupFlags.Exterior, new CAaBox(box), nameOffset));
        root.GroupFdids.Add(groupFileDataId);
        foreach (ModelSurfaceInput s in surfaces)
        {
            MaterialFlags flags = 0;
            if (s.Unlit) flags |= MaterialFlags.Unlit;
            if (s.TwoSided) flags |= MaterialFlags.TwoSided;
            root.Materials.Add(new SmoMaterial
            {
                Flags = flags,
                BlendMode = BlendValue(s.Blend),
                Texture1 = s.TextureFileDataId,
            });
        }

        using var group = new WoWLib.Formats.WMO.Group.WMOGroupLegion();
        var body = group.Body;
        body.Header.Flags = GroupFlags.HasBsp | GroupFlags.Exterior;
        body.Header.BoundingBox = new CAaBox(box);
        body.Header.ExtBatchCount = (ushort)surfaces.Count;
        body.Header.FogIds[0] = 0;

        var uvVector = new WoWLib.Vector<C2Vector>();
        for (int i = 0; i < positions.Count; i++)
        {
            body.Vertices.Add(new C3Vector(positions[i].X, positions[i].Y, positions[i].Z));
            body.Normals.Add(new C3Vector(normals[i].X, normals[i].Y, normals[i].Z));
            uvVector.Add(new C2Vector(uvs[i].X, uvs[i].Y));
        }

        body.AppendTexcoordSet(uvVector);
        foreach (int index in indices)
        {
            body.Indices.Add((ushort)index);
        }

        for (int b = 0; b < surfaces.Count; b++)
        {
            (int start, int count, int vMin, int vMax) = batchRanges[b];
            body.Batches.Add(new WowWmoBatch
            {
                StartIndex = (uint)start,
                Count = (ushort)count,
                MinIndex = (ushort)vMin,
                MaxIndex = (ushort)vMax,
                MaterialId = (byte)b,
                MaterialIdLarge = (ushort)b,
            });
            for (int t = 0; t < count / 3; t++)
            {
                body.Polys.Add(new SmoPoly(SolidPoly, (byte)b));
            }
        }

        (List<WmoBspNode> nodes, List<ushort> faces) = WmoBspBuilder.Build(positions, indices);
        foreach (WmoBspNode n in nodes)
        {
            body.BspNodes.Add(new CAaBspNode(n.Flags, n.NegChild, n.PosChild, n.FaceCount, n.FaceStart, n.PlaneDistance));
        }

        foreach (ushort face in faces)
        {
            body.BspFaceIndices.Add(face);
        }

        return (root.Write(), group.Write());
    }

    private static uint BlendValue(string blend) => blend switch
    {
        "alpha_key" => 1,
        "alpha" => 2,
        "add" => 3,
        _ => 0,
    };

    private static (Vector3 Min, Vector3 Max) Bounds(IReadOnlyList<Vector3> points)
    {
        if (points.Count == 0)
        {
            return (Vector3.Zero, Vector3.Zero);
        }

        Vector3 min = points[0];
        Vector3 max = min;
        foreach (Vector3 p in points)
        {
            min = new Vector3(Mathf.Min(min.X, p.X), Mathf.Min(min.Y, p.Y), Mathf.Min(min.Z, p.Z));
            max = new Vector3(Mathf.Max(max.X, p.X), Mathf.Max(max.Y, p.Y), Mathf.Max(max.Z, p.Z));
        }

        return (min, max);
    }
}
