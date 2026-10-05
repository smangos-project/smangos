#pragma once

#include <string>

// Fullscreen post-process passes that run after the client has drawn the world and before it composites the world and
// draws the UI, with the world's depth buffer available. Nothing an effect does touches the UI.
//
// An effect is a ps_5_0 pixel shader with entry point "main":
//   float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
// bound to:
//   Texture2D SceneColor : register(t0);         the previous pass's colour
//   Texture2D<float> SceneDepth : register(t1);  post-projection depth (reversed-Z: 1 near, 0 far/sky)
//   SamplerState LinearClamp : register(s0);
//   SamplerState PointClamp : register(s1);
//   cbuffer Frame : register(b0) { float4 Viewport; float4 Params[4]; };  Viewport = width, height, 1/width, 1/height
// Effects run in ascending Order, ping-ponged between two textures; the last one writes the scene back.

// Fills Params (16 floats). Return false to skip the pass this frame.
using PostProcessSetup = bool (*)(float* params);
// Return false to drop the pass before any work is done. Null means always on.
using PostProcessGate = bool (*)();

struct PostProcessEffect {
    char const* name = "";
    char const* pixelShaderHlsl = nullptr;
    PostProcessSetup setup = nullptr;
    PostProcessGate isEnabled = nullptr;
    int order = 0;
};

void RegisterPostProcessEffect(PostProcessEffect effect);
void StartPostProcess();
void SetPostProcessParameter(std::string const& name, float value);
// Named settings set through the control server's /postprocess endpoint; effects read them in their gate and setup.
float GetPostProcessParameter(std::string const& name, float fallback);
