
# First, get the root of the repository.
get_filename_component(ROOT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(ROOT_DIR "${ROOT_DIR}/.." ABSOLUTE)

# Set the directory to find dependencies
list(APPEND CMAKE_MODULE_PATH ${ROOT_DIR}/cmake/find)

# Apply MaNGOS-specific compile flags to a target.
# Keeps MaNGOS project flags (warnings, sanitizers, etc.) isolated from
# 3rd-party dependencies like cpptrace that are built in-tree.
# Usage: mangos_target_flags(<target>)
function(mangos_target_flags _target)
  if(MANGOS_C_FLAGS)
    separate_arguments(_c NATIVE_COMMAND "${MANGOS_C_FLAGS}")
    target_compile_options(${_target} PRIVATE $<$<COMPILE_LANGUAGE:C>:${_c}>)
  endif()
  if(MANGOS_CXX_FLAGS)
    separate_arguments(_cxx NATIVE_COMMAND "${MANGOS_CXX_FLAGS}")
    target_compile_options(${_target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${_cxx}>)
  endif()
endfunction()

# Copies the OpenSSL runtime DLLs next to a Windows executable target's output, so it can
# actually run (not just link) without OPENSSL_ROOT_DIR/bin on PATH.
# Usage: mangos_copy_openssl_dlls(<target>)
function(mangos_copy_openssl_dlls _target)
  if(WIN32 AND OPENSSL_RUNTIME_DLLS)
    add_custom_command(TARGET ${_target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENSSL_RUNTIME_DLLS} "$<TARGET_FILE_DIR:${_target}>"
      COMMAND_EXPAND_LISTS
    )
  endif()
endfunction()
