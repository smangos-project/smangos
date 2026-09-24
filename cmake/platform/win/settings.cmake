
if(MSVC)
  include("${ROOT_DIR}/cmake/compiler/msvc/settings.cmake")
elseif(MINGW) # Allows same flags as GCC
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wall")
  if(BUILD_WARNINGS_AS_ERROR)
    set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Werror")
  endif()
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-reorder") # ... will be initialized after ...
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-unused-variable") # unused variable
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-unused-but-set-variable") # set but not used

  # TODO: Compiler Version Bug?
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-array-bounds") # array subscript is outside array bounds
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-stringop-overflow") # writing 1 or more bytes into a region of size 0 overflows the destination

  # TODO Stuff we definitely want to fix:
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-nonnull-compare") # comparing the pointer of a reference
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-format-truncation") # output truncated before the last format character

  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-cpp") # You are using an incompatible mysql version!
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-format") # format error stack / crash
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-sign-compare") # comparison of integer expressions of different signedness
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-switch") # numeration value ‘xxx’ not handled in switch
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-unused-function") # functions that we ""inlined"" in .h files but are included everywhere and not used. TODO should be move to cpp file
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-parentheses") # suggest parentheses around assignment used as truth value TODO if intended use parathesis
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-address") # retrieving the pointer of reference
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-address-of-packed-member") # taking address of packed member of may result in an unaligned pointer value
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-overloaded-virtual") # hidden overloaded virtual function
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-tautological-constant-out-of-range-compare") # Sometimes used for MAGNOS_ASSERT
  set(BUILD_ADDITIONAL_FLAGS "${BUILD_ADDITIONAL_FLAGS} -Wno-strict-aliasing") # dereferencing type-punned pointer will break strict-aliasing rules


  set(MANGOS_C_FLAGS "${MANGOS_C_FLAGS} ${BUILD_ADDITIONAL_FLAGS}")
  set(MANGOS_CXX_FLAGS "${MANGOS_CXX_FLAGS} ${BUILD_ADDITIONAL_FLAGS}")
endif()

# Added by Giperion, in WIN32 output all compiled files in one directory
# ASan builds get their own output directories so they never overwrite a normal build's binaries,
# and the ASan runtime DLL (found by asking the compiler) is dropped next to them so freshly built
# tools like protoc can run during the build.
if(ENABLE_ASAN)
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin-asan )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib-asan )
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-resource-dir OUTPUT_VARIABLE _clang_resource_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
    file(GLOB _asan_dlls "${_clang_resource_dir}/lib/windows/clang_rt.asan_dynamic-*.dll")
    foreach(_config ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
      file(COPY ${_asan_dlls} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_config})
    endforeach()
  endif()
else()
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib )
endif()
# Allow creating folders in solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
