cmake_minimum_required (VERSION 3.12)
project ("mlnserver")

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED YES)
set(CXX_EXTENSIONS NO)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

INCLUDE (CheckIncludeFiles)
include(ExternalProject)
include(fmt)
include(spdlog)

include_directories(${FMT_INCLUDE_DIR} ${SPDLOG_INCLUDE_DIR} 
    ${CMAKE_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/include/"
    "${CMAKE_SOURCE_DIR}/mlnserver/"
)

if(WIN32)
    message("Platfrom Windows")
elseif(UNIX)
    message("Platform Unix")
endif()

add_subdirectory("net")
add_subdirectory ("mlnserver")

add_dependencies(mln fmt spdlog)