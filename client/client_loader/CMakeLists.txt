# custom_launcher.exe: starts WowClassic.exe with custom.dll loaded before
# the client's own startup code (see src/main.cpp).

add_executable(client_loader src/main.cpp)
set_target_properties(client_loader PROPERTIES OUTPUT_NAME "custom_launcher")
target_link_libraries(client_loader PRIVATE client_common)
