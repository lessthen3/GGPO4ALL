####################################### Enforce CMake Version

cmake_minimum_required(VERSION 4.2.0)

#######################################

option(BUILD_P2P_TESTS "Build P2P tests uwu" ON)
option(BUILD_SPECTATOR_TESTS "" ON)
option(BASIC_UDP_TEST "" ON)
option(BASIC_TCP_TEST "" ON)

option(BUILD_GGPO_C_API_SHARED "Build C API as dynamic or static lib >w<" OFF)

####################################### Project definition

project(
	GGPO4ALL 
	VERSION 0.0.1 
	DESCRIPTION "Cross Platform GGPO" 
	LANGUAGES CXX
)

####################################### Enforcing C++20 standard

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

####################################### Check For No Config Set

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "No build type specified, ending build. CMAKE WILL NOW EXIT")
endif()

####################################### Detect Current Target OS

set(PLATFORM_NAME "Unknown")
set(PLATFORM_FAMILY "Unknown")
set(PLATFORM_MOBILE FALSE)
set(PLATFORM_DESKTOP FALSE)

if(ANDROID)
    set(PLATFORM_NAME "Android")
    set(PLATFORM_FAMILY "Mobile")
    set(PLATFORM_MOBILE TRUE)

elseif(IOS)
    set(PLATFORM_NAME "iOS")
    set(PLATFORM_FAMILY "Mobile")
    set(PLATFORM_MOBILE TRUE)

elseif(APPLE)
    set(PLATFORM_NAME "iOS")
    set(PLATFORM_FAMILY "Mobile")
    set(PLATFORM_MOBILE TRUE)
	
elseif(WIN32)
    set(PLATFORM_NAME "Windows")
    set(PLATFORM_FAMILY "Desktop")
    set(PLATFORM_DESKTOP TRUE)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_NAME "Linux")
    set(PLATFORM_FAMILY "Desktop")
    set(PLATFORM_DESKTOP TRUE)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Haiku")
    set(PLATFORM_NAME "Haiku")
    set(PLATFORM_FAMILY "Desktop")
    set(PLATFORM_DESKTOP TRUE)

elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(PLATFORM_NAME "FreeBSD")
    set(PLATFORM_FAMILY "Desktop")
    set(PLATFORM_DESKTOP TRUE)

# Optional: collapse all BSDs together
# elseif(BSD)
#     set(PLATFORM_NAME "${BSD}")   # FreeBSD / OpenBSD / NetBSD / DragonFlyBSD
#     set(PLATFORM_FAMILY "BSD")
#     set(PLATFORM_DESKTOP TRUE)

else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

####################################### Detect Current Target Architecture

set(PLATFORM_ARCH "unknown")

# Visual Studio target arch is better than relying only on CMAKE_SYSTEM_PROCESSOR
if(CMAKE_VS_PLATFORM_NAME)
    if(CMAKE_VS_PLATFORM_NAME STREQUAL "Win32")
        set(PLATFORM_ARCH "x86")
    elseif(CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
        set(PLATFORM_ARCH "x86_64")
    elseif(CMAKE_VS_PLATFORM_NAME STREQUAL "ARM")
        set(PLATFORM_ARCH "arm")
    elseif(CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
        set(PLATFORM_ARCH "arm64")
    endif()
endif()

# Fallback for non-VS generators and general cross-platform use
if(PLATFORM_ARCH STREQUAL "unknown")
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _proc)

    if(_proc MATCHES "^(x86_64|amd64)$")
        set(PLATFORM_ARCH "x86_64")
    elseif(_proc MATCHES "^(x86|i[3-6]86)$")
        set(PLATFORM_ARCH "x86")
    elseif(_proc MATCHES "^(aarch64|arm64)$")
        set(PLATFORM_ARCH "arm64")
    elseif(_proc MATCHES "^arm")
        set(PLATFORM_ARCH "arm")
    else()
        set(PLATFORM_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

####################################### Enforce 64-bit Requirement

if(NOT PLATFORM_ARCH STREQUAL "x86" AND NOT PLATFORM_ARCH STREQUAL "arm")
    set(PLATFORM_IS_64_BIT TRUE)
else()
    set(PLATFORM_IS_64_BIT FALSE)
endif()

if(NOT PLATFORM_IS_64_BIT)
    message(FATAL_ERROR "GGPO4ALL only supports 64-bit platforms. Detected: ${PLATFORM_ARCH}")
endif()

####################################### Android-specific info

if(ANDROID)
    message(STATUS "Android ABI: ${CMAKE_ANDROID_ARCH_ABI}")
    message(STATUS "Android API: ${CMAKE_SYSTEM_VERSION}")
endif()

####################################### Output directory

set(OUTPUT_DIR
    "${CMAKE_SOURCE_DIR}/build/${PLATFORM_NAME}/${PLATFORM_ARCH}/$<CONFIG>"
)

####################################### Library Target

set(C_API GGPO4ALL_C_API)

if(BUILD_GGPO_C_API_SHARED)
    set(GGPO4ALL_C_BUILD_TYPE SHARED)
else()
    set(GGPO4ALL_C_BUILD_TYPE STATIC)
endif()

# GGPO4ALL
file(
    GLOB_RECURSE 
    GGPO4ALL_C_API_SOURCES 
    CONFIGURE_DEPENDS 
    "c_api/*.cpp"
    "c_api/*.h"
)

# STATIC LIBRARY C API
add_library(${C_API} ${GGPO4ALL_C_BUILD_TYPE}
	${GGPO4ALL_C_API_SOURCES}
)

target_include_directories( ${C_API} PRIVATE
	"header_include"
)

set_target_properties(${C_API} PROPERTIES POSITION_INDEPENDENT_CODE ON) #make peach_core PIC friendly UwU

add_executable(
	Peer2Peer_Test 
	${PROJECT_SOURCE_DIR}/tests/Peer2Peer/Peer2Peer.h
	${PROJECT_SOURCE_DIR}/tests/Peer2Peer/main.cpp
)

target_include_directories(Peer2Peer_Test PRIVATE
	"header_include"
)

add_executable(
	Spectator_Test 
	${PROJECT_SOURCE_DIR}/tests/Spectator/Spectator.h
	${PROJECT_SOURCE_DIR}/tests/Spectator/main.cpp
)

target_include_directories(Spectator_Test PRIVATE
	"header_include"
)

####################################### Compiler warnings

target_compile_options(${C_API} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>: -Wall -Wextra -Wpedantic>
    # $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:-Werror>
    $<$<CXX_COMPILER_ID:MSVC>: /W4>
    # $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/WX>
)

####################################### Build Message

message(STATUS "Building GGPO4ALL for ${PLATFORM_NAME} (${PLATFORM_ARCH}) in ${CMAKE_BUILD_TYPE} mode.")
message(STATUS "Target system name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Target processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Build artifacts will be stored in: ${OUTPUT_DIR}")

####################################### Set Startup Project (Visual Studio & Xcode)

# Set GGPO4ALL as the default scheme in Xcode

if(CMAKE_GENERATOR STREQUAL "Xcode")
    set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
    set(CMAKE_XCODE_SCHEME_EXECUTABLE ${PROJECT_NAME})
    message(STATUS "Setting ${PROJECT_NAME} as the default startup project for Xcode")

# Set GGPO4ALL as the startup project for Visual Studio

elseif(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    message(STATUS "Setting ${PROJECT_NAME} as the default startup project for VS2022")

endif()