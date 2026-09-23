set(REALBOTS_DIR ${CMAKE_SOURCE_DIR}/src/game/RealBots)

add_executable(tests-realbots
  ${REALBOTS_DIR}/Core/RealBot.cpp
  ${REALBOTS_DIR}/Behavior/BotBrain.cpp
  ${REALBOTS_DIR}/Behavior/BotContext.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskArgs.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskRegistry.cpp
  ${REALBOTS_DIR}/Coroutines/BotRoutine.cpp
  ${REALBOTS_DIR}/Coroutines/BotAwaitables.cpp
  ${REALBOTS_DIR}/Protocol/WorldHandshakeCrypto.cpp
  ${REALBOTS_DIR}/Net/BotConnector.cpp
  ${REALBOTS_DIR}/Net/BotWorldConnection.cpp
  ${REALBOTS_DIR}/Net/BotTlsStream.cpp
  ${REALBOTS_DIR}/Net/BotRestClient.cpp
  ${REALBOTS_DIR}/Net/BotBnetConnection.cpp
  CoroutineTests.cpp
  TestRoutineTests.cpp
  WorldHandshakeCryptoTests.cpp
  BotWorldConnectionTests.cpp
  BotTlsStreamTests.cpp
  BotRestClientTests.cpp
  BotBnetConnectionTests.cpp
  TestMain.cpp
)

# Deliberately not linking `game`: RealBots touches no server runtime state, so its
# unit tests need only shared/framework, not the world/scripts/database that the
# rest of `game` pulls in (bot-plan.md, "Bots touch no server runtime state").
target_include_directories(tests-realbots PRIVATE
  ${CMAKE_SOURCE_DIR}/tests/support
  ${REALBOTS_DIR}/Core
  ${REALBOTS_DIR}/Behavior
  ${REALBOTS_DIR}/Coroutines
  ${REALBOTS_DIR}/Protocol
  ${REALBOTS_DIR}/Net
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/shared
)

target_link_libraries(tests-realbots PRIVATE shared framework Catch2 zlib bnetproto)

if (WIN32)
  target_link_libraries(tests-realbots PRIVATE
    optimized ${OPENSSL_LIBRARIES}
    debug ${OPENSSL_DEBUG_LIBRARIES}
    $<$<CONFIG:Debug>:${OPENSSL_SSL_DEBUG_LIBRARIES}>
    $<$<NOT:$<CONFIG:Debug>>:${OPENSSL_SSL_LIBRARIES}>
  )
endif()

if (UNIX)
  target_link_libraries(tests-realbots PRIVATE
    ${OPENSSL_LIBRARIES}
    ${OPENSSL_EXTRA_LIBRARIES}
  )
endif()

set_target_properties(tests-realbots PROPERTIES FOLDER "Tests")
mangos_copy_openssl_dlls(tests-realbots)

catch_discover_tests(tests-realbots TEST_PREFIX "realbots: " PROPERTIES LABELS "unit")
