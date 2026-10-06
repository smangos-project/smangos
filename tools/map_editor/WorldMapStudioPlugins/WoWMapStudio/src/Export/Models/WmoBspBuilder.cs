using System;
using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

public readonly record struct WmoBspNode(ushort Flags, short NegChild, short PosChild, ushort FaceCount, uint FaceStart, float PlaneDistance);

/// <summary>
/// Axis-aligned BSP over a group's triangles (MOBN/MOBR). Nodes split on the longest axis of their
/// box at the median triangle centroid; triangles reaching into both halves are listed in both children.
/// </summary>
public static class WmoBspBuilder
{
    public const ushort LeafFlag = 4;
    public const int MaxFaces = 65535;
    private const int LeafFaces = 128;
    private const int MaxDepth = 24;
    private const float MaxDuplication = 1.5f;

    public static (List<WmoBspNode> Nodes, List<ushort> Faces) Build(IReadOnlyList<Vector3> vertices, IReadOnlyList<int> indices)
    {
        int faceCount = indices.Count / 3;
        if (faceCount > MaxFaces)
        {
            throw new InvalidOperationException($"{faceCount} faces exceed the {MaxFaces} a group BSP can address");
        }

        var nodes = new List<WmoBspNode>();
        var faces = new List<ushort>();
        if (faceCount == 0)
        {
            return (nodes, faces);
        }

        var lo = new Vector3[faceCount];
        var hi = new Vector3[faceCount];
        var centre = new Vector3[faceCount];
        for (int f = 0; f < faceCount; f++)
        {
            Vector3 a = vertices[indices[f * 3]];
            Vector3 b = vertices[indices[f * 3 + 1]];
            Vector3 c = vertices[indices[f * 3 + 2]];
            lo[f] = a.Min(b).Min(c);
            hi[f] = a.Max(b).Max(c);
            centre[f] = (a + b + c) / 3f;
        }

        var all = new int[faceCount];
        for (int i = 0; i < faceCount; i++)
        {
            all[i] = i;
        }

        var stack = new Stack<(int[] Ids, int Depth, int Parent, int Side)>();
        stack.Push((all, 0, -1, 0));
        var nodeData = new List<(ushort Flags, short Neg, short Pos, ushort Count, uint Start, float Plane)>();
        while (stack.Count > 0)
        {
            (int[] ids, int depth, int parent, int side) = stack.Pop();
            int index = nodeData.Count;
            if (parent >= 0)
            {
                var p = nodeData[parent];
                if (side == 0) p.Neg = (short)index; else p.Pos = (short)index;
                nodeData[parent] = p;
            }

            Vector3 boxLo = lo[ids[0]];
            Vector3 boxHi = hi[ids[0]];
            foreach (int id in ids)
            {
                boxLo = boxLo.Min(lo[id]);
                boxHi = boxHi.Max(hi[id]);
            }

            (int Axis, float Plane, int[] Neg, int[] Pos)? split = null;
            if (ids.Length > LeafFaces && depth < MaxDepth)
            {
                Vector3 extent = boxHi - boxLo;
                int[] axes = [0, 1, 2];
                Array.Sort(axes, (x, y) => extent[y].CompareTo(extent[x]));
                foreach (int axis in axes)
                {
                    if (extent[axis] <= 1e-6f)
                    {
                        break;
                    }

                    float d = Median(ids, centre, axis);
                    int[] neg = Select(ids, id => lo[id][axis] <= d);
                    int[] pos = Select(ids, id => hi[id][axis] >= d);
                    if (Degenerate(neg, pos, ids))
                    {
                        d = (boxLo[axis] + boxHi[axis]) / 2f;
                        neg = Select(ids, id => lo[id][axis] <= d);
                        pos = Select(ids, id => hi[id][axis] >= d);
                    }

                    if (!Degenerate(neg, pos, ids) && neg.Length + pos.Length <= MaxDuplication * ids.Length)
                    {
                        split = (axis, d, neg, pos);
                        break;
                    }
                }
            }

            if (split is not { } s)
            {
                nodeData.Add((LeafFlag, -1, -1, (ushort)ids.Length, (uint)faces.Count, 0f));
                foreach (int id in ids)
                {
                    faces.Add((ushort)id);
                }
            }
            else
            {
                nodeData.Add(((ushort)s.Axis, -1, -1, 0, 0, s.Plane));
                stack.Push((s.Pos, depth + 1, index, 1));
                stack.Push((s.Neg, depth + 1, index, 0));
            }
        }

        if (nodeData.Count > short.MaxValue)
        {
            throw new InvalidOperationException("BSP has too many nodes");
        }

        foreach (var n in nodeData)
        {
            nodes.Add(new WmoBspNode(n.Flags, n.Neg, n.Pos, n.Count, n.Start, n.Plane));
        }

        return (nodes, faces);
    }

    private static bool Degenerate(int[] neg, int[] pos, int[] ids) =>
        Math.Min(neg.Length, pos.Length) == 0 || Math.Max(neg.Length, pos.Length) >= ids.Length;

    private static int[] Select(int[] ids, Func<int, bool> keep)
    {
        var result = new List<int>();
        foreach (int id in ids)
        {
            if (keep(id))
            {
                result.Add(id);
            }
        }

        return result.ToArray();
    }

    private static float Median(int[] ids, Vector3[] centre, int axis)
    {
        var values = new float[ids.Length];
        for (int i = 0; i < ids.Length; i++)
        {
            values[i] = centre[ids[i]][axis];
        }

        Array.Sort(values);
        int mid = values.Length / 2;
        return values.Length % 2 == 1 ? values[mid] : (values[mid - 1] + values[mid]) / 2f;
    }
}
