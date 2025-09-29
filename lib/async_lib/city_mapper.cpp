#include "city_mapper.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>


AsyncCityMapper::AsyncCityMapper(TaskScheduler& scheduler
                            , const std::string& api_key)
    : scheduler_(scheduler)
    , api_key_(api_key)
{}


void AsyncCityMapper::LoadCitiesAsync(MappingCallback callback) {
    LoadCitiesFromCache([this, callback](bool cache_success, const std::string& cache_result)
        {
            if (cache_success) {
                callback(true, "Cities loaded from cache");
            } else {
                LoadCitiesFromApi(callback);
            }
        }
    );
}


void AsyncCityMapper::MapCityCodesAsync(ConfigVariables& config, MappingCallback callback) {
    bool dep_is_code = IsAlreadyCode(config.departure_town_);
    bool arr_is_code = IsAlreadyCode(config.arrival_town_);

    if (dep_is_code && arr_is_code) {
        callback(true, "Cities are already codes, no mapping needed");
        return;
    }

    LoadCitiesAsync([this, &config, callback, dep_is_code, arr_is_code](bool load_success
                                                    , const std::string& load_result)
    {
        if (!load_success) {
            callback(false, "Failed to load cities: " + load_result);
            return;
        }

        std::string task_id = "city_mapping_" + config.departure_town_ + "_" + config.arrival_town_;

        auto task = std::make_shared<CacheReadTask>(task_id, kCacheFilePath);

        scheduler_.ScheduleTask(task, [this, &config, callback, dep_is_code
                , arr_is_code](bool success, const std::string& result)
        {
            if (!success) {
                callback(false, "Failed to read cities cache");
                return;
            }

            std::istringstream iss(result);
            std::string line;
            bool dep_found = dep_is_code;
            bool arr_found = arr_is_code;
            std::string original_dep = config.departure_town_;
            std::string original_arr = config.arrival_town_;

            while (std::getline(iss, line)) {
                std::istringstream line_stream(line);
                std::string city, word, code;
                std::vector<std::string> words;
                while (line_stream >> word) {
                    words.push_back(word);
                }

                if (words.size() >= 2) {
                    code = words.back();
                    words.pop_back();

                    city = words[0];
                    for (std::size_t i = 1; i < words.size(); ++i) {
                        city += " " + words[i];
                    }

                    if (!dep_is_code && city == original_dep) {
                        config.departure_town_ = code;
                        dep_found = true;
                    }
                    if (!arr_is_code && city == original_arr) {
                        config.arrival_town_ = code;
                        arr_found = true;
                    }
                }
            }

            if (dep_found && arr_found) {
                callback(true, "Cities mapped successfully");
            } else {
                callback(false, "Failed to find city codes for: "
                        + (dep_found ? "" : original_dep + " ")
                        + (arr_found ? "" : original_arr));
            }

        });
    });
}


void AsyncCityMapper::LoadCitiesFromCache(MappingCallback callback) {
    if (!IsCacheValid()) {
        callback(false, "Cache is not valid or doesn't exist");
        return;
    }

    std::string task_id = "load_cities_cache";
    auto task = std::make_shared<CacheReadTask>(task_id, kCacheFilePath);

    scheduler_.ScheduleTask(task, [callback](bool success, const std::string& result)
        {
            if (success && !result.empty()) {
                callback(true, "Cities loaded from cache");
            } else {
                callback(false, "Cache file is empty or corrupted");
            }
        }
    );
}


void AsyncCityMapper::LoadCitiesFromApi(MappingCallback callback) {
    std::string url = BuildCitiesApiUrl();
    std::string task_id = "load_cities_api";

    auto task = std::make_shared<ApiRequestTask>(task_id, url, api_key_);

    scheduler_.ScheduleTask(task, [this, callback](bool success, const std::string& result) {
        if (!success) {
            callback(false, "Failed to load cities from API: " + result);
            return;
        }

        try {
            nlohmann::json api_json = nlohmann::json::parse(result);

            std::ostringstream cache_content;

            if (api_json.contains("countries")) {
                for (const auto& country : api_json["countries"]) {
                    if (country.contains("regions")) {
                        for (const auto& region : country["regions"]) {
                            if (region.contains("settlements")) {
                                for (const auto& settlement : region["settlements"]) {
                                    if (settlement.contains("title") &&
                                        settlement.contains("codes") &&
                                        settlement["codes"].contains("yandex_code")) {

                                        std::string city = settlement["title"];
                                        std::string code = settlement["codes"]["yandex_code"];
                                        cache_content << city << " " << code << "\n";
                                    }
                                }
                            }
                        }
                    }
                }
            }

            std::string cache_task_id = "save_cities_cache";
            auto cache_task = std::make_shared<CacheWriteTask>(cache_task_id, kCacheFilePath
                                                            , cache_content.str());

            scheduler_.ScheduleTask(cache_task, [callback](bool cache_success
                                            , const std::string& cache_result)
                {
                    if (cache_success) {
                        callback(true,
                            "Cities loaded and cached successfully");
                    } else {
                        callback(false, "Cities loaded but failed to cache: "
                            + cache_result);
                    }
                }
            );

        } catch (const nlohmann::json::parse_error& e) {
            callback(false, "Failed to parse cities API response: " + std::string(e.what()));
        }
    });
}


std::string AsyncCityMapper::BuildCitiesApiUrl() const {
    return std::string(kCitiesApiUrl) + api_key_ + kLanguageFlag + kResponseFormat;
}


bool AsyncCityMapper::IsCacheValid() const {
    std::ifstream file(kCacheFilePath);
    return file.is_open() && file.peek() != std::ifstream::traits_type::eof();
}

bool AsyncCityMapper::IsAlreadyCode(const std::string& city) const {
    if (city.empty()) {
        return false;
    }
    return (std::isalpha(city[0])
        && city.find(' ') == std::string::npos
        && city.length() >= 2
        && std::all_of(city.begin() + 1, city.end(), [](char c) { return std::isdigit(c); }));
}
