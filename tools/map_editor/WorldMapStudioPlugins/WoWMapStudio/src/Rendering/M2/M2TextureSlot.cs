namespace WorldMapStudio.WoW;

/// <summary>
/// Named M2 replaceable texture slot ids, matching <c>M2TextureDef.Type</c> — see
/// <see cref="M2Material.TextureSlot"/>'s own doc comment for the full correspondence. Plain
/// constants rather than an enum: a <see cref="MeshParameter"/> value is stored as a raw int, and a
/// texture-by-slot map (e.g. a creature's resolved skin) keys on these same raw values, not a CLR
/// enum type nothing upstream of it knows about.
/// </summary>
public static class M2TextureSlot
{
    /// <summary>Body/skin — the base replaceable texture on a character model.</summary>
    public const uint Skin = 1;

    /// <summary>Cape/item — "Item\ObjectComponents\Cape\*.blp".</summary>
    public const uint Cape = 2;

    public const uint Hair = 6;

    /// <summary>Tauren fur.</summary>
    public const uint Fur = 8;

    public const uint CreatureSkin1 = 11;

    public const uint CreatureSkin2 = 12;

    public const uint CreatureSkin3 = 13;
}
