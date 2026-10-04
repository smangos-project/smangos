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
)

target_include_directories(map_extractor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(map_extractor PRIVATE cxx_std_20)
target_compile_definitions(map_extractor PRIVATE MAP_EXTRACTOR_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_include_directories(map_extractor PRIVATE ${CMAKE_SOURCE_DIR}/src/game/Maps)
target_link_libraries(map_extractor PRIVATE database_tool_lib)
doltlite_copy_runtime(map_extractor)
