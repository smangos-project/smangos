using System;
using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>One chunk's texture set as it is stored: the texture keys in slot order and the editor-oriented
/// RGBA alpha block (slot 1 to R, slot 2 to G, slot 3 to B).</summary>
public readonly record struct AdtStoredChunk(IReadOnlyList<string> Textures, byte[] Rgba, bool SlotsChanged);

/// <summary>
/// One MCNK's texture stack edited the way a texture painter expects: up to four textures, each with a
/// per-texel weight, the weights summing to 1. Painting a texture finds its slot, or adds it (freeing a
/// slot from a layer nothing shows when the chunk is full). Loading converts the stored lerp alphas to
/// weights (<see cref="AdtBlendSpace"/>); storing converts back, and only for texels an edit reached, so a
/// chunk that was only looked at stores back byte for byte.
/// </summary>
public sealed class AdtChunkTextureSet
{
    /// <summary>Alpha texels along one chunk edge.</summary>
    public const int Edge = TextureMapper.AlphaEdge;

    public const int TexelCount = Edge * Edge;

    public const int MaxLayers = AdtBlendSpace.Layers;

    /// <summary>A layer whose strongest texel is below this is invisible: one alpha step.</summary>
    public const float VisibleThreshold = 1.0f / 255.0f;

    private const float Epsilon = 1e-6f;

    private readonly List<string> _textures = [];
    private readonly float[][] _weights;
    private readonly bool[] _changed = new bool[TexelCount];
    private byte[]? _original;
    private bool _slotsChanged;

    private AdtChunkTextureSet()
    {
        _weights = new float[MaxLayers][];
        for (int layer = 0; layer < MaxLayers; layer++)
        {
            _weights[layer] = new float[TexelCount];
        }
    }

    public int Count => _textures.Count;

    public IReadOnlyList<string> Textures => _textures;

    /// <summary>Whether the slot list changed, which re-encodes the whole block on store.</summary>
    public bool SlotsChanged => _slotsChanged;

    public bool IsFull => _textures.Count >= MaxLayers;

    /// <summary>Builds a set from its stored form. <paramref name="rgba"/> is not modified.</summary>
    public static AdtChunkTextureSet FromStored(IReadOnlyList<string> textures, byte[] rgba)
    {
        if (rgba.Length != TexelCount * 4)
        {
            throw new ArgumentException($"An alpha block is {TexelCount * 4} bytes.", nameof(rgba));
        }

        var set = new AdtChunkTextureSet { _original = (byte[])rgba.Clone() };
        for (int slot = 0; slot < textures.Count && slot < MaxLayers; slot++)
        {
            set._textures.Add(textures[slot]);
        }

        int count = set._textures.Count;
        Span<float> weights = stackalloc float[MaxLayers];
        for (int i = 0; i < TexelCount; i++)
        {
            if (count == 0)
            {
                continue;
            }

            int offset = i * 4;
            AdtBlendSpace.ToWeights(
                count > 1 ? rgba[offset] : (byte)0,
                count > 2 ? rgba[offset + 1] : (byte)0,
                count > 3 ? rgba[offset + 2] : (byte)0,
                weights);
            for (int layer = 0; layer < MaxLayers; layer++)
            {
                set._weights[layer][i] = weights[layer];
            }
        }

        return set;
    }

    /// <summary>The stored form. Texels no edit reached keep their original bytes; every texel is
    /// re-encoded when the slot list changed, since the channels then mean something else.</summary>
    public AdtStoredChunk ToStored()
    {
        var rgba = new byte[TexelCount * 4];
        _original?.CopyTo(rgba, 0);

        Span<float> weights = stackalloc float[MaxLayers];
        for (int i = 0; i < TexelCount; i++)
        {
            if (!_slotsChanged && !_changed[i])
            {
                continue;
            }

            float sum = 0.0f;
            for (int layer = 0; layer < MaxLayers; layer++)
            {
                weights[layer] = layer < Count ? Math.Max(_weights[layer][i], 0.0f) : 0.0f;
                sum += weights[layer];
            }

            if (sum > Epsilon)
            {
                for (int layer = 0; layer < MaxLayers; layer++)
                {
                    weights[layer] /= sum;
                }
            }
            else
            {
                weights[0] = 1.0f;
            }

            AdtBlendSpace.ToAlphas(weights, out byte a1, out byte a2, out byte a3);
            int offset = i * 4;
            rgba[offset] = a1;
            rgba[offset + 1] = a2;
            rgba[offset + 2] = a3;
        }

        return new AdtStoredChunk(_textures.ToArray(), rgba, _slotsChanged);
    }

    public int IndexOf(string texture) => _textures.FindIndex(existing => AdtTexturePath.Same(existing, texture));

    public float WeightAt(int slot, int x, int y) => _weights[slot][(y * Edge) + x];

    /// <summary>The slot with the most weight at a texel, or -1 for an empty chunk.</summary>
    public int DominantAt(int x, int y)
    {
        int best = -1;
        float bestWeight = 0.0f;
        for (int slot = 0; slot < Count; slot++)
        {
            float weight = _weights[slot][(y * Edge) + x];
            if (weight > bestWeight)
            {
                bestWeight = weight;
                best = slot;
            }
        }

        return best;
    }

    public float MaxWeight(int slot)
    {
        float max = 0.0f;
        foreach (float weight in _weights[slot])
        {
            max = Math.Max(max, weight);
        }

        return max;
    }

    /// <summary>The share of the chunk's texels this layer accounts for, 0 to 1.</summary>
    public float Coverage(int slot)
    {
        double total = 0.0;
        foreach (float weight in _weights[slot])
        {
            total += weight;
        }

        return (float)(total / TexelCount);
    }

    /// <summary>Whether <see cref="GetOrAdd"/> could give <paramref name="texture"/> a slot.</summary>
    public bool CanAdd(string texture) =>
        IndexOf(texture) >= 0 || !IsFull || HasUnused(VisibleThreshold);

    public bool HasUnused(float threshold)
    {
        if (Count <= 1)
        {
            return false;
        }

        for (int slot = 0; slot < Count; slot++)
        {
            if (MaxWeight(slot) < threshold)
            {
                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// The slot holding <paramref name="texture"/>, adding it on top when it is not there: appended while
    /// there is room, otherwise after freeing a slot from a layer nothing shows. -1 when it cannot be added,
    /// and always -1 for a texture that is absent and <paramref name="strength"/> is not above 0.
    /// </summary>
    public int GetOrAdd(string texture, float strength)
    {
        int existing = IndexOf(texture);
        if (existing >= 0)
        {
            return existing;
        }

        if (strength <= 0.0f)
        {
            return -1;
        }

        if (IsFull)
        {
            EraseUnused(VisibleThreshold);
            if (IsFull)
            {
                return -1;
            }
        }

        _textures.Add(texture);
        _slotsChanged = true;
        if (Count == 1)
        {
            Array.Fill(_weights[0], 1.0f);
        }

        return Count - 1;
    }

    /// <summary>
    /// Moves the weight at every texel the dab covers on <paramref name="slot"/> towards
    /// <paramref name="targetWeight"/>, by the dab's strength times the brush falloff, taking it from (or
    /// handing it to) the other layers in proportion. An inverted dab paints towards 0. True when a texel
    /// changed.
    /// </summary>
    public bool Paint(int slot, in BrushGrid grid, in BrushDab dab, float targetWeight)
    {
        float target = dab.Invert ? 0.0f : Math.Clamp(targetWeight, 0.0f, 1.0f);
        float rate = Math.Clamp(dab.Strength * dab.Pressure, 0.0f, 1.0f);
        bool changed = false;
        BrushRaster.Walk(grid, dab, (x, y, falloff) =>
        {
            changed |= PaintTexel(slot, (y * Edge) + x, target, rate * falloff);
        });

        return changed;
    }

    /// <summary>Drops every layer whose strongest texel is under <paramref name="threshold"/>, so a chunk
    /// does not keep textures nothing shows. True when a layer was dropped.</summary>
    public bool EraseUnused(float threshold)
    {
        bool any = false;
        for (int slot = Count - 1; slot >= 0 && Count > 1; slot--)
        {
            if (MaxWeight(slot) < threshold)
            {
                any |= Remove(slot);
            }
        }

        return any;
    }

    /// <summary>Removes a layer and gives its weight to the others in proportion, the ones above shifting
    /// down. A chunk always keeps one layer.</summary>
    public bool Remove(int slot)
    {
        if (slot < 0 || slot >= Count || Count <= 1)
        {
            return false;
        }

        float[] removed = _weights[slot];
        int fallback = slot > 0 ? slot - 1 : 1;
        for (int i = 0; i < TexelCount; i++)
        {
            float weight = removed[i];
            if (weight <= 0.0f)
            {
                continue;
            }

            float others = 0.0f;
            for (int layer = 0; layer < Count; layer++)
            {
                if (layer != slot)
                {
                    others += _weights[layer][i];
                }
            }

            if (others > Epsilon)
            {
                float scale = 1.0f + (weight / others);
                for (int layer = 0; layer < Count; layer++)
                {
                    if (layer != slot)
                    {
                        _weights[layer][i] *= scale;
                    }
                }
            }
            else
            {
                _weights[fallback][i] += weight;
            }
        }

        RemoveSlot(slot);
        return true;
    }

    /// <summary>Swaps two layers' stacking order. Each texture keeps its weights, so the chunk looks the
    /// same and only the stored channels change.</summary>
    public bool Swap(int a, int b)
    {
        if (a == b || a < 0 || b < 0 || a >= Count || b >= Count)
        {
            return false;
        }

        (_textures[a], _textures[b]) = (_textures[b], _textures[a]);
        (_weights[a], _weights[b]) = (_weights[b], _weights[a]);
        _slotsChanged = true;
        return true;
    }

    /// <summary>Adds layer <paramref name="from"/>'s weight to layer <paramref name="into"/> and removes it.</summary>
    public bool Merge(int into, int from)
    {
        if (into == from || into < 0 || from < 0 || into >= Count || from >= Count)
        {
            return false;
        }

        for (int i = 0; i < TexelCount; i++)
        {
            _weights[into][i] += _weights[from][i];
            _weights[from][i] = 0.0f;
        }

        RemoveSlot(from);
        return true;
    }

    /// <summary>Merges layers that hold the same texture. Returns how many were merged away.</summary>
    public int RemoveDuplicates()
    {
        int merged = 0;
        for (int i = 0; i < Count; i++)
        {
            for (int j = Count - 1; j > i; j--)
            {
                if (AdtTexturePath.Same(_textures[i], _textures[j]) && Merge(i, j))
                {
                    merged++;
                }
            }
        }

        return merged;
    }

    /// <summary>
    /// Swaps <paramref name="from"/> for <paramref name="to"/>. Without a mask it is the whole chunk: the
    /// layer is renamed, or merged into <paramref name="to"/> when that is already present. With one, only
    /// the masked texels' weight moves, adding <paramref name="to"/> if it needs a slot. False when nothing
    /// could be replaced.
    /// </summary>
    public bool Replace(string from, string to, bool[]? mask = null)
    {
        int source = IndexOf(from);
        if (source < 0 || AdtTexturePath.Same(from, to))
        {
            return false;
        }

        if (mask == null)
        {
            int existing = IndexOf(to);
            if (existing < 0)
            {
                _textures[source] = to;
                _slotsChanged = true;
                return true;
            }

            return Merge(existing, source);
        }

        int target = GetOrAdd(to, 1.0f);
        if (target < 0)
        {
            return false;
        }

        source = IndexOf(from);
        bool moved = false;
        for (int i = 0; i < TexelCount; i++)
        {
            if (!mask[i] || _weights[source][i] <= 0.0f)
            {
                continue;
            }

            _weights[target][i] += _weights[source][i];
            _weights[source][i] = 0.0f;
            _changed[i] = true;
            moved = true;
        }

        return moved;
    }

    private bool PaintTexel(int slot, int i, float target, float rate)
    {
        float current = _weights[slot][i];
        float next = Math.Clamp(current + ((target - current) * rate), 0.0f, 1.0f);
        float delta = next - current;
        if (MathF.Abs(delta) < Epsilon)
        {
            return false;
        }

        float others = 0.0f;
        for (int layer = 0; layer < Count; layer++)
        {
            if (layer != slot)
            {
                others += _weights[layer][i];
            }
        }

        _weights[slot][i] = next;
        if (others > Epsilon)
        {
            float scale = (1.0f - next) / others;
            for (int layer = 0; layer < Count; layer++)
            {
                if (layer != slot)
                {
                    _weights[layer][i] *= scale;
                }
            }
        }
        else if (delta < 0.0f && Count > 1)
        {
            _weights[slot == 0 ? 1 : 0][i] += 1.0f - next;
        }

        if (delta > 0.0f)
        {
            for (int layer = 0; layer < Count; layer++)
            {
                if (layer != slot && _weights[layer][i] is > 0.0f and < VisibleThreshold)
                {
                    _weights[slot][i] += _weights[layer][i];
                    _weights[layer][i] = 0.0f;
                }
            }
        }

        _changed[i] = true;
        return true;
    }

    private void RemoveSlot(int slot)
    {
        float[] freed = _weights[slot];
        for (int layer = slot; layer < MaxLayers - 1; layer++)
        {
            _weights[layer] = _weights[layer + 1];
        }

        Array.Clear(freed);
        _weights[MaxLayers - 1] = freed;
        _textures.RemoveAt(slot);
        _slotsChanged = true;
    }
}
