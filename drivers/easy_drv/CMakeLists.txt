cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

project(easy_drv VERSION 2022.09.27.0)

set(CMAKE_CXX_STANDARD 11)

set(COMMON_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common)

set(GENERAL_INCLUDES
        $<BUILD_INTERFACE:${COMMON_DIR}/win_util>
        $<BUILD_INTERFACE:${COMMON_DIR}/WTL>
        $<BUILD_INTERFACE:${COMMON_DIR}/WTL/list_control>
        $<BUILD_INTERFACE:${COMMON_DIR}/QuickLZ>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/easy_drv_vc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/zlib>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/zlib>
        $<BUILD_INTERFACE:${COMMON_DIR}/PAC-driver>
        $<BUILD_INTERFACE:${COMMON_DIR}/lua>
        $<BUILD_INTERFACE:${COMMON_DIR}/tolua++>)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/zlib EXCLUDE_FROM_ALL)

file(GLOB SRC_FILES
        *.cpp
        easy_drv_vc/*.cpp
        ${COMMON_DIR}/win_util/*.cpp
        ${COMMON_DIR}/WTL/*.cpp
        ${COMMON_DIR}/WTL/list_control/*.cpp
        ${COMMON_DIR}/QuickLZ/*.c
        ${COMMON_DIR}/PAC-driver/errors.cpp
        ${COMMON_DIR}/lua/*.c
        ${COMMON_DIR}/tolua++/*.c)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES}  ${CMAKE_CURRENT_SOURCE_DIR}/easy_drv_vc/dllmain.cpp)

target_compile_definitions(${PROJECT_NAME}
        PUBLIC DRIVER WIN_OS)
target_link_libraries(${PROJECT_NAME} zlibstatic)
target_include_directories(${PROJECT_NAME} PUBLIC ${GENERAL_INCLUDES})