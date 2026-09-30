# custom_launcher.exe: starts WowClassic.exe with custom.dll loaded before
# the client's own startup code (see src/main.cpp).

add_executable(launcher src/main.cpp)
set_target_properties(launcher PROPERTIES OUTPUT_NAME "custom_launcher")
target_link_libraries(launcher PRIVATE client_common)
