using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Turns water placements in the scene into world-space <see cref="WaterSurfacePatch"/>es for
/// <see cref="WowLiquidRasterizer"/>. Reads built Godot meshes, so it runs on the main thread only.
/// </summary>
public static class WowWaterPatches
{
    public static bool IsWater(ProceduralModel model) =>
        model.FunctionId is WaterSurfaceMeshFunction.FunctionId or RiverSurfaceMeshFunction.FunctionId;

    /// <summary>The patches of <paramref name="entity"/>'s water surfaces; empty when it is not a water placement.</summary>
    public static IEnumerable<WaterSurfacePatch> Gather(EditorContext editor, SceneEntity entity)
    {
        if (entity.Component<ProceduralComponent>()?.Model is not { } model || !IsWater(model))
        {
            yield break;
        }

        // fishable/fatigue are function parameters, not material values.
        MeshParameterValues parameters = MeshParameterValues.Parse(model.Parameters);
        bool fishable = parameters.GetBool(WaterAttributeParameters.Fishable);
        bool fatigue = parameters.GetBool(WaterAttributeParameters.Fatigue);

        foreach (ProceduralModelOutput output in editor.Procedural.Build(model).Models)
        {
            foreach (ModelPart part in output.Asset.Parts)
            {
                Transform3D world = entity.Transform * output.Transform * part.Transform;
                foreach (ModelSurface surface in part.Surfaces)
                {
                    if (surface.Material.TypeId == WowLiquidMaterialType.TypeId &&
                        TryBuild(editor, surface, world, fishable, fatigue) is { } patch)
                    {
                        yield return patch;
                    }
                }
            }
        }
    }

    private static WaterSurfacePatch? TryBuild(EditorContext editor, ModelSurface surface, Transform3D world, bool fishable, bool fatigue)
    {
        if (surface.Mesh.GetSurfaceCount() == 0)
        {
            return null;
        }

        Godot.Collections.Array arrays = surface.Mesh.SurfaceGetArrays(0);
        Vector3[] local = arrays[(int)Mesh.ArrayType.Vertex].AsVector3Array();
        int[] indices = arrays[(int)Mesh.ArrayType.Index].AsInt32Array();
        if (local.Length == 0 || indices.Length == 0)
        {
            return null;
        }

        var vertices = new Vector3[local.Length];
        for (int i = 0; i < local.Length; i++)
        {
            vertices[i] = world * local[i];
        }

        int liquidTypeId = surface.Material.Values.GetInt(WowLiquidParameters.LiquidTypeIdLookup);
        return new WaterSurfacePatch(liquidTypeId, WowLiquidCatalog.BasicType(editor, liquidTypeId), vertices, indices, fishable, fatigue);
    }
}
