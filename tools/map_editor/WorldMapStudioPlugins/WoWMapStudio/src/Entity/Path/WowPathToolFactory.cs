using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>
/// Hands the shared <see cref="NetworkEditTool"/> to path authoring — the same reuse
/// <see cref="WowZoneLightToolFactory"/> makes for a zone light's ring. Works against any
/// <see cref="WowPathComponent{TNode}"/> via <see cref="IWowPathComponent"/>, so one tool serves a
/// taxi route and a waypoint chain alike. Self-registers into <see cref="ToolSystem"/>.
/// </summary>
[Subsystem(nameof(ToolSystem))]
public sealed class WowPathToolFactory : IToolFactory
{
    public float Priority => 22f;

    public string Name => "Path";

    public WowPathToolFactory(ToolSystem tools)
    {
    }

    public ITool Create(ToolContext context) =>
        new NetworkEditTool(context, "Path", entity => entity.ComponentsOf<IWowPathComponent>().FirstOrDefault());
}
