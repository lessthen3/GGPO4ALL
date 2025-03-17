####################################### Enforce CMake Version

cmake_minimum_required(VERSION 3.20)

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

####################################### Check For No Config Set

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "No build type specified, ending build. CMAKE WILL NOW EXIT")
endif()

####################################### Enforce 64-bit Requirement

if(NOT ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    message(FATAL_ERROR "GGPO4ALL only supports 64-bit platforms.")
endif()

####################################### Detect Current OS

if(WIN32)

    set(PLATFORM_NAME "win")

elseif(APPLE)

	set(PLATFORM_NAME "osx")

	set_property( #Clang++/g++ compiler settings
		TARGET ${target} 
		APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wpedantic
	)

elseif(UNIX)

	set(PLATFORM_NAME "linux")

	set_property( #Clang++/g++ compiler settings
		TARGET ${target} 
		APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wpedantic
	)

else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

####################################### Set Build Output Directories

set(
	OUTPUT_DIR 
	"${CMAKE_SOURCE_DIR}/build/${PLATFORM_NAME}/${CMAKE_BUILD_TYPE}"
)

set(
	CMAKE_ARCHIVE_OUTPUT_DIRECTORY #static lib
	${OUTPUT_DIR}
)

set(
	CMAKE_LIBRARY_OUTPUT_DIRECTORY #dynamic linkable
	${OUTPUT_DIR}
)

####################################### Find All Source Files

file(
	GLOB_RECURSE 
	GGPO4ALL_SOURCES 
	CONFIGURE_DEPENDS
	"src/*.cpp" 
	"include/*.h"
)

####################################### Library Target

# STATIC LIBRARY

add_library(
	${PROJECT_NAME} STATIC 
	${GGPO4ALL_SOURCES}
)

target_include_directories(
	${PROJECT_NAME} PUBLIC 
	"include"
)

# SHARED LIBRARY idk this isnt working and i dont feel like getting it to work xd

# add_library(
# 	${PROJECT_NAME}_shared SHARED 
# 	${GGPO4ALL_SOURCES}
# )

# target_include_directories(
# 	${PROJECT_NAME}_shared PUBLIC 
# 	"include"
# )

# if(WIN32)
#     target_compile_definitions(${PROJECT_NAME}_shared PRIVATE GGPO4ALL_DLL_EXPORT)
# endif()

####################################### Build Message

message(STATUS "Building GGPO4ALL for ${PLATFORM_NAME} in ${CMAKE_BUILD_TYPE} mode.")
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