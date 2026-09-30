# Generic hooking pieces: MinHook wrapper, memory patch helpers, logging, process image lookup.

add_library(hook_lib STATIC
    src/Log.cpp
    src/ProcessImage.cpp
    src/Memory.cpp
    src/HookManager.cpp
    src/Pattern.cpp
    src/ImageRemap.cpp
)

target_include_directories(hook_lib PUBLIC include)
target_link_libraries(hook_lib PUBLIC minhook)
