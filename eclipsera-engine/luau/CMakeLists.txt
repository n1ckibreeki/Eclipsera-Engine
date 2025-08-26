# luau/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(LuauBuilder LANGUAGES C CXX)

# MSVC runtime (/MD)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# Luau build config header
set(LUAU_CFG "${CMAKE_BINARY_DIR}/luau_build_config.h")
file(WRITE "${LUAU_CFG}" "#pragma once\n#define LUAU_ENABLE_VECTOR 1\n#define LUAU_ENABLE_SANDBOX 1\n")

# Include directories
set(LUAU_INC
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Ast/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Compiler/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Config/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/VM/include"
)

# Source files
file(GLOB LUAU_AST_SRC      "Ast/src/*.cpp")
file(GLOB LUAU_COMMON_SRC   "Common/src/*.cpp")
file(GLOB LUAU_COMPILER_SRC "Compiler/src/*.cpp")
file(GLOB LUAU_CONFIG_SRC   "Config/src/*.cpp")
file(GLOB LUAU_VM_CPP_SRC   "VM/src/*.cpp")
file(GLOB LUAU_VM_C_SRC     "VM/src/*.c")

# Create the static library
add_library(Luau STATIC
  ${LUAU_AST_SRC} ${LUAU_COMMON_SRC} ${LUAU_COMPILER_SRC}
  ${LUAU_CONFIG_SRC} ${LUAU_VM_CPP_SRC} ${LUAU_VM_C_SRC}
)
target_include_directories(Luau PUBLIC ${LUAU_INC})
target_compile_features(Luau PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(Luau PRIVATE /EHsc /O2 /DNOMINMAX /FI"${LUAU_CFG}")
else()
  target_compile_definitions(Luau PRIVATE LUAU_ENABLE_VECTOR=1 LUAU_ENABLE_SANDBOX=1)
endif()


# ---vvv--- CORRECTED INSTALLATION BLOCK ---vvv---
# This is the critical fix. We install each component's 'include' directory
# separately to preserve the folder structure, just like in the source tree.
include(GNUInstallDirs)

install(TARGETS Luau
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Note the lack of a trailing slash on the source directory names.
# This copies the 'include' directory itself into the destination.
install(DIRECTORY Ast/include      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/luau/Ast)
install(DIRECTORY Common/include   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/luau/Common)
install(DIRECTORY Compiler/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/luau/Compiler)
install(DIRECTORY Config/include   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/luau/Config)
install(DIRECTORY VM/include       DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/luau/VM)
# ---^^^--- CORRECTED INSTALLATION BLOCK ---^^^---