// Dark outlines where the scene depth jumps, e.g. around silhouettes. Off unless the "outline" parameter is set;
// "outline_threshold" is the relative distance change that counts as an edge.

#include "PostProcess.hpp"

namespace {

char const* const OutlineShader = R"(
Texture2D SceneColor : register(t0);
Texture2D<float> SceneDepth : register(t1);
SamplerState LinearClamp : register(s0);
SamplerState PointClamp : register(s1);
cbuffer Frame : register(b0) { float4 Viewport; float4 Params[4]; };

// Proportional to view distance for reversed-Z with an infinite far plane; the near plane cancels out in ratios.
float Distance(float2 uv)
{
    return 1.0 / max(SceneDepth.Sample(PointClamp, uv), 1e-6);
}

float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
{
    float4 color = SceneColor.Sample(LinearClamp, uv);
    float2 texel = Viewport.zw * Params[0].y;
    float center = Distance(uv);
    float nearest = min(min(Distance(uv + float2(texel.x, 0)), Distance(uv - float2(texel.x, 0))),
                        min(Distance(uv + float2(0, texel.y)), Distance(uv - float2(0, texel.y))));
    // Only the far side of an edge is darkened, so the outline sits just outside the nearer object.
    float edge = saturate((center - nearest) / nearest / Params[0].x - 1.0);
    color.rgb *= 1.0 - edge * Params[0].z;
    return color;
}
)";

bool OutlineEnabled()
{
    return GetPostProcessParameter("outline", 0.f) != 0.f;
}

bool OutlineSetup(float* params)
{
    params[0] = GetPostProcessParameter("outline_threshold", 0.1f);
    params[1] = GetPostProcessParameter("outline_width", 1.f);
    params[2] = GetPostProcessParameter("outline", 0.f);
    return true;
}

struct Registration {
    Registration() { RegisterPostProcessEffect({"outline", OutlineShader, OutlineSetup, OutlineEnabled, 100}); }
};

Registration Instance;

} // namespace
