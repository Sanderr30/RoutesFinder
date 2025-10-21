add_library(ApiLib STATIC api_manage.cpp api_manage.h)

target_link_libraries(ApiLib PUBLIC cpr::cpr)
target_link_libraries(ApiLib PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(ApiLib PUBLIC ParseCmdLib)