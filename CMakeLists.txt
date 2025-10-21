cmake_minimum_required(VERSION 3.28.3) 

project(labwork6 LANGUAGES CXX) 
 
set(CMAKE_CXX_STANDARD 20) 
 
include(FetchContent) 

FetchContent_Declare(
    cpr 
    GIT_REPOSITORY https://github.com/libcpr/cpr.git 
    GIT_TAG 2b2b169) 

set(CPR_BUILD_SHARED_LIBS ON)
    
FetchContent_MakeAvailable(cpr) 

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_subdirectory(lib/parser_cmd_lib)
add_subdirectory(lib/api_lib)
add_subdirectory(lib/json_parser_lib)
add_subdirectory(bin)