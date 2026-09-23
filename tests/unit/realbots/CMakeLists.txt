set(REALBOTS_DIR ${CMAKE_SOURCE_DIR}/src/game/RealBots)

add_executable(tests-realbots
  ${REALBOTS_DIR}/Core/RealBot.cpp
  ${REALBOTS_DIR}/Behavior/BotBrain.cpp
  ${REALBOTS_DIR}/Behavior/BotContext.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskArgs.cpp
  ${REALBOTS_DIR}/Behavior/BotTaskRegistry.cpp
  ${REALBOTS_DIR}/Coroutines/BotRoutine.cpp
  ${REALBOTS_DIR}/Coroutines/BotAwaitables.cpp
  CoroutineTests.cpp
  TestRoutineTests.cpp
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
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/shared
)

target_link_libraries(tests-realbots PRIVATE shared framework Catch2)
set_target_properties(tests-realbots PROPERTIES FOLDER "Tests")

catch_discover_tests(tests-realbots TEST_PREFIX "realbots: " PROPERTIES LABELS "unit")
