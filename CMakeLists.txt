cmake_minimum_required(VERSION 3.11)
project(GemeiEmu)

set(UNICORN_ARCH "arm")
add_subdirectory(dependencies/unicorn EXCLUDE_FROM_ALL)

add_library(GemeiEmu SHARED src/main.c "src/ccdl.c")

target_include_directories(GemeiEmu PRIVATE include)

get_target_property(UNICORN_INCLUDES unicorn INCLUDE_DIRECTORIES)
foreach(dir ${UNICORN_INCLUDES})
    target_include_directories(GemeiEmu PRIVATE ${dir})
endforeach()

target_link_libraries(GemeiEmu PRIVATE unicorn)

# TODO: copy DLL
