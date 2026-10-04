#ifndef MAP_EXTRACTOR_MAP_MAPSTAGE_H
#define MAP_EXTRACTOR_MAP_MAPSTAGE_H

#include "Context.h"

// Converts the root ADTs of `map` inside `region` to z1.4 .map files. Tiles whose inputs and
// output are unchanged are skipped.
StageStats RunMapStage(Context& ctx, KnownMap const& map, TileRect const& region);

// Source file of a tile's root ADT.
std::string RootAdtPath(Context const& ctx, KnownMap const& map, int row, int col);

#endif
