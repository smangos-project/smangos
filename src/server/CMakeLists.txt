set(EXECUTABLE_NAME server)
set(EXECUTABLE_SRCS
  ArgparserForServer.h
  ArgparserForServer.cpp
  Main.cpp
)

if(WIN32)
  list(APPEND EXECUTABLE_SRCS
    server.rc
  )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/mangosd
  ${CMAKE_SOURCE_DIR}/src/bnetserver
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/shared
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)

if(WIN32)
  include_directories(
    ${CMAKE_SOURCE_DIR}/dep/windows/include
  )
endif()

set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")

add_executable(${EXECUTABLE_NAME}
  ${EXECUTABLE_SRCS}
)
mangos_target_flags(${EXECUTABLE_NAME})

target_link_libraries(${EXECUTABLE_NAME}
  mangosd
  bnetserver
)

doltlite_copy_runtime(${EXECUTABLE_NAME})

set(EXECUTABLE_LINK_FLAGS "")

if(UNIX)
  set(EXECUTABLE_LINK_FLAGS "-pthread ${EXECUTABLE_LINK_FLAGS} -rdynamic")
endif()

if(APPLE)
  set(EXECUTABLE_LINK_FLAGS "-framework Carbon ${EXECUTABLE_LINK_FLAGS}")
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS
  "${EXECUTABLE_LINK_FLAGS}"
)

install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${BIN_DIR})
mangos_copy_openssl_dlls(${EXECUTABLE_NAME})

if(WIN32 AND MSVC)
  install(FILES $<TARGET_PDB_FILE:${EXECUTABLE_NAME}> DESTINATION ${BIN_DIR} OPTIONAL)
endif()
