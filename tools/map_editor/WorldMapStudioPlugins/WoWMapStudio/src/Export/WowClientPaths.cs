using System;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>
/// Where the project keeps the client's data, read from the project's named paths.
/// </summary>
[Subsystem(nameof(EditorContext))]
public sealed class WowClientPaths : ISubsystem
{
    /// <summary>The project path that holds <see cref="Content"/>.</summary>
    public const string ContentKey = "wow.clientContent";

    private readonly EditorContext _context;

    public WowClientPaths(EditorContext context)
    {
        _context = context;
    }

    /// <summary>
    /// The client's loose-file overlay (<c>.client/Data/Content</c>). Everything the exporter writes is
    /// relative to this, so an exported tile lands at the path the client already reads
    /// (<c>World\Maps\&lt;Map&gt;\&lt;Map&gt;_X_Y.adt</c>) with no copy step in between. Null when the
    /// project does not set <see cref="ContentKey"/>.
    /// </summary>
    public string? Content =>
        _context.Project.Paths.TryGetValue(ContentKey, out string? path) && path.Length > 0 ? path : null;

    /// <summary>Absolute path of a client-relative file under <see cref="Content"/>, lowercased.</summary>
    public string ContentFile(string relative) =>
        ContentFile(Content ?? throw new InvalidOperationException(MissingContentMessage), relative);

    /// <summary>
    /// <see cref="ContentFile(string)"/> for code that already holds the content root. The client's
    /// content tree is stored lowercase, so every segment of <paramref name="relative"/> is folded
    /// (either separator is accepted); <paramref name="contentRoot"/> is left as given.
    /// </summary>
    public static string ContentFile(string contentRoot, string relative) =>
        Path.Combine(contentRoot, relative.ToLowerInvariant()
            .Replace('\\', Path.DirectorySeparatorChar)
            .Replace('/', Path.DirectorySeparatorChar));

    /// <summary>Why <see cref="Content"/> is unavailable.</summary>
    public static string MissingContentMessage => $"Set paths.{ContentKey} in the project config";
}
