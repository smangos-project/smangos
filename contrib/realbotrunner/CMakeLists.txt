if (NOT ENABLE_REALBOTS)
  message(FATAL_ERROR "BUILD_REALBOT_RUNNER requires ENABLE_REALBOTS to be ON")
endif()

set(REALBOTS_DIR ${CMAKE_SOURCE_DIR}/src/game/RealBots)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/src/game/Server)
set(SERVER_PROTOCOL_DIR ${SERVER_DIR}/Protocol)

# Deliberately not linking `game` (see Main.cpp's header comment): compiled directly from the
# same RealBots sources tests-realbots already builds, so the runner needs only
# shared/framework/bnetproto/zlib/openssl, never the world/scripts/database `game` pulls in.
add_executable(realbotrunner
  Main.cpp
  ${REALBOTS_DIR}/RealBotConfig.cpp
  ${REALBOTS_DIR}/RealBotMgr.cpp
  ${REALBOTS_DIR}/RealBotTarget.cpp
  ${REALBOTS_DIR}/Core/RealBot.cpp
  ${REALBOTS_DIR}/Core/BotAccountLease.cpp
  ${REALBOTS_DIR}/Core/BotStage.cpp
  ${REALBOTS_DIR}/Core/BotPacketLog.cpp
  ${REALBOTS_DIR}/RealBotCommands.cpp
  ${REALBOTS_DIR}/Behavior/BotBrain.cpp
  ${REALBOTS_DIR}/Behavior/BotContext.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskArgs.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskRegistry.cpp
  ${REALBOTS_DIR}/Behavior/Tasks/BuiltinBotTasks.cpp
  ${REALBOTS_DIR}/Coroutines/BotRoutine.cpp
  ${REALBOTS_DIR}/Coroutines/BotAwaitables.cpp
  ${SERVER_PROTOCOL_DIR}/ClassicOpcodes.cpp
  ${REALBOTS_DIR}/Net/BotConnector.cpp
  ${REALBOTS_DIR}/Net/BotWorldConnection.cpp
  ${REALBOTS_DIR}/Net/BotRestClient.cpp
  ${REALBOTS_DIR}/Net/BotBnetConnection.cpp
  ${REALBOTS_DIR}/Net/BotBnetLogin.cpp
  ${REALBOTS_DIR}/Net/BotWorldLogin.cpp
  ${REALBOTS_DIR}/Net/BotLogin.cpp
)
mangos_target_flags(realbotrunner)
SET_TARGET_PROPERTIES(realbotrunner PROPERTIES FOLDER Tools)

target_include_directories(realbotrunner PRIVATE
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

target_link_libraries(realbotrunner PRIVATE shared framework zlib bnetproto)

if (WIN32)
  target_link_libraries(realbotrunner PRIVATE
    optimized ${OPENSSL_LIBRARIES}
    debug ${OPENSSL_DEBUG_LIBRARIES}
    $<$<CONFIG:Debug>:${OPENSSL_SSL_DEBUG_LIBRARIES}>
    $<$<NOT:$<CONFIG:Debug>>:${OPENSSL_SSL_LIBRARIES}>
  )
endif()

if (UNIX)
  target_link_libraries(realbotrunner PRIVATE
    ${OPENSSL_LIBRARIES}
    ${OPENSSL_EXTRA_LIBRARIES}
  )
endif()

mangos_copy_openssl_dlls(realbotrunner)

install(TARGETS realbotrunner DESTINATION ${BIN_DIR})
