add_executable(doltlite_smoketest smoketest.c)
target_link_libraries(doltlite_smoketest PRIVATE doltlite)
set_target_properties(doltlite_smoketest PROPERTIES FOLDER "3rd Party")

doltlite_copy_runtime(doltlite_smoketest)
