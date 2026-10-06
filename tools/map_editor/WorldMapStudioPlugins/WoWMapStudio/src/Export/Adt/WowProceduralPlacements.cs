using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Turns procedural models whose output format is M2 into generated <c>.m2</c> files plus
/// <see cref="EditorPlacement"/>s for <see cref="PlacementExtractor"/>. Builds Godot meshes, so main thread only.
/// </summary>
public static class WowProceduralPlacements
{
    /// <summary>The M2 placements of <paramref name="entity"/>; empty when it is not a persisted procedural M2 model.</summary>
    public static IEnumerable<EditorPlacement> Gather(EditorContext editor, ProceduralModelExporter exporter, SceneEntity entity)
    {
        if (entity.RecordId is not { } recordId
            || entity.Component<ProceduralComponent>()?.Model is not { } model
            || model.RecordId == null
            || WowWaterPatches.IsWater(model))
        {
            yield break;
        }

        foreach (ProceduralModelOutput output in editor.Procedural.Build(model).Models)
        {
            if (editor.Procedural.ResolveFormatId(model, output.Slot) != M2Format.FormatId
                || exporter.GetOrCreate(model, output, M2Format.FormatId) is not { } path)
            {
                continue;
            }

            // Stored placements sit at the bounds centre of a re-centred model; PlacementExtractor undoes that.
            Aabb bounds = output.Asset.LocalBounds;
            Transform3D transform = entity.Transform * output.Transform;
            yield return new EditorPlacement(recordId, path, new Transform3D(transform.Basis, transform.Origin + (transform.Basis * bounds.GetCenter())), bounds, 0);
            yield break;
        }
    }
}
