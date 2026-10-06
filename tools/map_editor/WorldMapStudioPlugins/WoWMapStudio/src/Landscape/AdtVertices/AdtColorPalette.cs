using System.Collections.Generic;
using System.Numerics;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>
/// The swatch controls of the vertex paint tool: a tint picker and a brightness slider, a row of preset swatches,
/// the recently used swatches, and a preview of what the ground will look like. Colour is shown in the client's own
/// terms: 100% is no change and 200% twice as bright, and light runs from 0 to 100%.
/// </summary>
public static class AdtColorPalette
{
    private const float SwatchSize = 20.0f;
    private const float PreviewSize = 28.0f;
    private const float PreviewGround = 0.5f;

    /// <summary>Draws the controls. True, with <paramref name="swatch"/> updated, when a person changed it.</summary>
    public static bool Draw(AdtVertexKind kind, ref AdtSwatch swatch, IReadOnlyList<AdtSwatch> recents)
    {
        bool changed = false;
        bool light = kind == AdtVertexKind.Light;
        AdtSwatch before = swatch;

        Vector3 tint = swatch.Tint;
        if (ImGui.ColorButton("##tint", new Vector4(tint, 1.0f), ImGuiColorEditFlags.NoTooltip, new Vector2(PreviewSize, PreviewSize)))
        {
            ImGui.OpenPopup("adt_tint_picker");
        }

        if (ImGui.IsItemHovered())
        {
            ImGui.SetTooltip("Tint");
        }

        if (ImGui.BeginPopup("adt_tint_picker"))
        {
            if (ImGui.ColorPicker3("##picker", ref tint, ImGuiColorEditFlags.PickerHueWheel | ImGuiColorEditFlags.NoSidePreview))
            {
                swatch = swatch with { Tint = AdtSwatch.Normalize(tint) };
            }

            ImGui.EndPopup();
        }

        ImGui.SameLine();
        float percent = swatch.Brightness * 100.0f;
        ImGui.SetNextItemWidth(120.0f);
        if (ImGui.SliderFloat("Brightness", ref percent, 0.0f, light ? 100.0f : 200.0f, "%.0f%%"))
        {
            swatch = swatch with { Brightness = percent / 100.0f };
        }

        ImGui.SameLine();
        DrawPreview(kind, swatch);

        foreach (AdtSwatchPreset preset in AdtSwatches.Presets(kind))
        {
            ImGui.SameLine();
            if (DrawSwatch($"##preset_{preset.Name}", kind, preset.Swatch, preset.Name))
            {
                swatch = preset.Swatch;
            }
        }

        if (recents.Count > 0)
        {
            ImGui.SameLine();
            ImGui.TextDisabled("|");
            for (int i = 0; i < recents.Count; i++)
            {
                ImGui.SameLine();
                if (DrawSwatch($"##recent_{i}", kind, recents[i], "Recent"))
                {
                    swatch = recents[i];
                }
            }
        }

        changed = !swatch.Equals(before);
        return changed;
    }

    /// <summary>What a swatch looks like on ground of middling grey.</summary>
    public static Vector4 PreviewColor(AdtVertexKind kind, AdtSwatch swatch)
    {
        Vector3 value = swatch.Value;
        Vector3 shown = kind == AdtVertexKind.Light ? new Vector3(PreviewGround) + value : value * PreviewGround;
        return new Vector4(Vector3.Clamp(shown, Vector3.Zero, Vector3.One), 1.0f);
    }

    private static void DrawPreview(AdtVertexKind kind, AdtSwatch swatch)
    {
        ImGui.ColorButton("##preview", PreviewColor(kind, swatch), ImGuiColorEditFlags.NoTooltip | ImGuiColorEditFlags.NoPicker, new Vector2(PreviewSize, PreviewSize));
        ImGui.SameLine();
        Vector3 value = swatch.Value;
        ImGui.TextDisabled(kind == AdtVertexKind.Light
            ? $"+{value.X:0.00} +{value.Y:0.00} +{value.Z:0.00}"
            : $"x{value.X:0.00} x{value.Y:0.00} x{value.Z:0.00}");
    }

    private static bool DrawSwatch(string id, AdtVertexKind kind, AdtSwatch swatch, string name)
    {
        bool clicked = ImGui.ColorButton(id, PreviewColor(kind, swatch), ImGuiColorEditFlags.NoTooltip, new Vector2(SwatchSize, SwatchSize));
        if (ImGui.IsItemHovered())
        {
            ImGui.SetTooltip(name);
        }

        return clicked;
    }
}
