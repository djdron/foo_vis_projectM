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

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
set(ARCH "x64/")
set(ARCH2 "-x64")
else()
set(ARCH "")
set(ARCH2 "-Win32")
endif()

find_package(libprojectM REQUIRED)
find_package(GLEW REQUIRED)
find_package(ZLIB REQUIRED)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "src/*.h")
file(GLOB RES_RC "src/*.rc")

set(FOOBAR_SDK_LIBRARIES
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/SDK/${ARCH}Release/foobar2000_SDK.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/${ARCH}Release/foobar2000_component_client.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/helpers/${ARCH}Release/foobar2000_sdk_helpers.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/shared/shared${ARCH2}.lib"
	optimized "${FOOBAR_SDK_ROOT}/pfc/${ARCH}Release/pfc.lib"
	optimized "${FOOBAR_SDK_ROOT}/libPPUI/${ARCH}Release/libPPUI.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/SDK/${ARCH}Debug/foobar2000_SDK.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/${ARCH}Debug/foobar2000_component_client.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/helpers/${ARCH}Debug/foobar2000_sdk_helpers.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/shared/shared${ARCH2}.lib"
	debug "${FOOBAR_SDK_ROOT}/pfc/${ARCH}Debug/pfc.lib"
	debug "${FOOBAR_SDK_ROOT}/libPPUI/${ARCH}Debug/libPPUI.lib"
)

add_library(foo_vis_projectM MODULE ${SRCS_CXX} ${SRCS_H} ${RES_RC})
target_link_libraries(foo_vis_projectM ${FOOBAR_SDK_LIBRARIES} libprojectM::static GLEW::GLEW ZLIB::ZLIB)
