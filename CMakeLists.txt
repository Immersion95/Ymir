cmake_minimum_required(VERSION 3.28)
project(Ymir VERSION 0.1.4)

include(cmake/CMakeRC.cmake)
include(cmake/CopyRuntimeDLLs.cmake)

# CMake project structured adapted from Alex Reinking's SharedStaticStarter
# GitHub: https://github.com/alexreinking/SharedStaticStarter
# Blog: https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html

## Determine if this is a top-level build or included as a subproject
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

option(Ymir_INCLUDE_PACKAGING "Include packaging rules for Ymir" "${is_top_level}")
option(Ymir_DEV_BUILD "Mark as a development build" ON)
option(Ymir_ENABLE_TESTS "Enable tests for Ymir" "${is_top_level}")
option(Ymir_ENABLE_SANDBOX "Compile the sandbox app" "${is_top_level}")
option(Ymir_ENABLE_YMDASM "Compile the disassembly tool" "${is_top_level}")
option(Ymir_ENABLE_IPO "Enable IPO / LTO for Ymir" ON)
option(Ymir_ENABLE_DEVLOG "Enable development logs" ${Ymir_DEV_BUILD})
option(Ymir_ENABLE_IMGUI_DEMO "Enable ImGui demo window" ON)

if (Ymir_DEV_BUILD)
    message(STATUS "Ymir: Development build")
else ()
    message(STATUS "Ymir: Release build")
endif ()
message(STATUS "Ymir: Devlog ${Ymir_ENABLE_DEVLOG}")

# Detect target architectures
# ARCHITECTURES may have multiple variables in the case of a multi-arch build
# such as a universal binary on MacOS
include(cmake/DetectArchitecture.cmake)
if (NOT DEFINED ARCHITECTURES)
    message(FATAL_ERROR "Unsupported architecture")
endif ()
message(STATUS "Ymir: Architecture(s) ${ARCHITECTURES}")

## Performance options
if ("x86_64" IN_LIST ARCHITECTURES)
	option(Ymir_AVX2 "Enable AVX2 instruction set for Ymir" OFF)
	if (Ymir_AVX2)
		message(STATUS "Ymir: Using AVX2 instruction set")
	else ()
		message(STATUS "Ymir: Using SSE2 instruction set")
	endif ()
endif ()

if ("arm64" IN_LIST ARCHITECTURES)
	message(STATUS "Ymir: Using NEON instruction set")
endif ()


## C++ language configuration boilerplate
if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND
    NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif ()

## Let Ymir_SHARED_LIBS override BUILD_SHARED_LIBS
if (DEFINED Ymir_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${Ymir_SHARED_LIBS}")
endif ()

## Configure Visual Studio solution
if (MSVC)
    include(cmake/VSHelpers.cmake)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif ()

## Enable LTO if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

if (IPO_SUPPORTED)
    message(STATUS "IPO / LTO supported")
else ()
    message(STATUS "IPO / LTO not supported: ${IPO_ERROR}")
endif ()

## Include third-party libraries
add_subdirectory(vendor)

## Include project libraries and apps
add_subdirectory(libs)
add_subdirectory(apps)

## Include the install rules if the user wanted them (included by default when top-level)
if (Ymir_INCLUDE_PACKAGING)
    add_subdirectory(packaging)
endif ()

## Include the unit tests if the user wanted them (included by default when top-level)
if (Ymir_ENABLE_TESTS)
    add_subdirectory(tests)
endif ()

## Configure Visual Studio solution
if (MSVC)
    set_property(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY VS_STARTUP_PROJECT ymir-sdl3)
endif ()
