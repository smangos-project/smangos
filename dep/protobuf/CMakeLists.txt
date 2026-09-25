# Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
# Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Vendored Google Protocol Buffers 2.6.1 (proto2 syntax), following vibemangos's own vendoring
# of it (dep/protobuf) rather than requiring a manually-installed system copy -- see
# bot-phase-1.md's shared-pieces list ("dep/protobuf (2.6 runtime + protoc) ... built as
# bnetproto"). Only the pieces the Battle.net RPC connection needs are built: libprotobuf
# (linked into anything speaking the RPC protocol) and protoc + libprotoc (a build-time-only
# code generator turning src/shared/Bnet/proto/*.proto into .pb.cc/.pb.h, see
# BotBnetConnection). Source list mirrors upstream src/Makefile.am (libprotobuf_lite_la_SOURCES
# + libprotobuf_la_SOURCES / libprotoc_la_SOURCES); java/python generators are included because
# command_line_interface.cc references them unconditionally.

set(PROTOBUF_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/protobuf-2.6.1)
set(PROTOBUF_SRC ${PROTOBUF_ROOT}/src)
# Exposed for consumers that need to pass protoc `-I` so imports of the well-known types
# (google/protobuf/descriptor.proto) resolve.
set(PROTOBUF_SRC_DIR ${PROTOBUF_SRC} CACHE INTERNAL "")

# Equivalent of what protobuf-2.6.1's `configure` generates into config.h on Linux/gcc;
# only common.cc includes it (for the HAVE_ZLIB / hash-map-type checks), so this is written
# by hand rather than run through autoconf.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.h [[
#define HASH_MAP_CLASS unordered_map
#define HASH_MAP_H <unordered_map>
#define HASH_NAMESPACE std
#define HASH_SET_CLASS unordered_set
#define HASH_SET_H <unordered_set>
#define HAVE_HASH_MAP 1
#define HAVE_HASH_SET 1
#define HAVE_PTHREAD 1
#define HAVE_ZLIB 1
#define PACKAGE_VERSION "2.6.1"
#define VERSION "2.6.1"
]])

set(LIBPROTOBUF_LITE_SRCS
  google/protobuf/stubs/atomicops_internals_x86_gcc.cc
  google/protobuf/stubs/atomicops_internals_x86_msvc.cc
  google/protobuf/stubs/common.cc
  google/protobuf/stubs/once.cc
  google/protobuf/stubs/stringprintf.cc
  google/protobuf/extension_set.cc
  google/protobuf/generated_message_util.cc
  google/protobuf/message_lite.cc
  google/protobuf/repeated_field.cc
  google/protobuf/wire_format_lite.cc
  google/protobuf/io/coded_stream.cc
  google/protobuf/io/zero_copy_stream.cc
  google/protobuf/io/zero_copy_stream_impl_lite.cc
)

set(LIBPROTOBUF_SRCS
  ${LIBPROTOBUF_LITE_SRCS}
  google/protobuf/stubs/strutil.cc
  google/protobuf/stubs/substitute.cc
  google/protobuf/stubs/structurally_valid.cc
  google/protobuf/descriptor.cc
  google/protobuf/descriptor.pb.cc
  google/protobuf/descriptor_database.cc
  google/protobuf/dynamic_message.cc
  google/protobuf/extension_set_heavy.cc
  google/protobuf/generated_message_reflection.cc
  google/protobuf/message.cc
  google/protobuf/reflection_ops.cc
  google/protobuf/service.cc
  google/protobuf/text_format.cc
  google/protobuf/unknown_field_set.cc
  google/protobuf/wire_format.cc
  google/protobuf/io/gzip_stream.cc
  google/protobuf/io/printer.cc
  google/protobuf/io/strtod.cc
  google/protobuf/io/tokenizer.cc
  google/protobuf/io/zero_copy_stream_impl.cc
  google/protobuf/compiler/importer.cc
  google/protobuf/compiler/parser.cc
)
list(TRANSFORM LIBPROTOBUF_SRCS PREPEND ${PROTOBUF_SRC}/)

add_library(protobuf STATIC ${LIBPROTOBUF_SRCS})
target_include_directories(protobuf PUBLIC ${PROTOBUF_SRC})
target_include_directories(protobuf PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(protobuf PRIVATE -w)
set_target_properties(protobuf PROPERTIES FOLDER "3rd Party")

# The `zlib` target (dep/src/zlib) only adds its own headers via directory-scoped
# include_directories(), not a target-scoped PUBLIC include, so consumers add them explicitly
# the same way src/shared and src/game already do.
if(WIN32)
  target_include_directories(protobuf PUBLIC ${ZLIB_INCLUDE_DIRS})
  target_link_libraries(protobuf PUBLIC zlib)
else()
  target_include_directories(protobuf PUBLIC ${ZLIB_INCLUDE_DIRS})
  target_link_libraries(protobuf PUBLIC ${ZLIB_LIBRARIES})
endif()

if(UNIX)
  find_package(Threads)
  target_link_libraries(protobuf PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()

set(LIBPROTOC_SRCS
  google/protobuf/compiler/code_generator.cc
  google/protobuf/compiler/command_line_interface.cc
  google/protobuf/compiler/plugin.cc
  google/protobuf/compiler/plugin.pb.cc
  google/protobuf/compiler/subprocess.cc
  google/protobuf/compiler/zip_writer.cc
  google/protobuf/compiler/cpp/cpp_enum.cc
  google/protobuf/compiler/cpp/cpp_enum_field.cc
  google/protobuf/compiler/cpp/cpp_extension.cc
  google/protobuf/compiler/cpp/cpp_field.cc
  google/protobuf/compiler/cpp/cpp_file.cc
  google/protobuf/compiler/cpp/cpp_generator.cc
  google/protobuf/compiler/cpp/cpp_helpers.cc
  google/protobuf/compiler/cpp/cpp_message.cc
  google/protobuf/compiler/cpp/cpp_message_field.cc
  google/protobuf/compiler/cpp/cpp_primitive_field.cc
  google/protobuf/compiler/cpp/cpp_service.cc
  google/protobuf/compiler/cpp/cpp_string_field.cc
  google/protobuf/compiler/java/java_context.cc
  google/protobuf/compiler/java/java_enum.cc
  google/protobuf/compiler/java/java_enum_field.cc
  google/protobuf/compiler/java/java_extension.cc
  google/protobuf/compiler/java/java_field.cc
  google/protobuf/compiler/java/java_file.cc
  google/protobuf/compiler/java/java_generator.cc
  google/protobuf/compiler/java/java_generator_factory.cc
  google/protobuf/compiler/java/java_helpers.cc
  google/protobuf/compiler/java/java_lazy_message_field.cc
  google/protobuf/compiler/java/java_message.cc
  google/protobuf/compiler/java/java_message_field.cc
  google/protobuf/compiler/java/java_name_resolver.cc
  google/protobuf/compiler/java/java_primitive_field.cc
  google/protobuf/compiler/java/java_shared_code_generator.cc
  google/protobuf/compiler/java/java_service.cc
  google/protobuf/compiler/java/java_string_field.cc
  google/protobuf/compiler/java/java_doc_comment.cc
  google/protobuf/compiler/python/python_generator.cc
)
list(TRANSFORM LIBPROTOC_SRCS PREPEND ${PROTOBUF_SRC}/)

add_library(protoc_lib STATIC ${LIBPROTOC_SRCS})
target_include_directories(protoc_lib PUBLIC ${PROTOBUF_SRC})
target_link_libraries(protoc_lib PUBLIC protobuf)
target_compile_options(protoc_lib PRIVATE -w)
set_target_properties(protoc_lib PROPERTIES FOLDER "3rd Party")

add_executable(protoc ${PROTOBUF_SRC}/google/protobuf/compiler/main.cc)
target_link_libraries(protoc PRIVATE protoc_lib protobuf)
target_compile_options(protoc PRIVATE -w)
set_target_properties(protoc PROPERTIES FOLDER "3rd Party")
