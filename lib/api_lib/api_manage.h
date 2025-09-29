#pragma once

#include <fstream>
#include <string>
#include <iostream>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <lib/parser_cmd_lib/parser.h>


class ApiMng {
public:
    ApiMng(const std::string& api_key_value, const ConfigVariables& cfg);
    nlohmann::json GetJson() const;

private:
    std::string api_key_;
    std::string api_http_;

    static constexpr const char* kDefaultApiKeyFilePath =
        "../../lib/app_files/api_key/api_key_holder.txt";

    static constexpr const char* kDefaultApiHttp =
        "https://api.rasp.yandex.net/v3.0/search/?apikey=";

    static constexpr const char* kResponceFormat = "&format=json&";
    static constexpr const char* kFromCityFlag = "from=";
    static constexpr const char* kToCityFlag = "&to=";
    static constexpr const char* kLanguageFlag = "&lang=ru_RU";
    static constexpr const char* kDateFlag = "&date=";
    static constexpr const char* kTransferFlag = "&transfers=true";
    static constexpr const char* kLimitFlag = "&limit=10000";

private:
    std::string Encode() const;
    std::string Decode(const std::string& encoded_api_key) const;
    void SaveKey() const;
};
