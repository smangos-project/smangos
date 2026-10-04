#ifndef MAP_EXTRACTOR_VMAP_VMAPSTAGE_H
#define MAP_EXTRACTOR_VMAP_VMAPSTAGE_H

#include "Context.h"

// Converts the WMO/M2 models placed on `maps` (inside `region`) to raw models under <output>/Buildings
// and assembles vmtree/vmtile/vmo files in <output>/vmaps. Also exports the gameobject models.
// `done` counts redone models and maps, `missingInput` models that could not be read.
StageStats RunVmapStage(Context& ctx, std::vector<KnownMap const*> const& maps, TileRect const& region);

#endif
