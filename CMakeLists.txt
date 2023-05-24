cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

project(easy_drv VERSION 2022.09.27.0)

set(CMAKE_CXX_STANDARD 11)

set(COMMON_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/common)

set(DRIVER_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/easy_drv)

set(ZLIB_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib)

set(LUA_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/lua)
set(LUA_SRC_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/lua/src)

set(TO_LUA_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/toluapp)

set(GENERAL_INCLUDES
        $<BUILD_INTERFACE:${COMMON_DIR}/win_util>
        $<BUILD_INTERFACE:${COMMON_DIR}/WTL>
        $<BUILD_INTERFACE:${COMMON_DIR}/WTL/list_control>
        $<BUILD_INTERFACE:${COMMON_DIR}/QuickLZ>
        $<BUILD_INTERFACE:${DRIVER_DIR}>
        $<BUILD_INTERFACE:${DRIVER_DIR}/easy_drv_vc>
        $<BUILD_INTERFACE:${ZLIB_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/zlib>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/lua>
        $<BUILD_INTERFACE:${COMMON_DIR}/PAC-driver>
        $<BUILD_INTERFACE:${LUA_SRC_DIR}>
        $<BUILD_INTERFACE:${TO_LUA_DIR}/include>)

add_subdirectory(${ZLIB_DIR} EXCLUDE_FROM_ALL)
add_subdirectory(${LUA_DIR} EXCLUDE_FROM_ALL)
add_subdirectory(${TO_LUA_DIR} EXCLUDE_FROM_ALL)

file(GLOB SRC_FILES
        ${DRIVER_DIR}/*.cpp
        ${DRIVER_DIR}/easy_drv_vc/*.cpp
        ${COMMON_DIR}/win_util/*.cpp
        ${COMMON_DIR}/WTL/*.cpp
        ${COMMON_DIR}/WTL/list_control/*.cpp
        ${COMMON_DIR}/QuickLZ/*.c
        ${COMMON_DIR}/PAC-driver/errors.cpp)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES}  ${DRIVER_DIR}/easy_drv_vc/dllmain.cpp)

target_compile_definitions(${PROJECT_NAME}
        PUBLIC DRIVER WIN_OS)
target_link_libraries(${PROJECT_NAME} PRIVATE liblua_static toluapp_lib_static zlibstatic)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${GENERAL_INCLUDES})