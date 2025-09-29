add_library(async_lib STATIC
    task_scheduler.cpp
    input_handler.cpp
    api_manager.cpp
    routes_handler.cpp
    app.cpp
    city_mapper.cpp
)

target_link_libraries(async_lib
    PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
    Boost::system
    Threads::Threads
)

target_include_directories(async_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
