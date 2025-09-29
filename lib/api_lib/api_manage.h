#pragma once

#include <fstream>
#include <string>
#include <iostream>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "../parser_cmd_lib/parser.h"

class ApiMng {
public:
    ApiMng(const std::string& api_key_value, const ConfigVariables& cfg);
    nlohmann::json GetJson() const;

private:
    std::string api_key_;
    std::string api_http_;
    
    const std::string kDefaultApiKeyFilePath = "../../lib/app_files/api_key/api_key_holder.txt";
    
    const std::string kDefaultApiHttp = "https://api.rasp.yandex.net/v3.0/search/?apikey=";
    const std::string kResponceFormat = "&format=json&";
    const std::string kFromCityFlag = "from=";
    const std::string kToCityFlag = "&to=";
    const std::string kLanguageFlag = "&lang=ru_RU";
    const std::string kDateFlag = "&date=";
    const std::string kTransferFlag = "&transfers=true";
    const std::string kLimitFlag = "&limit=10000";

private:
    std::string Encode() const;
    std::string Decode(const std::string& encoded_api_key) const;
    void SaveKey() const;
};