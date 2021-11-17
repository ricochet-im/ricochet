cmake_minimum_required (VERSION 3.15)

project (tego)

get_filename_component (srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component (outdir "${CMAKE_BINARY_DIR}" REALPATH)

# no in source builds
if ("${srcdir}" STREQUAL "${outdir}")
    message (
        FATAL_ERROR
            "In-source builds are not allowed. Please create a seperate build directory and run cmake from there.")
endif ()

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

add_subdirectory (libtego)
add_subdirectory (libtego_ui)
add_subdirectory (ricochet-refresh)
