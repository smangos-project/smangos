add_executable(map_extractor
  Main.cpp
  Client/ClientStorage.cpp
  Client/ChunkedFile.cpp
  Client/Db2Tables.cpp
  Client/Options.cpp
  Cache/Manifest.cpp
  Map/MapStage.cpp
  Formats/Wdt.cpp
  Vmap/RawModels.cpp
  Vmap/VmapStage.cpp
  Mmap/MmapStage.cpp
  ${CMAKE_SOURCE_DIR}/contrib/mmap/src/IntermediateValues.cpp
  ${CMAKE_SOURCE_DIR}/contrib/mmap/src/MapBuilder.cpp
  ${CMAKE_SOURCE_DIR}/contrib/mmap/src/TerrainBuilder.cpp
  ${CMAKE_SOURCE_DIR}/contrib/mmap/src/TileWorker.cpp
  ${CMAKE_SOURCE_DIR}/contrib/mmap/src/VMapExtensions.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/BIH.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/MapTree.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/ModelInstance.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/TileAssembler.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/VMapManager2.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/WorldModel.cpp
)

target_include_directories(map_extractor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(map_extractor PRIVATE cxx_std_20)
target_compile_definitions(map_extractor PRIVATE MMAP_GENERATOR MAP_EXTRACTOR_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_include_directories(map_extractor PRIVATE ${CMAKE_SOURCE_DIR}/src/game/Maps ${CMAKE_SOURCE_DIR}/src/game/vmap ${CMAKE_SOURCE_DIR}/src/game)
target_include_directories(map_extractor PRIVATE ${CMAKE_SOURCE_DIR}/contrib/mmap/src ${CMAKE_SOURCE_DIR}/dep/recastnavigation ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast/Include)
target_link_libraries(map_extractor PRIVATE database_tool_lib g3dlite Detour Recast)
doltlite_copy_runtime(map_extractor)
