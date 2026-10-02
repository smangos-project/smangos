add_subdirectory(third_party/minhook)
add_subdirectory(hook_lib)

# custom.dll: loaded into WowClassic.exe by custom_launcher.exe before any of
# the client's own startup code runs (see ../client_loader/src/main.cpp).
add_library(client_library SHARED src/dllmain.cpp src/CascBypass.cpp src/CertBypass.cpp src/ClientPatches.cpp
            src/DumpTrigger.cpp src/NetDiagnostics.cpp src/ControlServer.cpp src/ControlCapture.cpp
            src/ControlInput.cpp)
set_target_properties(client_library PROPERTIES OUTPUT_NAME "custom")
target_link_libraries(client_library PRIVATE user32 crypt32 shell32 ws2_32 secur32 psapi gdi32 windowscodecs ole32 uuid hook_lib client_common)
target_include_directories(client_library PRIVATE ${CLIENT_DEP_INCLUDE})
if(MINGW)
    # no "lib" prefix, and no dependency on the mingw runtime DLLs next to the client
    set_target_properties(client_library PROPERTIES PREFIX "")
    target_link_options(client_library PRIVATE -static -static-libgcc -static-libstdc++)
endif()
