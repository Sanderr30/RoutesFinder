#pragma once

#include "task_scheduler.h"
#include "../parser_cmd_lib/parser.h"
#include <nlohmann/json.hpp>
#include <functional>
#include <unordered_map>

class AsyncCityMapper {
public:
    using MappingCallback = std::function<void(bool success
                            , const std::string& message)>;

    AsyncCityMapper(TaskScheduler& scheduler
                    , const std::string& api_key);

    void LoadCitiesAsync(MappingCallback callback);

    void MapCityCodesAsync(ConfigVariables& config
                        , MappingCallback callback);

private:
    void LoadCitiesFromApi(MappingCallback callback);
    void LoadCitiesFromCache(MappingCallback callback);

    std::string BuildCitiesApiUrl() const;
    bool IsCacheValid() const;
    bool IsAlreadyCode(const std::string& city) const;

    TaskScheduler& scheduler_;
    std::string api_key_;

private:
    static constexpr const char* kCacheFilePath =
        "../../lib/app_files/cache_files/cities_codes.txt";
    static constexpr const char* kCitiesApiUrl =
        "http://api.rasp.yandex.net/v3.0/stations_list/?apikey=";
    static constexpr const char* kLanguageFlag =
        "&lang=ru_RU";
    static constexpr const char* kResponseFormat =
        "&format=json";
};
