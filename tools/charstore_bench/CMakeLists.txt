add_executable(charstore_bench Main.cpp Synthetic.cpp)
target_include_directories(charstore_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(charstore_bench PRIVATE shared framework ${MYSQL_LIBRARY} ${OPENSSL_LIBRARIES} ${OPENSSL_EXTRA_LIBRARIES})
doltlite_copy_runtime(charstore_bench)
