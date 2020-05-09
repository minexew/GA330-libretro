cmake_minimum_required(VERSION 3.11)
project(GemeiEmu)

set(UNICORN_ARCH "arm")
add_subdirectory(dependencies/unicorn EXCLUDE_FROM_ALL)

# GA330 core

set(SVC_HANDLERS_CPP ${CMAKE_CURRENT_BINARY_DIR}/svc_handlers.cpp)

add_library(GemeiEmu SHARED
        src/ccdl.cpp
        src/ccdl.hpp
        src/filesystem.cpp
        src/filesystem.hpp
        src/libretro-main.cpp
        src/svc_handlers.hpp
        ${SVC_HANDLERS_CPP}
        )

target_include_directories(GemeiEmu PRIVATE include)

get_target_property(UNICORN_INCLUDES unicorn INCLUDE_DIRECTORIES)
foreach(dir ${UNICORN_INCLUDES})
    target_include_directories(GemeiEmu PRIVATE ${dir})
endforeach()

target_link_libraries(GemeiEmu PRIVATE unicorn)

set_property(TARGET GemeiEmu PROPERTY CXX_STANDARD 17)

# SVC thunks & handlers

set(CSV ${CMAKE_CURRENT_LIST_DIR}/api.csv)

add_custom_command(OUTPUT ${SVC_HANDLERS_CPP}
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/make-thunks.py
        ${CSV}
        ${CMAKE_CURRENT_LIST_DIR}/minisys/thunks.c      # FIXME
        ${CMAKE_CURRENT_LIST_DIR}/minisys/thunks.h      # FIXME
        ${SVC_HANDLERS_CPP}
        DEPENDS ${CSV})

# libccos: currently not used in native code, included for documentation, CI, and IDEs

add_library(libccos STATIC
        libccos/include/libccos/ccos_host.h
        libccos/include/libccos/dl.h
        libccos/include/libccos/malloc.h
        libccos/include/libccos/ucos_ii.h
        libccos/src/dl.c
        libccos/src/malloc.c
        libccos/src/ucos_ii.c
        )

target_include_directories(libccos PUBLIC libccos/include)

# TODO: copy DLL
