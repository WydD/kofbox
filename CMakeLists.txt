cmake_minimum_required(VERSION 3.7)
project(kofbox)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_RELEASE "/MT")
set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
INCLUDE_DIRECTORIES(deps)
LINK_DIRECTORIES(deps)
set(SOURCE_FILES
        injector/kofbox-injector.cpp
        injector/stdafx.h
        injector/targetver.h)
set(INJECTION_DLL_FILES
        hook/dllmain.cpp
        hook/kofbox.cpp
        hook/stdafx.h
        hook/targetver.h
        hook/hook_util.cpp
        hook/hook_util.h
        hook/kofbox.h)

add_executable(kofbox-inject ${SOURCE_FILES})
add_library(kofbox SHARED ${INJECTION_DLL_FILES})
add_custom_command(
        TARGET kofbox-inject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/deps/EasyHook64.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
TARGET_LINK_LIBRARIES(kofbox-inject EasyHook64)
TARGET_LINK_LIBRARIES(kofbox EasyHook64 psapi)