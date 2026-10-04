#ifndef MAP_EXTRACTOR_MMAP_MMAPSTAGE_H
#define MAP_EXTRACTOR_MMAP_MMAPSTAGE_H

#include "Context.h"

StageStats RunMmapStage(Context& ctx, std::vector<KnownMap const*> const& maps, TileRect const& region);

#endif
