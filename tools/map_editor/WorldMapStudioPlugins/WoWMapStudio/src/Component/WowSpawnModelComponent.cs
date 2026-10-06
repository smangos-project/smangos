using System;
using System.Threading.Tasks;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Draws a spawn (creature or game object) with its resolved model, or a coloured box until the
/// appearance and model have loaded or when there is none. Keyed by the spawn's template entry; the
/// model's origin is its feet, so unlike <see cref="ModelRendererComponent"/> it is not re-centred.
/// Attached by the spawn factories only, not through the "Add Component" UI.
/// </summary>
public sealed class WowSpawnModelComponent : SceneComponent, ISceneBoundsProvider, ISceneNodeComponent, ITransformPolicy, IMeshPickable
{
    private readonly string _typeId;
    private readonly string _displayName;
    private readonly Color _color;
    private readonly float _markerSize;
    private readonly AssetSystem _assets;
    private readonly MeshMaterialSystem _materials;
    private readonly Func<uint> _entryOf;
    private readonly Func<uint, Task<CreatureAppearance?>> _resolve;

    private uint? _pendingEntry;
    private uint? _resolvedEntry;
    private CreatureAppearance? _appearance;
    private string _pendingModelPath = "";

    public WowSpawnModelComponent(
        string typeId, string displayName, Color color, float markerSize,
        AssetSystem assets, MeshMaterialSystem materials,
        Func<uint> entryOf, Func<uint, Task<CreatureAppearance?>> resolve)
    {
        _typeId = typeId;
        _displayName = displayName;
        _color = color;
        _markerSize = markerSize;
        _assets = assets;
        _materials = materials;
        _entryOf = entryOf;
        _resolve = resolve;
    }

    public override string TypeId => _typeId;

    public override string DisplayName => _displayName;

    public override int ContentVersion => (int)_entryOf();

    public SelfRotation SelfRotation => SelfRotation.HeightOnly;

    public SelfScale SelfScale => SelfScale.Uniform;

    public Aabb LocalBounds => TryGetModel(out CreatureAppearance? appearance, out ModelAsset? model)
        ? new Aabb(model!.LocalBounds.Position * appearance!.Scale, model.LocalBounds.Size * appearance.Scale)
        : new Aabb(new Vector3(-0.5f, 0f, -0.5f) * _markerSize, Vector3.One * _markerSize);

    public override SceneComponent Clone() =>
        new WowSpawnModelComponent(_typeId, _displayName, _color, _markerSize, _assets, _materials, _entryOf, _resolve);

    public Node3D BuildNode()
    {
        if (!TryGetModel(out CreatureAppearance? appearance, out ModelAsset? model))
        {
            return new MeshInstance3D
            {
                Name = "Placeholder",
                Position = new Vector3(0f, _markerSize * 0.5f, 0f),
                Mesh = new BoxMesh { Size = Vector3.One * _markerSize },
                MaterialOverride = new StandardMaterial3D
                {
                    AlbedoColor = _color,
                    ShadingMode = BaseMaterial3D.ShadingModeEnum.Unshaded,
                },
            };
        }

        Node3D node = model!.Instantiate(_materials, appearance!.ToInstantiateOptions());
        node.Name = "SpawnModel";
        node.Scale = Vector3.One * appearance.Scale;
        return node;
    }

    private bool TryGetModel(out CreatureAppearance? appearance, out ModelAsset? model)
    {
        model = null;
        appearance = null;
        if (!TryGetAppearance(out appearance) || appearance!.ModelPath.Length == 0)
        {
            return false;
        }

        Task<ModelAsset?> task = _assets.LoadModelAssetAsync(appearance.ModelPath);
        if (task.IsCompletedSuccessfully)
        {
            model = task.Result;
            return model != null;
        }

        if (_pendingModelPath != appearance.ModelPath)
        {
            string path = _pendingModelPath = appearance.ModelPath;
            WorkQueue.Schedule("Refresh Spawn Model", async work =>
            {
                await task.ConfigureAwait(false);
                await work.SwitchToMain();
                if (_pendingModelPath == path)
                {
                    Owner?.RefreshRepresentation();
                }
            });
        }

        return false;
    }

    private bool TryGetAppearance(out CreatureAppearance? appearance)
    {
        uint entry = _entryOf();
        if (_resolvedEntry == entry)
        {
            appearance = _appearance;
            return appearance != null;
        }

        appearance = null;
        if (_pendingEntry != entry)
        {
            _pendingEntry = entry;
            Task<CreatureAppearance?> task = _resolve(entry);
            WorkQueue.Schedule("Resolve Spawn Appearance", async work =>
            {
                CreatureAppearance? resolved = await task.ConfigureAwait(false);
                await work.SwitchToMain();
                if (_pendingEntry == entry)
                {
                    _resolvedEntry = entry;
                    _appearance = resolved;
                    Owner?.RefreshRepresentation();
                }
            });
        }

        return false;
    }
}
