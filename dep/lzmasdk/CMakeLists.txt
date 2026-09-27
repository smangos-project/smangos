# LZMA SDK: public domain 7z archive decoder (Igor Pavlov), used by
# tools/database_tool to unpack the pinned world base dump
# (.local/plan/wms/database_tool/world-sql.md). Only the files reachable from
# the SDK's own C/Util/7z/7zMain.c ("7zDec") reference decoder are built --
# nothing that encodes.
#
# The LZMA SDK's own 7-zip.org downloads are only offered as .7z archives,
# which can't bootstrap themselves, so this pins a tagged release of the
# ip7z/7zip GitHub mirror (the project's post-2024 home for its own source)
# instead. Bumping the pin means updating the tag in the URL and its
# URL_HASH.

include(FetchContent)

FetchContent_Declare(lzmasdk_src
  URL                        https://github.com/ip7z/7zip/archive/refs/tags/24.09.tar.gz
  URL_HASH                   SHA256=e4757b307925227724a2044651193664ba3d04e9ac13b8e631f1667896014bbf
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(lzmasdk_src)

set(LZMASDK_C_DIR "${lzmasdk_src_SOURCE_DIR}/C")

add_library(lzmasdk STATIC
  ${LZMASDK_C_DIR}/7zAlloc.c
  ${LZMASDK_C_DIR}/7zArcIn.c
  ${LZMASDK_C_DIR}/7zBuf.c
  ${LZMASDK_C_DIR}/7zBuf2.c
  ${LZMASDK_C_DIR}/7zCrc.c
  ${LZMASDK_C_DIR}/7zCrcOpt.c
  ${LZMASDK_C_DIR}/7zDec.c
  ${LZMASDK_C_DIR}/7zFile.c
  ${LZMASDK_C_DIR}/7zStream.c
  ${LZMASDK_C_DIR}/Bcj2.c
  ${LZMASDK_C_DIR}/Bra.c
  ${LZMASDK_C_DIR}/Bra86.c
  ${LZMASDK_C_DIR}/BraIA64.c
  ${LZMASDK_C_DIR}/CpuArch.c
  ${LZMASDK_C_DIR}/Delta.c
  ${LZMASDK_C_DIR}/Lzma2Dec.c
  ${LZMASDK_C_DIR}/LzmaDec.c
  ${LZMASDK_C_DIR}/Ppmd7.c
  ${LZMASDK_C_DIR}/Ppmd7Dec.c
  Extract7z.c
)
target_include_directories(lzmasdk
  PRIVATE ${LZMASDK_C_DIR}
  PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(lzmasdk PROPERTIES LINKER_LANGUAGE C)
if (MSVC)
  # The SDK's own C files use the plain (non-_s) CRT string functions.
  target_compile_definitions(lzmasdk PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
