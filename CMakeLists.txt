﻿cmake_minimum_required (VERSION 2.8)

# global options
#set (CMAKE_VERBOSE_MAKEFILE ON)
set_property (GLOBAL PROPERTY USE_FOLDERS ON)

# version
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_MICRO 0)
set (VERSION_DEVEL devel)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
if (VERSION_DEVEL)
 unset (VERSION)
 set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}-${VERSION_DEVEL})
endif ()

# build type
# *NOTE*: if the user specifies -DCMAKE_BUILD_TYPE on the command line, take
#         their definition and dump it in the cache along with proper
#         documentation, otherwise set CMAKE_BUILD_TYPE to Debug prior to
#         calling PROJECT()
set (BUILD_TYPE_CACHE_STRING "choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
if (DEFINED CMAKE_BUILD_TYPE)
 set (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING ${BUILD_TYPE_CACHE_STRING})
else ()
 set (CMAKE_BUILD_TYPE Debug CACHE STRING ${BUILD_TYPE_CACHE_STRING})
endif ()

# project
cmake_policy (SET CMP0048 NEW) # allow project VERSION
project (Arkanoid
         VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
         LANGUAGES CXX C)

# directories
# override the installation directories ?
set (INSTALL_LIB_DIR     lib     CACHE PATH "Installation directory for libraries")
set (INSTALL_BIN_DIR     bin     CACHE PATH "Installation directory for executables")
set (INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
if (WIN32 AND NOT CYGWIN)
 set (DEF_INSTALL_CMAKE_DIR CMake)
else ()
 set (DEF_INSTALL_CMAKE_DIR lib/CMake/FooBar)
endif ()
set (INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# make relative paths absolute (needed later on)
foreach (path LIB BIN INCLUDE CMAKE)
 set (var INSTALL_${path}_DIR)
 if (NOT IS_ABSOLUTE "${${var}}")
  set (${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
 endif ()
endforeach ()

# project options
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options (-g)
  add_compile_options (-O0)
  add_compile_options (-fno-inline)
 endif ()
 add_definitions (-D_DEBUG)
 
 if (WIN32)
  add_definitions (-D_ITERATOR_DEBUG_LEVEL=2)
 endif (WIN32)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS _DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL Release)
 if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options (-O3)
 endif ()
 add_definitions (-DACE_NDEBUG)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACE_NDEBUG)
 if (WIN32)
  add_definitions (-D_ITERATOR_DEBUG_LEVEL=0)
 endif ()
endif ()

option (TRACING "enable tracing" OFF)
if (TRACING)
 add_definitions (-DARKANOID_NTRACE=0)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ARDRONE_NTRACE=0)
else ()
 add_definitions (-DARKANOID_NTRACE=1)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ARDRONE_NTRACE=1)
endif ()

option (VALGRIND_SUPPORT "enable valgrind support" OFF)
if (VALGRIND_SUPPORT)
 include (CheckIncludeFiles)
 check_include_files (valgrind/valgrind.h HAVE_VALGRIND_H)
#  include (CheckLibraryExists)
#  check_library_exists (libvalgrind "" "" HAVE_LIB_VALGRIND)
 include (FindPkgConfig)
 pkg_check_modules (PKGVALGRIND REQUIRED valgrind)
 add_definitions (-DVALGRIND_SUPPORT)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS LIBACESTREAM_ENABLE_VALGRIND_SUPPORT)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
 add_compile_options (-ansi)
 add_compile_options (-std=c++17)
 add_compile_options (-Wall)
 set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -g")
 add_definitions (-D_GNU_SOURCE)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS _GNU_SOURCE)
endif ()

option (BUILD_MODULES "build (git) sub-modules" OFF)

if (MSVC)
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

 set (MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:NO")
 foreach (FLAG_TYPE EXE MODULE SHARED STATIC)
  string (REGEX REPLACE "/INCREMENTAL* " "/INCREMENTAL:NO " CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG "${CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG}")
#  message (STATUS "flag: ${FLAG_TYPE}: ${CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG}")
  string (REPLACE "/EDITANDCONTINUE"     ""                 CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG "${CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG}")
  set (CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG "${CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG}"
       CACHE STRING "Overriding default debug ${FLAG_TYPE} linker flags." FORCE)
  mark_as_advanced (CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG)
 endforeach ()
endif (MSVC)

################################################################################

#list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

################################################################################

file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
if (WIN32)
 string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
endif (WIN32)
list (APPEND CMAKE_MODULE_PATH "${LIB_ROOT}/cmake-modules")
include (UseBackportedModules)
cmake_policy (SET CMP0026 OLD)
cmake_policy (SET CMP0053 OLD)
include (CreateLaunchers)

add_subdirectory (src)

################################################################################

# configuration files
# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export (PACKAGE ${PROJECT_NAME})

# Create the ArkanoidConfig.cmake and ArkanoidConfigVersion.cmake files
file (RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set (CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file (${PROJECT_NAME}Config.cmake.in "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)
# ... for the install tree
set (CONF_INCLUDE_DIRS "\${ARKANOID_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file (${PROJECT_NAME}Config.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake" @ONLY)
# ... for both
configure_file (${PROJECT_NAME}ConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

set (PACKAGE ${PROJECT_NAME})
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.h @ONLY)
add_definitions (-DHAVE_CONFIG_H)

################################################################################

if (UNIX)
 include (GNUInstallDirs)
elseif (WIN32)
 include (InstallRequiredSystemLibraries)
endif ()

# Install the ArkanoidConfig.cmake and ArkanoidConfigVersion.cmake
install (FILES
         "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
         "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
         DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
# Install the export set for use with the install-tree
install (EXPORT ${PROJECT_NAME}Targets
         DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

################################################################################

# enable (unit/integration) tests
#include (CTest)
enable_testing ()

################################################################################

# build a CPack driven installer package
set (CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${VERSION_MICRO})

set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set (CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README)

set (CPACK_PACKAGE_DESCRIPTION "this is a portable Arkanoid game")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "this is a portable Arkanoid game")
#set (CPACK_PACKAGE_VENDOR "Vendor")
set (CPACK_PACKAGE_CONTACT "erik.sohns@posteo.de")

# *NOTE*: this does not work on Win32 using the NSIS installer
set (CPACK_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION OFF)

#set (CPACK_COMPONENTS_ALL bin config data)

include (CPack)

################################################################################
