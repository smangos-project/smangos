using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>
/// Viewport header toggles that draw the extracted server data (height grid, collision, navmesh) for the
/// tiles around the camera. Tiles rebuild when the camera crosses into a new tile or when the extractor
/// rewrites a source file.
/// </summary>
[Subsystem(nameof(ViewportHeader))]
public sealed class WowServerPreviewViewportHeaderItem : IViewportHeaderItem
{
    private const int TileRadius = 1;
    private const double StaleCheckSeconds = 1.0;

    private sealed record Built(string Key, List<(WowServerMeshData Mesh, string Layer)> Meshes, WowServerFileStamp[] Stamps);

    private readonly EditorContext _context;
    private readonly Node3D _root = new() { Name = "WowServerPreview" };

    private bool _terrain;
    private bool _collision;
    private bool _navmesh;
    private bool _liquid;
    private string? _appliedKey;
    private string? _pendingKey;
    private Task<Built>? _task;
    private WowServerFileStamp[] _stamps = [];
    private double _lastStaleCheck;
    private string? _drift;

    public float Priority => 3f;

    public WowServerPreviewViewportHeaderItem(ViewportHeader header)
    {
        _context = header.Context;
        _context.Root.AddChild(_root);
    }

    public void Draw(in ViewportHeaderContext context)
    {
        ImGui.TextUnformatted("Server:");
        ImGui.SameLine();
        ImGui.Checkbox("Heights", ref _terrain);
        ImGui.SameLine();
        ImGui.Checkbox("Collision", ref _collision);
        ImGui.SameLine();
        ImGui.Checkbox("Navmesh", ref _navmesh);
        ImGui.SameLine();
        ImGui.Checkbox("Liquid", ref _liquid);
        ImGui.SameLine();
        if (ImGui.Button("Drift"))
        {
            _drift = MeasureDrift(context.Camera.GlobalTransform.Origin);
        }

        if (_drift != null)
        {
            ImGui.SameLine();
            ImGui.TextUnformatted(_drift);
        }

        Update(context.Camera.GlobalTransform.Origin);
    }

    private string MeasureDrift(Vector3 cameraGodot)
    {
        if (WowMapBindings.For(_context).Resolve(_context.Maps.CurrentMap) is not int wowMap)
        {
            return "no map binding";
        }

        Vector3 server = CreatureCoordinates.PositionFromGodot(cameraGodot);
        (int fileX, int fileY) = WowTileAddressing.TileAtServer(server.X, server.Y);
        return WowHeightDrift.Describe(_context, wowMap, fileX, fileY);
    }

    private void Update(Vector3 cameraGodot)
    {
        if (_task is { IsCompleted: true } task)
        {
            _task = null;
            if (task.IsCompletedSuccessfully)
            {
                Apply(task.Result);
            }
            else
            {
                GD.PushWarning($"server preview failed: {task.Exception?.GetBaseException().Message}");
                _appliedKey = task.Exception is null ? _appliedKey : _pendingKey;
            }
        }

        if (!(_terrain || _collision || _navmesh || _liquid))
        {
            Clear();
            return;
        }

        WowServerRoots roots = _context.WowServerPaths.Roots;
        if (WowMapBindings.For(_context).Resolve(_context.Maps.CurrentMap) is not int wowMap)
        {
            return;
        }

        Vector3 server = CreatureCoordinates.PositionFromGodot(cameraGodot);
        (int fileX, int fileY) = WowTileAddressing.TileAtServer(server.X, server.Y);
        string key = $"{wowMap}:{fileX}:{fileY}:{_terrain}{_collision}{_navmesh}{_liquid}";

        double now = Time.GetTicksMsec() / 1000.0;
        if (key == _appliedKey && now - _lastStaleCheck > StaleCheckSeconds)
        {
            _lastStaleCheck = now;
            if (!WowServerFileStamp.AllCurrent(_stamps))
            {
                _appliedKey = null;
            }
        }

        if (key == _appliedKey || _task != null)
        {
            return;
        }

        _pendingKey = key;
        bool terrain = _terrain, collision = _collision, navmesh = _navmesh, liquid = _liquid;
        _task = Task.Run(() => BuildTiles(key, roots, wowMap, fileX, fileY, terrain, collision, navmesh, liquid));
    }

    private static Built BuildTiles(string key, WowServerRoots roots, int wowMap, int fileX, int fileY, bool terrain, bool collision, bool navmesh, bool liquid)
    {
        var meshes = new List<(WowServerMeshData, string)>();
        var files = new List<string>();
        for (int y = fileY - TileRadius; y <= fileY + TileRadius; y++)
        {
            for (int x = fileX - TileRadius; x <= fileX + TileRadius; x++)
            {
                if (x < 0 || y < 0 || x >= WowTileAddressing.TilesPerMapEdge || y >= WowTileAddressing.TilesPerMapEdge)
                {
                    continue;
                }

                var tile = new WowServerTileKey(wowMap, x, y);
                if ((terrain || liquid) && roots.MapFile(tile) is { } mapFile)
                {
                    files.Add(mapFile);
                    if (File.Exists(mapFile))
                    {
                        WowMapTile map = WowMapTileReader.Read(File.ReadAllBytes(mapFile));
                        if (terrain)
                        {
                            meshes.Add((WowServerTerrainMesh.Build(map, x, y), "terrain"));
                        }

                        if (liquid && WowServerLiquidMesh.Build(map, x, y) is { } liquidMesh)
                        {
                            meshes.Add((liquidMesh, "liquid"));
                        }
                    }
                }

                if (collision && roots.VmapTileFile(tile) is { } vmapFile && roots.Vmaps is { } vmaps)
                {
                    files.Add(vmapFile);
                    if (File.Exists(vmapFile))
                    {
                        var spawns = WowVmapTileReader.Read(File.ReadAllBytes(vmapFile));
                        if (WowServerCollisionMesh.Build(spawns, name => WowServerModelCache.Get(vmaps, name)).Mesh is { } mesh)
                        {
                            meshes.Add((mesh, "collision"));
                        }
                    }
                }

                if (navmesh && roots.NavTileFile(tile) is { } navFile)
                {
                    files.Add(navFile);
                    if (File.Exists(navFile))
                    {
                        meshes.AddRange(WowServerNavmeshMesh.Build(WowNavMeshTileReader.Read(File.ReadAllBytes(navFile))).Select(m => (m, "navmesh")));
                    }
                }
            }
        }

        return new Built(key, meshes, WowServerFileStamp.Capture(files));
    }

    private void Apply(Built built)
    {
        Clear();
        foreach ((WowServerMeshData data, string layer) in built.Meshes)
        {
            var arrays = new Godot.Collections.Array();
            arrays.Resize((int)Mesh.ArrayType.Max);
            arrays[(int)Mesh.ArrayType.Vertex] = data.Vertices;
            arrays[(int)Mesh.ArrayType.Color] = data.Colors;
            if (data.Indices != null)
            {
                arrays[(int)Mesh.ArrayType.Index] = data.Indices;
            }

            var mesh = new ArrayMesh();
            mesh.AddSurfaceFromArrays(data.Primitive, arrays);
            var material = new StandardMaterial3D
            {
                ShadingMode = BaseMaterial3D.ShadingModeEnum.Unshaded,
                VertexColorUseAsAlbedo = true,
                CullMode = BaseMaterial3D.CullModeEnum.Disabled,
                Transparency = data.Translucent ? BaseMaterial3D.TransparencyEnum.Alpha : BaseMaterial3D.TransparencyEnum.Disabled,
            };
            _root.AddChild(new MeshInstance3D { Mesh = mesh, MaterialOverride = material, Name = layer });
        }

        _stamps = built.Stamps;
        _appliedKey = built.Key;
    }

    private void Clear()
    {
        foreach (Node child in _root.GetChildren())
        {
            _root.RemoveChild(child);
            child.QueueFree();
        }

        if (!(_terrain || _collision || _navmesh || _liquid))
        {
            _appliedKey = null;
        }
    }
}
