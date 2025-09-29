#pragma once

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include "json_parser.h"
#include "../parser_cmd_lib/parser.h"


class CityMapper {
public:
    CityMapper(const std::string& api_key_value, ConfigVariables& cfg);

private:
    std::string api_key_;
    std::string api_http_;

    YandexJsonCodeWords JsonWords;

    static constexpr const char* kPathToCitiesAndCodes
        = "../../lib/app_files/cache_files/cities_codes.txt";
    static constexpr const char* kDefaultApiHttp
        = "https://api.rasp.yandex.net/v3.0/stations_list/?apikey=";
    static constexpr const char* kLanguageFlag
        = "&lang=ru_RU";
    static constexpr const char* kResponceFormat
        = "&format=json";

private:
    bool ValidKey() const;
    void LoadCache();
    void CityCodeChanger(ConfigVariables& cfg);
};
