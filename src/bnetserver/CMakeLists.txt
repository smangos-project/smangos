set(LIBRARY_NAME bnetserver)
set(LIBRARY_SRCS
  BnetServer.h
  BnetServer.cpp
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

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)
mangos_target_flags(${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME}
  shared
  framework
  bnetproto
)

if(WIN32)
  target_link_libraries(${LIBRARY_NAME}
    optimized ${MYSQL_LIBRARY}
    optimized ${OPENSSL_LIBRARIES}
    debug ${MYSQL_DEBUG_LIBRARY}
    debug ${OPENSSL_DEBUG_LIBRARIES}
    $<$<CONFIG:Debug>:${OPENSSL_SSL_DEBUG_LIBRARIES}>
    $<$<NOT:$<CONFIG:Debug>>:${OPENSSL_SSL_LIBRARIES}>
  )
  if(MINGW)
    target_link_libraries(${LIBRARY_NAME}
      -lws2_32
    )
  endif()
endif()

if(UNIX)
  target_link_libraries(${LIBRARY_NAME}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${OPENSSL_EXTRA_LIBRARIES}
  )
endif()

set_property(GLOBAL APPEND PROPERTY SERVER_SETUP_CONFIGS "${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.conf.dist.in|bnetserver.conf")
