add_executable(tests-shared
  PlaceholderTest.cpp
  CryptoTests.cpp
  ByteBufferTests.cpp
  WorldHandshakeCryptoTests.cpp
  HttpMessageTests.cpp
  BnetLoginFormsTests.cpp
  BnetRealmListJsonTests.cpp
  TlsStreamTests.cpp
  TestMain.cpp
)

if (ENABLE_REALBOTS)
  target_sources(tests-shared PRIVATE BnetRpcFramingTests.cpp)
endif()

target_include_directories(tests-shared PRIVATE
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
)

target_link_libraries(tests-shared PRIVATE shared framework Catch2)

if (ENABLE_REALBOTS)
  target_link_libraries(tests-shared PRIVATE bnetproto)
endif()

if (WIN32)
  target_link_libraries(tests-shared PRIVATE
    optimized ${MYSQL_LIBRARY}
    optimized ${OPENSSL_LIBRARIES}
    debug ${MYSQL_DEBUG_LIBRARY}
    debug ${OPENSSL_DEBUG_LIBRARIES}
    $<$<CONFIG:Debug>:${OPENSSL_SSL_DEBUG_LIBRARIES}>
    $<$<NOT:$<CONFIG:Debug>>:${OPENSSL_SSL_LIBRARIES}>
  )
endif()

if (UNIX)
  target_link_libraries(tests-shared PRIVATE
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${OPENSSL_EXTRA_LIBRARIES}
  )
endif()

set_target_properties(tests-shared PROPERTIES FOLDER "Tests")
mangos_copy_openssl_dlls(tests-shared)

catch_discover_tests(tests-shared TEST_PREFIX "shared: " PROPERTIES LABELS "unit")
