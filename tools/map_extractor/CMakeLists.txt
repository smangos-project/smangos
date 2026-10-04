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
  ${CMAKE_SOURCE_DIR}/src/game/vmap/BIH.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/MapTree.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/ModelInstance.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/TileAssembler.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/VMapManager2.cpp
  ${CMAKE_SOURCE_DIR}/src/game/vmap/WorldModel.cpp
)

target_include_directories(map_extractor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(map_extractor PRIVATE cxx_std_20)
target_compile_definitions(map_extractor PRIVATE MAP_EXTRACTOR_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_include_directories(map_extractor PRIVATE ${CMAKE_SOURCE_DIR}/src/game/Maps ${CMAKE_SOURCE_DIR}/src/game/vmap ${CMAKE_SOURCE_DIR}/src/game)
target_link_libraries(map_extractor PRIVATE database_tool_lib g3dlite)
doltlite_copy_runtime(map_extractor)
