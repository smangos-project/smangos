using System.IO;
using Godot;

namespace WorldMapStudio.WoW;

public static class ProceduralModelExporterTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Main)]
    public static void M2_export_writes_files_and_registers_ids()
    {
        string root = Path.Combine(Path.GetTempPath(), "wms-pme-" + System.Guid.NewGuid().ToString("N"));
        try
        {
            var mesh = new ArrayMesh();
            var arrays = new Godot.Collections.Array();
            arrays.Resize((int)Mesh.ArrayType.Max);
            arrays[(int)Mesh.ArrayType.Vertex] = new[] { new Vector3(0, 0, 0), new Vector3(1, 0, 0), new Vector3(1, 0, -1) };
            arrays[(int)Mesh.ArrayType.Normal] = new[] { Vector3.Up, Vector3.Up, Vector3.Up };
            arrays[(int)Mesh.ArrayType.TexUV] = new[] { new Vector2(0, 0), new Vector2(1, 0), new Vector2(1, 1) };
            arrays[(int)Mesh.ArrayType.Index] = new[] { 0, 1, 2 };
            mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, arrays);
            var asset = new ModelAsset("proc", [new ModelSurface("s", mesh, M2Material.Describe())]);

            var ids = WowFileDataIds.Parse([]);
            var exporter = new ProceduralModelExporter(root, ids);
            string? path = exporter.WriteM2("Tri_7", asset);

            Assert.AreEqual("World\\Generated\\Tri_7.m2", path);
            uint skinId = ids.IdOf("World\\Generated\\Tri_700.skin") ?? 0;
            Assert.IsTrue(skinId >= WowFileDataIds.CustomBase);
            string m2File = WowClientPaths.ContentFile(root, path!);
            Assert.IsTrue(File.Exists(m2File));
            Assert.IsTrue(File.Exists(WowClientPaths.ContentFile(root, "World\\Generated\\Tri_700.skin")));
            M2Document document = M2Reader.Read(File.ReadAllBytes(m2File), "Tri_7.m2")!;
            Assert.AreEqual(3, document.Vertices.Length);
        }
        finally
        {
            if (Directory.Exists(root))
            {
                Directory.Delete(root, true);
            }
        }
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Main)]
    public static void WMO_export_writes_root_and_group_and_registers_ids()
    {
        string root = Path.Combine(Path.GetTempPath(), "wms-pme-" + System.Guid.NewGuid().ToString("N"));
        try
        {
            var mesh = new ArrayMesh();
            var arrays = new Godot.Collections.Array();
            arrays.Resize((int)Mesh.ArrayType.Max);
            arrays[(int)Mesh.ArrayType.Vertex] = new[] { new Vector3(0, 0, 0), new Vector3(1, 0, 0), new Vector3(1, 0, -1) };
            arrays[(int)Mesh.ArrayType.Normal] = new[] { Vector3.Up, Vector3.Up, Vector3.Up };
            arrays[(int)Mesh.ArrayType.TexUV] = new[] { new Vector2(0, 0), new Vector2(1, 0), new Vector2(1, 1) };
            arrays[(int)Mesh.ArrayType.Index] = new[] { 0, 1, 2 };
            mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, arrays);
            var asset = new ModelAsset("proc", [new ModelSurface("s", mesh, WmoMaterial.Describe())]);

            var ids = WowFileDataIds.Parse([]);
            var exporter = new ProceduralModelExporter(root, ids);
            string? path = exporter.WriteWmo("Tri_7", asset);

            Assert.AreEqual("World\\Generated\\Tri_7.wmo", path);
            uint groupId = ids.IdOf("World\\Generated\\Tri_7_000.wmo") ?? 0;
            Assert.IsTrue(groupId >= WowFileDataIds.CustomBase);
            string m2File = WowClientPaths.ContentFile(root, path!);
            Assert.IsTrue(File.Exists(m2File));
            Assert.IsTrue(File.Exists(WowClientPaths.ContentFile(root, "World\\Generated\\Tri_7_000.wmo")));
            WmoRoot root2 = WmoRootReader.Read(File.ReadAllBytes(m2File), "Tri_7.wmo")!;
            Assert.AreEqual(1, root2.GroupCount);
        }
        finally
        {
            if (Directory.Exists(root))
            {
                Directory.Delete(root, true);
            }
        }
    }
}
