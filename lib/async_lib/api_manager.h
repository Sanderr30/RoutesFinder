#pragma once

#include <functional>
#include <iostream>
#include <string>

#include "task_scheduler.h"
#include "../parser_cmd_lib/parser.h"
#include <nlohmann/json.hpp>


class AsyncApiManager {
public:
    using ApiResponseCallback = std::function<void(bool success
                                , const nlohmann::json& data)>;

    AsyncApiManager(TaskScheduler& scheduler
                , const std::string& api_key);

    void GetRoutesAsync(const ConfigVariables& config
                    , ApiResponseCallback callback);

    std::string BuildApiUrl(const ConfigVariables& config) const;

private:
    static constexpr const char* kDefaultApiHttp =
        "http://api.rasp.yandex.net/v3.0/search/?apikey=";

    static constexpr const char* kResponseFormat = "&format=json&";
    static constexpr const char* kFromCityFlag = "from=";
    static constexpr const char* kToCityFlag = "&to=";
    static constexpr const char* kLanguageFlag = "&lang=ru_RU";
    static constexpr const char* kDateFlag = "&date=";
    static constexpr const char* kTransferFlag = "&transfers=true";
    static constexpr const char* kLimitFlag = "&limit=10000";

private:
    TaskScheduler& scheduler_;
    std::string api_key_;
};
