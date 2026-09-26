add_executable(tests-game
  ${CMAKE_SOURCE_DIR}/src/game/Server/Packets/Classic/GuidMapping.cpp
  ${CMAKE_SOURCE_DIR}/src/game/Objects/Updates/UpdateField.cpp
  ${CMAKE_SOURCE_DIR}/src/game/Objects/Updates/UpdateFieldStructs.cpp
  ViewerDependentStubs.cpp
  ClassicGuidMappingTests.cpp
  ClassicClientTests.cpp
  UpdateFieldTests.cpp
  TestMain.cpp
)

# Compiles only the game sources it tests, not `game` itself, so it needs none of the world,
# script or database code.
target_include_directories(tests-game PRIVATE
  ${CMAKE_SOURCE_DIR}/src/game
  ${CMAKE_SOURCE_DIR}/src/game/Server
  ${CMAKE_SOURCE_DIR}/src/game/Server/Protocol
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
)

target_link_libraries(tests-game PRIVATE shared framework Catch2)

if (WIN32)
  target_link_libraries(tests-game PRIVATE
    optimized ${MYSQL_LIBRARY}
    optimized ${OPENSSL_LIBRARIES}
    debug ${MYSQL_DEBUG_LIBRARY}
    debug ${OPENSSL_DEBUG_LIBRARIES}
  )
endif()

if (UNIX)
  target_link_libraries(tests-game PRIVATE
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${OPENSSL_EXTRA_LIBRARIES}
  )
endif()

set_target_properties(tests-game PROPERTIES FOLDER "Tests")
mangos_copy_openssl_dlls(tests-game)

catch_discover_tests(tests-game TEST_PREFIX "game: " PROPERTIES LABELS "unit")
