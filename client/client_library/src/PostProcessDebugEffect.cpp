// Worked example of a post-process effect, and a check of the framework end to end. Off unless the "debug"
// parameter is set: 1 = vignette and cool tint, 2 = distance fog from the scene depth.

#include "PostProcess.hpp"

namespace {

char const* const DebugShader = R"(
Texture2D SceneColor : register(t0);
Texture2D<float> SceneDepth : register(t1);
SamplerState LinearClamp : register(s0);
SamplerState PointClamp : register(s1);
cbuffer Frame : register(b0) { float4 Viewport; float4 Params[4]; };

float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
{
    float4 color = SceneColor.Sample(LinearClamp, uv);
    if (Params[0].x < 1.5) {
        float2 d = uv - 0.5;
        color.rgb *= lerp(0.7, 1.0, saturate(1.0 - dot(d, d) * 1.5));
        color.rgb = lerp(color.rgb, color.rgb * float3(0.9, 0.95, 1.1), 0.15);
    } else {
        // Reversed-Z with an infinite far plane: view distance = near / depth.
        float distance = Params[0].y / max(SceneDepth.Sample(PointClamp, uv), 1e-7);
        color.rgb = lerp(color.rgb, float3(0.55, 0.65, 0.8), 1.0 - exp2(-distance / Params[0].z));
    }
    return color;
}
)";

bool DebugEnabled()
{
    return GetPostProcessParameter("debug", 0.f) != 0.f;
}

bool DebugSetup(float* params)
{
    params[0] = GetPostProcessParameter("debug", 0.f);
    params[1] = GetPostProcessParameter("near", 0.2f);
    params[2] = GetPostProcessParameter("fog", 40.f);
    return true;
}

struct Registration {
    Registration() { RegisterPostProcessEffect({"debug", DebugShader, DebugSetup, DebugEnabled, 1000}); }
};

Registration Instance;

} // namespace
