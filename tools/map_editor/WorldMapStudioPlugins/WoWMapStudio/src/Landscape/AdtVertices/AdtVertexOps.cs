using System;
using System.Numerics;

namespace WorldMapStudio.WoW;

/// <summary>Reads vertex values for an <see cref="IAdtVertexOp"/>, so a smoothing op can look at neighbours.</summary>
public interface IAdtVertexReader
{
    /// <summary>False when the vertex cannot be read: no chunk that holds it is loaded.</summary>
    bool TryGet(AdtVertex vertex, out Vector3 value);
}

/// <summary>One vertex under a dab: how strongly the brush reaches it, and where it is in the world.</summary>
public readonly record struct AdtVertexStep(AdtVertex Vertex, float WorldX, float WorldZ, float Amount, bool Invert);

/// <summary>
/// What a stroke does to each vertex under the brush. An op is pure: it sees the vertex's value and its
/// neighbours' values as they were before the dab and returns the new value, so the result does not depend on
/// the order the vertices are visited in.
/// </summary>
public interface IAdtVertexOp
{
    /// <summary>Called once, on a stroke's first dab, with the lattice vertex nearest the dab's centre.</summary>
    void Begin(AdtVertex nearest, IAdtVertexReader reader);

    Vector3 Apply(Vector3 current, in AdtVertexStep step, IAdtVertexReader reader);
}

/// <summary>Raises the ground, or lowers it when inverted, by <see cref="Rate"/> at full strength.</summary>
public sealed class AdtRaiseOp : IAdtVertexOp
{
    /// <summary>World units per dab at full strength.</summary>
    public float Rate { get; init; } = 0.5f;

    public void Begin(AdtVertex nearest, IAdtVertexReader reader)
    {
    }

    public Vector3 Apply(Vector3 current, in AdtVertexStep step, IAdtVertexReader reader) =>
        new(current.X + ((step.Invert ? -1.0f : 1.0f) * step.Amount * Rate), 0.0f, 0.0f);
}

/// <summary>Which side of the target height a flatten stroke is allowed to move vertices from.</summary>
public enum AdtFlattenSide
{
    Both,
    RaiseOnly,
    LowerOnly,
}

/// <summary>Pulls heights towards a target: the one given, or the height under the stroke's first dab.</summary>
public sealed class AdtFlattenOp : IAdtVertexOp
{
    private float _target;

    /// <summary>The height to flatten to. Null takes the height under the first dab.</summary>
    public float? Target { get; init; }

    public AdtFlattenSide Side { get; init; }

    /// <summary>The height this stroke flattens to, once it has begun.</summary>
    public float ResolvedTarget => _target;

    /// <summary>Whether the stroke has begun, so <see cref="ResolvedTarget"/> is settled.</summary>
    public bool Resolved { get; private set; }

    public void Begin(AdtVertex nearest, IAdtVertexReader reader)
    {
        _target = Target ?? (reader.TryGet(nearest, out Vector3 value) ? value.X : 0.0f);
        Resolved = true;
    }

    public Vector3 Apply(Vector3 current, in AdtVertexStep step, IAdtVertexReader reader)
    {
        bool below = current.X < _target;
        if ((Side == AdtFlattenSide.RaiseOnly && !below) || (Side == AdtFlattenSide.LowerOnly && below))
        {
            return current;
        }

        return new Vector3(current.X + ((_target - current.X) * step.Amount), 0.0f, 0.0f);
    }
}

/// <summary>Moves each vertex towards the average of itself and its neighbours: the four vertices of the other kind
/// diagonally around it, which the surface fans out to, and the eight of its own kind one cell away.</summary>
public sealed class AdtSmoothOp : IAdtVertexOp
{
    // Half-step offsets: (+-1, +-1) is the other kind of vertex, (+-2, +-2), (+-2, 0) and (0, +-2) the same kind.
    private static readonly (int Dx, int Dz)[] Neighbours =
    [
        (-1, -1), (1, -1), (1, 1), (-1, 1),
        (-2, -2), (0, -2), (2, -2), (-2, 0), (2, 0), (-2, 2), (0, 2), (2, 2),
    ];

    public void Begin(AdtVertex nearest, IAdtVertexReader reader)
    {
    }

    public Vector3 Apply(Vector3 current, in AdtVertexStep step, IAdtVertexReader reader)
    {
        Vector3 sum = current;
        int count = 1;
        foreach ((int dx, int dz) in Neighbours)
        {
            if (reader.TryGet(new AdtVertex(step.Vertex.X + dx, step.Vertex.Z + dz), out Vector3 neighbour))
            {
                sum += neighbour;
                count++;
            }
        }

        return Vector3.Lerp(current, sum / count, step.Amount);
    }
}

/// <summary>Moves values towards a colour or light. Inverted, it moves them towards the neutral value, which
/// is how the brush erases.</summary>
public sealed class AdtPaintOp : IAdtVertexOp
{
    private readonly Vector3 _neutral;

    public AdtPaintOp(Vector3 target, Vector3 neutral)
    {
        Target = target;
        _neutral = neutral;
    }

    public Vector3 Target { get; }

    public void Begin(AdtVertex nearest, IAdtVertexReader reader)
    {
    }

    public Vector3 Apply(Vector3 current, in AdtVertexStep step, IAdtVertexReader reader) =>
        Vector3.Lerp(current, step.Invert ? _neutral : Target, step.Amount);
}

/// <summary>Scales brightness and keeps the hue. Inverted, it darkens.</summary>
public sealed class AdtBrightnessOp : IAdtVertexOp
{
    /// <summary>Relative change per dab at full strength.</summary>
    public float Rate { get; init; } = 0.25f;

    public void Begin(AdtVertex nearest, IAdtVertexReader reader)
    {
    }

    public Vector3 Apply(Vector3 current, in AdtVertexStep step, IAdtVertexReader reader) =>
        current * MathF.Max(0.0f, 1.0f + ((step.Invert ? -1.0f : 1.0f) * step.Amount * Rate));
}

/// <summary>Moves the hue and saturation towards a tint and keeps the vertex's own brightness.</summary>
public sealed class AdtTintOp : IAdtVertexOp
{
    private static readonly Vector3 Luma = new(0.299f, 0.587f, 0.114f);

    public AdtTintOp(Vector3 tint)
    {
        Tint = tint;
    }

    public Vector3 Tint { get; }

    public void Begin(AdtVertex nearest, IAdtVertexReader reader)
    {
    }

    public Vector3 Apply(Vector3 current, in AdtVertexStep step, IAdtVertexReader reader)
    {
        float tintLuma = Vector3.Dot(Tint, Luma);
        if (tintLuma <= 1e-4f)
        {
            return current;
        }

        Vector3 matched = Tint * (Vector3.Dot(current, Luma) / tintLuma);
        return Vector3.Lerp(current, matched, step.Amount);
    }
}
