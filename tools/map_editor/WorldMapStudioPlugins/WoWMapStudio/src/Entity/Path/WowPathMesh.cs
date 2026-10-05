using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Builds real, pickable geometry for a <see cref="WowPathComponent{TNode}"/>: a square-section tube
/// along each of <see cref="NetworkSplineChains.FlattenChains"/>'s flattened chains, plus a small cube
/// marker at every authored vertex — the marker doubles as cover for the seam between two chains'
/// segment boxes, which are built independently rather than as one continuous ring-swept tube. Built
/// from whatever <see cref="VertexNetwork"/> is authored, valid or not, so a branching or disconnected
/// network still shows something for the user to fix rather than rendering nothing.
/// </summary>
internal static class WowPathMesh
{
    public static ArrayMesh Build(VertexNetwork network, float radius, float markerHalfSize)
    {
        var positions = new List<Vector3>();
        var normals = new List<Vector3>();
        var indices = new List<int>();

        foreach (NetworkSplinePolyline chain in NetworkSplineChains.FlattenChains(network))
        {
            IReadOnlyList<Vector3> points = chain.Points;
            int segments = chain.IsClosed ? points.Count : points.Count - 1;
            for (int i = 0; i < segments; i++)
            {
                Vector3 a = points[i];
                Vector3 b = points[(i + 1) % points.Count];
                AddSegmentBox(positions, normals, indices, a, b, radius);
            }
        }

        foreach (NetworkVertex vertex in network.Vertices)
        {
            AddCube(positions, normals, indices, vertex.Position, markerHalfSize);
        }

        var mesh = new ArrayMesh();
        if (positions.Count == 0)
        {
            return mesh;
        }

        var arrays = new Godot.Collections.Array();
        arrays.Resize((int)Mesh.ArrayType.Max);
        arrays[(int)Mesh.ArrayType.Vertex] = positions.ToArray();
        arrays[(int)Mesh.ArrayType.Normal] = normals.ToArray();
        arrays[(int)Mesh.ArrayType.Index] = indices.ToArray();
        mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, arrays);
        return mesh;
    }

    /// <summary>A square-cross-section box running from <paramref name="a"/> to <paramref name="b"/> —
    /// one segment of a chain's tube. No end caps: consecutive segments overlap at their shared vertex,
    /// which <see cref="AddCube"/>'s marker covers.</summary>
    private static void AddSegmentBox(List<Vector3> positions, List<Vector3> normals, List<int> indices, Vector3 a, Vector3 b, float radius)
    {
        Vector3 tangent = b - a;
        if (tangent.LengthSquared() < 1e-8f)
        {
            return;
        }

        tangent = tangent.Normalized();
        Vector3 reference = Mathf.Abs(tangent.Dot(Vector3.Up)) > 0.99f ? Vector3.Right : Vector3.Up;
        Vector3 right = tangent.Cross(reference).Normalized() * radius;
        Vector3 up = right.Normalized().Cross(tangent).Normalized() * radius;

        Vector3[] ringA = [a - right - up, a + right - up, a + right + up, a - right + up];
        Vector3[] ringB = [b - right - up, b + right - up, b + right + up, b - right + up];

        for (int side = 0; side < 4; side++)
        {
            int next = (side + 1) % 4;
            Vector3 normal = ((ringA[side] + ringB[side]) * 0.5f - (a + b) * 0.5f).Normalized();
            AddQuad(positions, normals, indices, ringA[side], ringA[next], ringB[next], ringB[side], normal);
        }
    }

    private static void AddCube(List<Vector3> positions, List<Vector3> normals, List<int> indices, Vector3 centre, float halfSize)
    {
        Vector3 e = Vector3.One * halfSize;
        Vector3[] c =
        [
            centre + new Vector3(-e.X, -e.Y, -e.Z), centre + new Vector3(e.X, -e.Y, -e.Z),
            centre + new Vector3(e.X, e.Y, -e.Z), centre + new Vector3(-e.X, e.Y, -e.Z),
            centre + new Vector3(-e.X, -e.Y, e.Z), centre + new Vector3(e.X, -e.Y, e.Z),
            centre + new Vector3(e.X, e.Y, e.Z), centre + new Vector3(-e.X, e.Y, e.Z),
        ];

        AddQuad(positions, normals, indices, c[0], c[1], c[2], c[3], Vector3.Back);
        AddQuad(positions, normals, indices, c[5], c[4], c[7], c[6], Vector3.Forward);
        AddQuad(positions, normals, indices, c[4], c[0], c[3], c[7], Vector3.Left);
        AddQuad(positions, normals, indices, c[1], c[5], c[6], c[2], Vector3.Right);
        AddQuad(positions, normals, indices, c[3], c[2], c[6], c[7], Vector3.Up);
        AddQuad(positions, normals, indices, c[4], c[5], c[1], c[0], Vector3.Down);
    }

    private static void AddQuad(List<Vector3> positions, List<Vector3> normals, List<int> indices, Vector3 a, Vector3 b, Vector3 c, Vector3 d, Vector3 normal)
    {
        int baseIndex = positions.Count;
        positions.Add(a);
        positions.Add(b);
        positions.Add(c);
        positions.Add(d);
        for (int i = 0; i < 4; i++)
        {
            normals.Add(normal);
        }

        indices.Add(baseIndex + 0);
        indices.Add(baseIndex + 1);
        indices.Add(baseIndex + 2);
        indices.Add(baseIndex + 0);
        indices.Add(baseIndex + 2);
        indices.Add(baseIndex + 3);
    }
}
