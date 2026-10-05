namespace WorldMapStudio.WoW;

/// <summary>
/// Named M2 attachment point ids (POSITION_SLOTS), matching <c>M2Attachment.Id</c> — the six an
/// equipped weapon/shield/helm/shoulders needs, plus the overhead-text anchor; the format defines many
/// more (back, particle points, ...) this project has no consumer for yet. Plain constants rather than
/// an enum, the same reasoning <see cref="M2TextureSlot"/> gives: a lookup keys on these as raw values,
/// not a CLR type. Ordinals are the format's own, not guessed.
/// </summary>
public static class M2AttachmentSlot
{
    /// <summary>Left wrist — where a shield attaches, not the left hand's own weapon grip.</summary>
    public const uint LeftWrist = 0;

    /// <summary>Right hand — the mainhand weapon.</summary>
    public const uint RightPalm = 1;

    /// <summary>Left hand — the offhand weapon, when the offhand item isn't a shield.</summary>
    public const uint LeftPalm = 2;

    /// <summary>Right shoulder — a shoulder item's own "right" half (`ItemDisplayInfo.ModelName2`).</summary>
    public const uint RightShoulder = 5;

    /// <summary>Left shoulder — a shoulder item's "left"/primary half (`ItemDisplayInfo.ModelName1`).</summary>
    public const uint LeftShoulder = 6;

    /// <summary>The single helm attachment point.</summary>
    public const uint Helmet = 11;

    /// <summary>Where overhead unit text (name, quest glyph) is anchored.</summary>
    public const uint PlayerName = 18;
}
