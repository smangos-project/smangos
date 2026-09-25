include(${CMAKE_SOURCE_DIR}/dep/catch2/extras/Catch.cmake)

add_subdirectory(unit/shared)
add_subdirectory(unit/game)

if (ENABLE_REALBOTS)
  add_subdirectory(unit/realbots)
  add_subdirectory(conformance)
endif()
