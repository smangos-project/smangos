# Shader utilities

**Code:** `client/client_library/src/PostProcess.hpp`, `PostProcess.cpp`, `PostProcessDebugEffect.cpp`, `PostProcessOutlineEffect.cpp`

A small post-process framework in `custom.dll`. An effect is a `ps_5_0` pixel shader that runs on the world after the client draws it and before the UI is composited, with the scene colour and depth buffers as inputs. Effects run in order and never touch the UI.

## What you can do

- Add an effect by registering a `PostProcessEffect` with an HLSL pixel shader.
- Use the built-in effects: a debug effect (vignette and tint, distance fog from depth) and a depth-based outline effect.
- Switch and tune effects at runtime through `POST /postprocess` on the control server.
- Inspect how the client renders a frame with `GET /frametrace`, which lists one frame's command lists, render targets and draws.
