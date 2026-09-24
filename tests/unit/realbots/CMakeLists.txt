set(REALBOTS_DIR ${CMAKE_SOURCE_DIR}/src/game/RealBots)

add_executable(tests-realbots
  ${REALBOTS_DIR}/RealBotConfig.cpp
  ${REALBOTS_DIR}/RealBotTarget.cpp
  ${REALBOTS_DIR}/Core/RealBot.cpp
  ${REALBOTS_DIR}/Core/BotAccountLease.cpp
  ${REALBOTS_DIR}/Core/BotStage.cpp
  ${REALBOTS_DIR}/Core/BotPacketLog.cpp
  ${REALBOTS_DIR}/Protocol/ClassicOpcodes.cpp
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
  ${REALBOTS_DIR}/Net/BotBnetLogin.cpp
  ${REALBOTS_DIR}/Net/BotWorldHandshake.cpp
  ${REALBOTS_DIR}/Net/BotCharSelect.cpp
  ${REALBOTS_DIR}/Net/BotInstanceConnect.cpp
  ${REALBOTS_DIR}/Net/BotLogin.cpp
  CoroutineTests.cpp
  BotHookTests.cpp
  TestRoutineTests.cpp
  WorldHandshakeCryptoTests.cpp
  ClassicPacketsTests.cpp
  ClassicPacketsRoundTripTests.cpp
  BotWorldConnectionTests.cpp
  BotWorldHandshakeTests.cpp
  BotCharSelectTests.cpp
  BotLoginTests.cpp
  BotLoginLiveTests.cpp
  BotTlsStreamTests.cpp
  BotRestClientTests.cpp
  BotBnetConnectionTests.cpp
  BotBnetLoginTests.cpp
  RealBotTargetTests.cpp
  BotAccountLeaseTests.cpp
  TestMain.cpp
)

# Deliberately not linking `game`: RealBots touches no server runtime state, so its
# unit tests need only shared/framework, not the world/scripts/database that the
# rest of `game` pulls in (bot-plan.md, "Bots touch no server runtime state").
target_include_directories(tests-realbots PRIVATE
  ${CMAKE_SOURCE_DIR}/tests/support
  ${REALBOTS_DIR}
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

# [live]-tagged tests (BotLoginLiveTests.cpp) need a reachable real server and aren't part of
# the hermetic "unit" suite (bot-plan.md's testing model: unit tests make no network calls to a
# real server; that's what the conformance suite, not yet built, is for). TEST_SPEC excludes them
# from discovery entirely, so `ctest -L unit` never touches the network; run them by hand with
# `tests-realbots.exe "[live]"`.
catch_discover_tests(tests-realbots TEST_PREFIX "realbots: " PROPERTIES LABELS "unit" TEST_SPEC "~[live]")
