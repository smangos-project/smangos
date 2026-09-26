set(REALBOTS_DIR ${CMAKE_SOURCE_DIR}/src/game/RealBots)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/src/game/Server)
set(SERVER_PROTOCOL_DIR ${SERVER_DIR}/Protocol)

add_executable(tests-realbots
  ${REALBOTS_DIR}/RealBotCommands.cpp
  ${REALBOTS_DIR}/RealBotConfig.cpp
  ${REALBOTS_DIR}/RealBotMgr.cpp
  ${REALBOTS_DIR}/RealBotTarget.cpp
  ${REALBOTS_DIR}/Core/RealBot.cpp
  ${REALBOTS_DIR}/Core/BotAccountLease.cpp
  ${REALBOTS_DIR}/Core/BotStage.cpp
  ${REALBOTS_DIR}/Core/BotPacketLog.cpp
  ${SERVER_PROTOCOL_DIR}/ClassicOpcodes.cpp
  ${REALBOTS_DIR}/Behavior/BotBrain.cpp
  ${REALBOTS_DIR}/Behavior/BotContext.cpp
  ${REALBOTS_DIR}/Behavior/BotWorldView.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskArgs.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskRegistry.cpp
  ${REALBOTS_DIR}/Behavior/Tasks/BuiltinBotTasks.cpp
  ${REALBOTS_DIR}/Coroutines/BotRoutine.cpp
  ${REALBOTS_DIR}/Coroutines/BotAwaitables.cpp
  ${REALBOTS_DIR}/Net/BotConnector.cpp
  ${REALBOTS_DIR}/Net/BotWorldConnection.cpp
  ${REALBOTS_DIR}/Net/BotRestClient.cpp
  ${REALBOTS_DIR}/Net/BotBnetConnection.cpp
  ${REALBOTS_DIR}/Net/BotBnetLogin.cpp
  ${REALBOTS_DIR}/Net/BotWorldLogin.cpp
  ${REALBOTS_DIR}/Net/BotLogin.cpp
  CoroutineTests.cpp
  BotHookTests.cpp
  PercentileTests.cpp
  BotReconnectTests.cpp
  BotStageWatchdogTests.cpp
  BotDispatchOrderTests.cpp
  TestRoutineTests.cpp
  ClassicOpcodesTests.cpp
  ClassicPacketsTests.cpp
  ClassicPacketsRoundTripTests.cpp
  PlayerStatePacketsTests.cpp
  SocialPacketsTests.cpp
  ChatPacketsTests.cpp
  MovementPacketsTests.cpp
  BotMovementAckTests.cpp
  WorldTransferPacketsTests.cpp
  ObjectUpdatePacketsTests.cpp
  TaxiMessagesTests.cpp
  CombatMessagesTests.cpp
  SpellCastTests.cpp
  AuraPacketsTests.cpp
  CombatLogTests.cpp
  DeathMessagesTests.cpp
  QueryMessagesTests.cpp
  ItemMessagesTests.cpp
  LootMessagesTests.cpp
  NpcMessagesTests.cpp
  QuestMessagesTests.cpp
  VendorMessagesTests.cpp
  BotWorldViewTests.cpp
  BotWorldConnectionTests.cpp
  BotWorldLoginTests.cpp
  BotLoginTests.cpp
  BotLoginLiveTests.cpp
  BotRestClientTests.cpp
  BotBnetConnectionTests.cpp
  BotBnetLoginTests.cpp
  RealBotMgrTests.cpp
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
  ${REALBOTS_DIR}/Behavior/Tasks
  ${REALBOTS_DIR}/Coroutines
  ${REALBOTS_DIR}/Net
  ${SERVER_PROTOCOL_DIR}
  ${SERVER_DIR}
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
