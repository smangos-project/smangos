set(EXECUTABLE_NAME bnetserver)
set(EXECUTABLE_SRCS
  BnetAccountStore.h
  BnetAccountStore.cpp
  BnetListener.h
  BnetListener.cpp
  BnetLoginPortal.h
  BnetLoginPortal.cpp
  BnetLoginTickets.h
  BnetLoginTickets.cpp
  BnetRestSession.h
  BnetRestSession.cpp
  BnetRpcSession.h
  BnetRpcSession.cpp
  BnetServices.h
  BnetServices.cpp
  BnetSslContext.h
  BnetSslContext.cpp
  Main.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
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

add_executable(${EXECUTABLE_NAME}
  ${EXECUTABLE_SRCS}
)
mangos_target_flags(${EXECUTABLE_NAME})

target_link_libraries(${EXECUTABLE_NAME}
  shared
  framework
  bnetproto
)

if(WIN32)
  target_link_libraries(${EXECUTABLE_NAME}
    optimized ${MYSQL_LIBRARY}
    optimized ${OPENSSL_LIBRARIES}
    debug ${MYSQL_DEBUG_LIBRARY}
    debug ${OPENSSL_DEBUG_LIBRARIES}
    $<$<CONFIG:Debug>:${OPENSSL_SSL_DEBUG_LIBRARIES}>
    $<$<NOT:$<CONFIG:Debug>>:${OPENSSL_SSL_LIBRARIES}>
  )
  if(MINGW)
    target_link_libraries(${EXECUTABLE_NAME}
      -lws2_32
    )
  endif()
endif()

if(UNIX)
  target_link_libraries(${EXECUTABLE_NAME}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${OPENSSL_EXTRA_LIBRARIES}
  )
endif()

set(EXECUTABLE_LINK_FLAGS "")

if(UNIX)
  set(EXECUTABLE_LINK_FLAGS "-pthread ${EXECUTABLE_LINK_FLAGS}")
endif()

if(APPLE)
  set(EXECUTABLE_LINK_FLAGS "-framework Carbon ${EXECUTABLE_LINK_FLAGS}")
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS
  "${EXECUTABLE_LINK_FLAGS}"
)

mangos_copy_openssl_dlls(${EXECUTABLE_NAME})

install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${BIN_DIR})
set_property(GLOBAL APPEND PROPERTY SERVER_SETUP_CONFIGS "${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.conf.dist.in|bnetserver.conf")
install(FILES bnetserver.conf.dist.in DESTINATION ${CONF_DIR} RENAME bnetserver.conf.dist)

if(WIN32 AND MSVC)
  install(FILES $<TARGET_PDB_FILE:${EXECUTABLE_NAME}> DESTINATION ${BIN_DIR} OPTIONAL)
endif()
