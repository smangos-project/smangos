using System;
using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>One (time, value) key of a <see cref="WowLightBand{T}"/>, in WoW's 0..2880 day units.</summary>
public readonly record struct WowLightBandKey<T>(int Time, T Value);

/// <summary>
/// Up to 16 (time, value) keys sampled over the WoW day with wrap-around interpolation: a query before the
/// first key interpolates from the last key forward through <c>time + WowDay.Duration</c>, so the curve
/// reads as continuous across the midnight boundary. Shared by both the 18 colour channels and the 6 float
/// channels a <see cref="LightParams"/> carries — only the lerp function differs.
/// </summary>
public sealed class WowLightBand<T>
{
    private readonly List<WowLightBandKey<T>> _keys = [];

    /// <summary>Keys in time order, as authored. Not enforced sorted by this type — callers add in order.</summary>
    public IReadOnlyList<WowLightBandKey<T>> Keys => _keys;

    public void Add(int time, T value) => _keys.Add(new WowLightBandKey<T>(time, value));

    public void Clear() => _keys.Clear();

    public void ReplaceKeys(IEnumerable<WowLightBandKey<T>> keys)
    {
        _keys.Clear();
        _keys.AddRange(keys);
    }

    /// <summary>
    /// Samples the band at <paramref name="time"/> (any int; wraps to 0..<see cref="WowDay.Duration"/>
    /// via reverse interpolation), or <paramref name="empty"/> if there are no keys.
    /// </summary>
    public T Sample(int time, T empty, Func<T, T, float, T> lerp)
    {
        if (_keys.Count == 0)
        {
            return empty;
        }

        int last = _keys.Count - 1;
        T c1, c2;
        int t1, t2;

        if (time < _keys[0].Time)
        {
            // Reverse interpolate: the query falls before the first key, so it's between the last
            // key of "yesterday" and the first key of "today", DayDuration apart.
            c1 = _keys[last].Value;
            c2 = _keys[0].Value;
            t1 = _keys[last].Time;
            t2 = _keys[0].Time + WowDay.Duration;
            time += WowDay.Duration;
        }
        else
        {
            int i = last;
            while (_keys[i].Time > time)
            {
                i--;
            }

            c1 = _keys[i].Value;
            t1 = _keys[i].Time;

            if (i == last)
            {
                c2 = _keys[0].Value;
                t2 = _keys[0].Time + WowDay.Duration;
            }
            else
            {
                c2 = _keys[i + 1].Value;
                t2 = _keys[i + 1].Time;
            }
        }

        float t = (time - t1) / (float)(t2 - t1);
        return lerp(c1, c2, t);
    }
}
