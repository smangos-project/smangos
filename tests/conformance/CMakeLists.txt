set(REALBOTS_DIR ${CMAKE_SOURCE_DIR}/src/game/RealBots)

# Deliberately not linking `game` (same reasoning as contrib/realbotrunner/CMakeLists.txt and
# tests/unit/realbots/CMakeLists.txt): bots touch no server runtime state, so this needs only
# shared/framework/bnetproto/zlib/openssl, compiled directly from the same RealBots sources.
# Links Catch2NoMain (dep/catch2/CMakeLists.txt), not Catch2 -- TestMain.cpp supplies its own
# main() for --target/--config/--strict, which would collide with the amalgamated file's built-in
# one if both were linked in.
add_executable(tests-conformance
  ${REALBOTS_DIR}/RealBotConfig.cpp
  ${REALBOTS_DIR}/RealBotMgr.cpp
  ${REALBOTS_DIR}/RealBotTarget.cpp
  ${REALBOTS_DIR}/Core/RealBot.cpp
  ${REALBOTS_DIR}/Core/BotAccountLease.cpp
  ${REALBOTS_DIR}/Core/BotStage.cpp
  ${REALBOTS_DIR}/Core/BotPacketLog.cpp
  ${REALBOTS_DIR}/Behavior/BotBrain.cpp
  ${REALBOTS_DIR}/Behavior/BotContext.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskArgs.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskRegistry.cpp
  ${REALBOTS_DIR}/Behavior/Tasks/BuiltinBotTasks.cpp
  ${REALBOTS_DIR}/Coroutines/BotRoutine.cpp
  ${REALBOTS_DIR}/Coroutines/BotAwaitables.cpp
  ${REALBOTS_DIR}/Protocol/ClassicOpcodes.cpp
  ${REALBOTS_DIR}/Protocol/WorldHandshakeCrypto.cpp
  ${REALBOTS_DIR}/Net/BotConnector.cpp
  ${REALBOTS_DIR}/Net/BotWorldConnection.cpp
  ${REALBOTS_DIR}/Net/BotTlsStream.cpp
  ${REALBOTS_DIR}/Net/BotRestClient.cpp
  ${REALBOTS_DIR}/Net/BotBnetConnection.cpp
  ${REALBOTS_DIR}/Net/BotBnetLogin.cpp
  ${REALBOTS_DIR}/Net/BotWorldHandshake.cpp
  ${REALBOTS_DIR}/Net/BotCharSelect.cpp
  ${REALBOTS_DIR}/Net/BotInstanceConnect.cpp
  ${REALBOTS_DIR}/Net/BotLogin.cpp
  support/ConformanceRun.cpp
  support/TestBot.cpp
  login/LoginTests.cpp
  world_entry/WorldEntryTests.cpp
  framework/FrameworkTests.cpp
  TestMain.cpp
)
mangos_target_flags(tests-conformance)
set_target_properties(tests-conformance PROPERTIES FOLDER "Tests")

target_include_directories(tests-conformance PRIVATE
  ${CMAKE_SOURCE_DIR}/tests/support
  support
  ${REALBOTS_DIR}
  ${REALBOTS_DIR}/Core
  ${REALBOTS_DIR}/Behavior
  ${REALBOTS_DIR}/Behavior/Tasks
  ${REALBOTS_DIR}/Coroutines
  ${REALBOTS_DIR}/Protocol
  ${REALBOTS_DIR}/Net
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/shared
)

target_link_libraries(tests-conformance PRIVATE shared framework Catch2NoMain zlib bnetproto)

if (WIN32)
  target_link_libraries(tests-conformance PRIVATE
    optimized ${OPENSSL_LIBRARIES}
    debug ${OPENSSL_DEBUG_LIBRARIES}
    $<$<CONFIG:Debug>:${OPENSSL_SSL_DEBUG_LIBRARIES}>
    $<$<NOT:$<CONFIG:Debug>>:${OPENSSL_SSL_LIBRARIES}>
  )
endif()

if (UNIX)
  target_link_libraries(tests-conformance PRIVATE
    ${OPENSSL_LIBRARIES}
    ${OPENSSL_EXTRA_LIBRARIES}
  )
endif()

mangos_copy_openssl_dlls(tests-conformance)

# Never part of `ctest -L unit` (bot-plan.md's testing model: conformance talks to the network,
# unit doesn't) -- no catch_discover_tests call here at all. Run by hand:
#   tests-conformance --target vibemangos
# from this directory (targets/*.conf and expectations/*.txt are read as relative paths).
