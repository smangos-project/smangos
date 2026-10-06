using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Reads the argument shapes the ADT script modules share: chunks addressed as
/// <c>{ TileX, TileY, Col, Row }</c> or <c>[tileX, tileY, col, row]</c>, stroke points as <c>[x, z]</c>, and the
/// option maps that carry them.</summary>
public static class AdtScriptArguments
{
    public static ChunkCoord ChunkAt(int tileX, int tileY, int col, int row) =>
        WowTileAddressing.ChunkAt(new WowTileId("", tileX, tileY), col, row);

    public static IReadOnlyList<ChunkCoord> Chunks(object? value)
    {
        if (value is not IEnumerable items || value is string)
        {
            throw new ArgumentException("Chunks must be an array of { TileX, TileY, Col, Row } or [tileX, tileY, col, row].");
        }

        var result = new List<ChunkCoord>();
        foreach (object? item in items)
        {
            if (ScriptJson.AsMap(item) is { } map)
            {
                result.Add(ChunkAt(
                    (int)Number(map, "tileX"), (int)Number(map, "tileY"), (int)Number(map, "col"), (int)Number(map, "row")));
            }
            else if (Numbers(item) is { Length: 4 } parts)
            {
                result.Add(ChunkAt((int)parts[0], (int)parts[1], (int)parts[2], (int)parts[3]));
            }
            else
            {
                throw new ArgumentException("A chunk must be { TileX, TileY, Col, Row } or [tileX, tileY, col, row].");
            }
        }

        return result;
    }

    public static List<Vector3> Points(object? points)
    {
        if (points is not IEnumerable items || points is string)
        {
            throw new ArgumentException("Points must be an array of [x, z].");
        }

        var result = new List<Vector3>();
        foreach (object? item in items)
        {
            double[] values = Numbers(item);
            result.Add(values.Length switch
            {
                2 => new Vector3((float)values[0], 0.0f, (float)values[1]),
                3 => new Vector3((float)values[0], (float)values[1], (float)values[2]),
                _ => throw new ArgumentException("Each point must be [x, z] or [x, y, z]."),
            });
        }

        return result;
    }

    public static object? Find(IDictionary<string, object?> map, string key) =>
        map.FirstOrDefault(pair => string.Equals(pair.Key, key, StringComparison.OrdinalIgnoreCase)).Value;

    public static double Number(IDictionary<string, object?> map, string key) =>
        Find(map, key) is { } value
            ? BrushScriptOptions.ToDouble(value)
            : throw new ArgumentException($"Missing '{key}'.");

    public static double[] Numbers(object? value) =>
        value is IEnumerable items && value is not string
            ? items.Cast<object?>().Select(BrushScriptOptions.ToDouble).ToArray()
            : [];
}
