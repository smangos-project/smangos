using System;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// The environment role of a <see cref="WowLight"/>, auto-attached by <see cref="WowLightFactory"/>:
/// a pure projection of the owner's falloff and clear-weather <see cref="LightParams"/>.
/// </summary>
public sealed class WowLightComponent : SceneComponent, ISceneBoundsProvider, IEnvironmentSource, IEnvironmentVolume
{
    public const string Kind = "wow-light";

    // A global light must never stream out regardless of camera distance.
    private static readonly Aabb GlobalBounds =
        new(new Vector3(-1.0e6f, -1.0e6f, -1.0e6f), new Vector3(2.0e6f, 2.0e6f, 2.0e6f));

    private readonly CatalogEntityRegistry _catalog;

    public WowLightComponent(CatalogEntityRegistry catalog)
    {
        _catalog = catalog;
    }

    private WowLight Light => (WowLight)Entity;

    public override string TypeId => Kind;

    public override string DisplayName => "WoW Light";

    public override int ContentVersion => HashCode.Combine(Light.FalloffStart, Light.FalloffEnd, Light.IsDefault, Light.LightParams);

    public Aabb LocalBounds => Light.IsDefault
        ? GlobalBounds
        : new Aabb(Vector3.One * -Light.FalloffEnd, Vector3.One * Light.FalloffEnd * 2.0f);

    public bool ContributesChunkOwnership => !Light.IsDefault;

    public override SceneComponent Clone() => new WowLightComponent(_catalog);

    bool IEnvironmentSource.IsGlobal => Light.IsDefault;

    int IEnvironmentSource.BlendLayer => 2;

    float IEnvironmentVolume.InnerRadius => Light.FalloffStart;

    float IEnvironmentVolume.OuterRadius => Light.FalloffEnd;

    Color IEnvironmentVolume.InnerColor => CurrentParams() is { } p
        ? p.SampleColor(WowSkyColorChannel.LightGlobalAmbient, _lastSampledTime)
        : Colors.Gray;

    Color IEnvironmentVolume.OuterColor => CurrentParams() is { } p
        ? p.SampleColor(WowSkyColorChannel.LightGlobalDiffuse, _lastSampledTime)
        : Colors.White;

    private int _lastSampledTime;

    public float WeightAt(Vector3 worldPosition) => Light.IsDefault
        ? 0.0f
        : EnvironmentFalloff.Sphere(worldPosition.DistanceTo(Light.Transform.Origin), Light.FalloffStart, Light.FalloffEnd);

    public EnvironmentValues Evaluate(in EnvironmentTime time)
    {
        _lastSampledTime = WowSkyEvaluation.WowTimeOf(time);
        return CurrentParams() is { } p
            ? WowSkyEvaluation.Evaluate(p, _catalog, _lastSampledTime)
            : new EnvironmentValues();
    }

    private LightParams? CurrentParams() =>
        _catalog.OfType<LightParams>().FirstOrDefault(candidate => candidate.ParamsId == Light.LightParams);
}
