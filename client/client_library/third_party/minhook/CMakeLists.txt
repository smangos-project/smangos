# Vendored from TrinityCore/dep/MinHook (x64 sources only). BSD-2-Clause
# licensed, see LICENSE.txt. Upstream: https://github.com/TsudaKageyu/minhook

add_library(minhook STATIC
    src/buffer.c
    src/hook.c
    src/trampoline.c
    src/hde/hde64.c
)

target_include_directories(minhook PUBLIC include)
target_include_directories(minhook PRIVATE src src/hde)
