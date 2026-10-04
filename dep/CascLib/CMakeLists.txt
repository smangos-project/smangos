add_library(casc STATIC
  src/common/Common.cpp
  src/common/Directory.cpp
  src/common/Csv.cpp
  src/common/FileStream.cpp
  src/common/FileTree.cpp
  src/common/ListFile.cpp
  src/common/Mime.cpp
  src/common/RootHandler.cpp
  src/common/Sockets.cpp
  src/hashes/md5.cpp
  src/hashes/sha1.cpp
  src/jenkins/lookup3.c
  src/overwatch/apm.cpp
  src/overwatch/cmf.cpp
  src/overwatch/aes.cpp
  src/CascDecompress.cpp
  src/CascDecrypt.cpp
  src/CascDumpData.cpp
  src/CascFiles.cpp
  src/CascFindFile.cpp
  src/CascIndexFiles.cpp
  src/CascOpenFile.cpp
  src/CascOpenStorage.cpp
  src/CascReadFile.cpp
  src/CascRootFile_Diablo3.cpp
  src/CascRootFile_Install.cpp
  src/CascRootFile_MNDX.cpp
  src/CascRootFile_Text.cpp
  src/CascRootFile_TVFS.cpp
  src/CascRootFile_OW.cpp
  src/CascRootFile_WoW.cpp)
target_include_directories(casc PUBLIC src)
target_compile_definitions(casc PUBLIC CASC_USE_SYSTEM_ZLIB CASCLIB_NO_AUTO_LINK_LIBRARY CASCLIB_NODEBUG)
target_link_libraries(casc PUBLIC ZLIB::ZLIB)
set_target_properties(casc PROPERTIES FOLDER "3rd Party")
