#pragma once

// Lets map chunks carry up to 8 texture layers instead of the client's 4.
//
// The client only ever sees the first 4 MCLY entries. Layers 5-8 are kept aside
// per chunk, decoded into a second blend texture per render batch, and bound
// together with their diffuse textures to PS slots 22 (blend, rgba = layers 5-8)
// and 23-26 for a terrain shader that knows about them. Batches without extra
// layers get a zero blend texture at slot 22.
bool InstallTerrainLayerHooks();
