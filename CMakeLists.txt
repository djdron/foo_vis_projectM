cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(foo_vis_projectM)

if("${FOOBAR_SDK_ROOT}" STREQUAL "")
message(FATAL_ERROR "Unable to find the foobar2000 SDK. Please set FOOBAR_SDK_ROOT to the root directory containing foobar2000 SDK. Get it here: http://www.foobar2000.org/SDK")
elseif(NOT EXISTS "${FOOBAR_SDK_ROOT}/foobar2000/SDK/foobar2000.h")
message(FATAL_ERROR "Unable to find the foobar2000 SDK in ${FOOBAR_SDK_ROOT}")
else()
include_directories("${FOOBAR_SDK_ROOT}")
include_directories("${FOOBAR_SDK_ROOT}/foobar2000")
endif()

if("${WTL_ROOT}" STREQUAL "")
message(FATAL_ERROR "Unable to find WTL. Please set WTL_ROOT to the root directory containing WTL. Get it here: http://wtl.sourceforge.net/")
elseif(NOT EXISTS "${WTL_ROOT}/Include/atlapp.h")
message(FATAL_ERROR "Unable to find WTL in ${WTL_ROOT}")
else()
include_directories("${WTL_ROOT}/Include")
endif()

find_package(libprojectM REQUIRED)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")
file(GLOB RES_RC "*.rc")

set(LIBS "")

set(FOOBAR_SDK_LIBRARIES
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/SDK/Release/foobar2000_SDK.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/Release/foobar2000_component_client.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/helpers/Release/foobar2000_sdk_helpers.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/shared/shared.lib"
	optimized "${FOOBAR_SDK_ROOT}/pfc/Release/pfc.lib"
	optimized "${FOOBAR_SDK_ROOT}/libPPUI/Release/libPPUI.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/SDK/Debug/foobar2000_SDK.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/Debug/foobar2000_component_client.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/helpers/Debug/foobar2000_sdk_helpers.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/shared/shared.lib"
	debug "${FOOBAR_SDK_ROOT}/pfc/Debug/pfc.lib"
	debug "${FOOBAR_SDK_ROOT}/libPPUI/Debug/libPPUI.lib"
)

add_library(foo_vis_projectM MODULE ${SRCS_CXX} ${SRCS_H} ${RES_RC})
target_link_libraries(foo_vis_projectM ${LIBS} ${FOOBAR_SDK_LIBRARIES} libprojectM::static)
