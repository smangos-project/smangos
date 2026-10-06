using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Turns procedural models whose output format is M2 or WMO into generated <c>.m2</c>/<c>.wmo</c> files plus
/// <see cref="EditorPlacement"/>s for <see cref="PlacementExtractor"/>. Builds Godot meshes, so main thread only.
/// </summary>
public static class WowProceduralPlacements
{
    /// <summary>The M2/WMO placements of <paramref name="entity"/>; empty when it is not a persisted procedural M2/WMO model.</summary>
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
            string formatId = editor.Procedural.ResolveFormatId(model, output.Slot);
            if ((formatId != M2Format.FormatId && formatId != WmoFormat.FormatId)
                || exporter.GetOrCreate(model, output, formatId) is not { } path)
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
