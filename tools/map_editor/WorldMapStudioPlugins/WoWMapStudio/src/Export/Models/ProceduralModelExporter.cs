using System.Collections.Generic;
using System.IO;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Writes a procedural model's built output as client files, once per model per export run: every
/// placement of the same model reuses one generated file (cached by catalog record id and format id).
/// The output's <c>ArrayMesh</c> arrays are read here, so call it from the main thread.
/// Only M2 is file-backed so far.
/// </summary>
public sealed class ProceduralModelExporter
{
    private const string RelativeDirectory = "World\\Generated";

    private readonly string _outputRoot;
    private readonly WowFileDataIds _fileDataIds;
    private readonly Dictionary<(int ModelId, string FormatId), string?> _pathsByModelAndFormat = new();

    public ProceduralModelExporter(string outputRoot, WowFileDataIds fileDataIds)
    {
        _outputRoot = outputRoot;
        _fileDataIds = fileDataIds;
    }

    /// <summary>The client-relative path placements should reference, or null when the model has no
    /// catalog id yet, the format is not file-backed, or the output has no geometry.</summary>
    public string? GetOrCreate(ProceduralModel model, ProceduralModelOutput output, string formatId)
    {
        if (model.RecordId is not { } modelId)
        {
            return null;
        }

        var key = (modelId, formatId);
        if (_pathsByModelAndFormat.TryGetValue(key, out string? cached))
        {
            return cached;
        }

        string? path = formatId == M2Format.FormatId ? WriteM2(SanitizeFileName($"{model.Name}_{modelId}"), output.Asset) : null;
        _pathsByModelAndFormat[key] = path;
        return path;
    }

    /// <summary>Writes <paramref name="asset"/> as <c>name.m2</c> and <c>name00.skin</c> and registers both
    /// file data ids; the client-relative m2 path, or null when the asset has no surfaces.</summary>
    public string? WriteM2(string name, ModelAsset asset)
    {
        var surfaces = new List<ModelSurfaceInput>();
        foreach (ModelPart part in asset.Parts)
        {
            foreach (ModelSurface surface in part.Surfaces)
            {
                RawSurfaceArrays raw = ExtractRawArrays(surface, part.Transform);
                MeshParameterValues values = surface.Material.Values;
                string texture = values.GetTexture(M2Material.Texture);
                surfaces.Add(new ModelSurfaceInput(
                    raw.Positions, raw.Normals, raw.Uvs, raw.Indices,
                    texture.Length > 0 ? _fileDataIds.IdOf(texture) ?? 0 : 0,
                    values.GetChoice(M2Material.Blend),
                    values.GetBool(M2Material.TwoSided),
                    values.GetBool(M2Material.Unlit),
                    values.GetBool(M2Material.DepthWrite)));
            }
        }

        if (surfaces.Count == 0)
        {
            return null;
        }

        string m2Path = $"{RelativeDirectory}\\{name}.m2";
        string skinPath = $"{RelativeDirectory}\\{name}00.skin";
        _fileDataIds.Allocate(m2Path);
        uint skinId = _fileDataIds.Allocate(skinPath);

        (byte[] m2Bytes, byte[] skinBytes) = M2ModelWriter.Write(name, surfaces, skinId);
        File.WriteAllBytes(OutputFile(m2Path), m2Bytes);
        File.WriteAllBytes(OutputFile(skinPath), skinBytes);
        return m2Path;
    }

    private string OutputFile(string relative)
    {
        string path = WowClientPaths.ContentFile(_outputRoot, relative);
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        return path;
    }

    private readonly record struct RawSurfaceArrays(Vector3[] Positions, Vector3[] Normals, Vector2[] Uvs, int[] Indices);

    /// <summary>Reads one surface's geometry in the owning part's space; assumes one Godot mesh surface
    /// per <see cref="ModelSurface"/>, which every builtin procedural function produces.</summary>
    private static RawSurfaceArrays ExtractRawArrays(ModelSurface surface, Transform3D partTransform)
    {
        Godot.Collections.Array arrays = surface.Mesh.SurfaceGetArrays(0);
        var rawPositions = arrays[(int)Mesh.ArrayType.Vertex].AsVector3Array();
        var rawNormals = arrays[(int)Mesh.ArrayType.Normal].AsVector3Array();
        var rawUvs = arrays[(int)Mesh.ArrayType.TexUV].AsVector2Array();
        var rawIndices = arrays[(int)Mesh.ArrayType.Index].AsInt32Array();

        var positions = new Vector3[rawPositions.Length];
        var normals = new Vector3[rawPositions.Length];
        Basis normalBasis = partTransform.Basis.Orthonormalized();
        for (int i = 0; i < rawPositions.Length; i++)
        {
            positions[i] = partTransform * rawPositions[i];
            normals[i] = i < rawNormals.Length ? (normalBasis * rawNormals[i]).Normalized() : Vector3.Up;
        }

        Vector2[] uvs = rawUvs.Length == rawPositions.Length ? rawUvs : new Vector2[rawPositions.Length];
        int[] indices = rawIndices.Length > 0 ? rawIndices : SequentialIndices(rawPositions.Length);
        return new RawSurfaceArrays(positions, normals, uvs, indices);
    }

    private static int[] SequentialIndices(int count)
    {
        var indices = new int[count];
        for (int i = 0; i < count; i++)
        {
            indices[i] = i;
        }

        return indices;
    }

    private static string SanitizeFileName(string value)
    {
        foreach (char invalid in Path.GetInvalidFileNameChars())
        {
            value = value.Replace(invalid, '_');
        }

        return value.Length == 0 ? "model" : value;
    }
}
