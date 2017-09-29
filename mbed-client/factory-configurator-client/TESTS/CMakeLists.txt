cmake_minimum_required(VERSION 2.6)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common_includes.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../devenv/build-common/common_tests_defines.cmake)


if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-DMBED_TRACE_MAX_LEVEL=TRACE_LEVEL_INFO)
else()
	add_definitions(-DMBED_TRACE_MAX_LEVEL=TRACE_LEVEL_ERROR)
endif()

# PAL's example code reference
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../mbed-client-pal/Examples/PlatformBSP/Include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../mbed-client-pal/Test/Unitest/Includes)

# tests includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/runner)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../unity)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../out/gen/testdata)

FILE(
	GLOB_RECURSE
	fcc_component_tests

	"${CMAKE_CURRENT_SOURCE_DIR}/runner/fcc-component-runner/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/runner/fcc-component-runner/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/runner/fcc-component-runner/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
	
	"${CMAKE_CURRENT_SOURCE_DIR}/unity/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/unity/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/../out/gen/testdata/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/../out/gen/testdata/*.h"

	${CMAKE_CURRENT_SOURCE_DIR}/../mbed-client-pal/Test/Unitest/TestRunner/pal_test${OS_BRAND}.c
)

add_definitions(
	-DPV_REPRODUCIBLE_RANDOM_FUNCTION
	-DPV_TEST_ONLY_APIS
)

message ("*********************************************************************")
message ("fcc_component_tests = [[${fcc_component_tests}]]")
message ("*********************************************************************")


CREATE_TEST_LIBRARY(fcc_component_tests "${fcc_component_tests}" "")
ADDSUBDIRS()
