add_library(JsonParserLib STATIC
    city_mapper.cpp city_mapper.h
    json_parser.cpp json_parser.h
)

target_link_libraries(JsonParserLib
    PUBLIC cpr::cpr
    PUBLIC nlohmann_json::nlohmann_json
    PUBLIC ParseCmdLib
    PUBLIC ApiLib
)
