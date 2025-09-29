cmake_minimum_required(VERSION 3.28.3)

project(labwork6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR})

include(FetchContent)

set(CPR_ENABLE_SSL OFF CACHE BOOL "Enable SSL for CPR")
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 2b2b169)

set(CPR_BUILD_SHARED_LIBS ON)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

find_package(Boost NO_MODULE QUIET COMPONENTS system)

if(NOT Boost_FOUND)

    set(BOOST_ENABLE_CMAKE ON)
    set(BOOST_INCLUDE_LIBRARIES system asio)

    FetchContent_Declare(
        Boost
        URL https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.gz
        URL_HASH SHA256=be0d91732d5b0cc6fbb275c7939974457e79b54d6f07ce2e3dfdd68bef883b0b
    )

    FetchContent_MakeAvailable(Boost)

    if(TARGET Boost::system)
        message(STATUS "Using downloaded Boost::system")
    else()
        add_library(Boost::system INTERFACE IMPORTED)
        target_include_directories(Boost::system INTERFACE ${boost_SOURCE_DIR})
    endif()
else()
    message(STATUS "Using system Boost")
endif()

include_directories(${boost_SOURCE_DIR})


find_package(Threads REQUIRED)

if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_subdirectory(lib/parser_cmd_lib)
add_subdirectory(lib/api_lib)
add_subdirectory(lib/json_parser_lib)
add_subdirectory(lib/async_lib)
add_subdirectory(bin)
