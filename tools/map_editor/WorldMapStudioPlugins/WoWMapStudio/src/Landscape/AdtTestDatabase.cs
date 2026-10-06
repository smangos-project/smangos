
namespace WorldMapStudio.WoW;

/// <summary>Points a test context's editor storage at a scratch database so catalog id seeding has tables to read.</summary>
internal static class AdtTestDatabase
{
    private static readonly string ScratchPath = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "__wms_adt_tools_test__.doltlite");

    public static EditorContext Attach(EditorContext context)
    {
        EditorStorage storage = context.Database.EditorStorage;
        storage.Location.DatabasePath = ScratchPath;
        storage.EnsureSchema();
        return context;
    }
}
