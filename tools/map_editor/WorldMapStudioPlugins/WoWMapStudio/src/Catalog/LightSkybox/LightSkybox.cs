namespace WorldMapStudio.WoW;

/// <summary>Flags on a <see cref="LightSkybox"/>, matching Light.dbc's LightSkybox flags.</summary>
[System.Flags]
public enum WowSkyboxFlags
{
    None = 0,

    /// <summary>Animation syncs with time of day (uses animation 0, time of day as a percentage).</summary>
    FullDay = 0x1,

    /// <summary>Also render the procedural sky, stars, sun and moons alongside the skybox model.</summary>
    Combine = 0x2,
}

/// <summary>
/// A `dbc_lightskybox` row as a catalog entity — a skybox model plus its <see cref="WowSkyboxFlags"/>,
/// shared by any number of <see cref="LightParams"/> via <see cref="LightParams.LightSkyboxId"/>
/// the same way a real `LightSkyboxID` foreign key is shared in retail data. Kept as its own catalog
/// rather than flattened onto <see cref="LightParams"/>, since editing a shared skybox in place would
/// otherwise silently change it for every other params set referencing the same row.
/// </summary>
public sealed class LightSkybox : CatalogEntity, ISavedCatalogEntity
{
    /// <summary>The game's own `dbc_lightskybox` id. Named <c>SkyboxId</c> rather than <c>Id</c> so it
    /// doesn't shadow <see cref="Entity.Id"/>, the base class's own unrelated auto-generated identity.</summary>
    public int SkyboxId { get; set; }

    /// <summary>Normalized to `.m2` on load (the DBC's own `Name` column uses the legacy `.mdx`
    /// extension, same as WMO doodad references) and stored that way from then on.</summary>
    public string ModelPath { get; set; } = "";

    public WowSkyboxFlags Flags { get; set; } = WowSkyboxFlags.None;

    /// <summary>Whether a row already exists for this <see cref="SkyboxId"/> in `dbc_lightskybox`.</summary>
    public bool IsSaved { get; set; }

    public override string DisplayName => $"{SkyboxId}: {(ModelPath.Length > 0 ? ModelPath : "(no model)")}";
}
