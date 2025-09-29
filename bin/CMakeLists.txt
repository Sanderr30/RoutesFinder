add_executable(labwork6 async_main.cpp)

target_link_libraries(labwork6
    PRIVATE
    async_lib
    ParseCmdLib
    ApiLib
    JsonParserLib
    cpr::cpr
    nlohmann_json::nlohmann_json
    Boost::system
    Threads::Threads
)

target_include_directories(labwork6 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)
