#pragma once

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <functional>
#include <sstream>

#include "task_scheduler.h"
#include "api_manager.h"
#include "../parser_cmd_lib/parser.h"
#include "../json_parser_lib/json_parser.h"
#include <nlohmann/json.hpp>


class AsyncRoutesHandler {
public:
    using RoutesCallback = std::function<void(bool success
                            , const std::string& message)>;

    AsyncRoutesHandler(TaskScheduler& scheduler
                    , AsyncApiManager& api_manager);

    void GetRoutesAsync(const ConfigVariables& config
                        , RoutesCallback callback);

private:
    void TryLoadFromCache(const ConfigVariables& config
                        , RoutesCallback callback);

    void LoadFromApiAndCache(const ConfigVariables& config
                            , RoutesCallback callback);

    std::string GetCacheFilePath(const ConfigVariables& config) const;

    bool IsCacheValid(const std::string& cache_path) const;

    bool IsValidSegment(const nlohmann::json& segment) const;

    void FillRouteFromSegment(const nlohmann::json& inp_json
                            , const nlohmann::json& segment
                            , RouteInfo& route) const;

    void ProcessAndPrintRoutes(const nlohmann::json& json_data
                            , const ConfigVariables& config
                            , RoutesCallback callback);

    std::string FormatRoutesOutput(const nlohmann::json& cache_data) const;

    void PrintRoutesWithoutTransfers(const RouteInfo& route
                                    , std::ostringstream& output) const;

    void PrintRoutesWithTransfers(const nlohmann::json& route_json
                                , const RouteInfo& route
                                , std::ostringstream& output) const;

private:
    TaskScheduler& scheduler_;
    AsyncApiManager& api_manager_;

private:
    static constexpr const char* kCacheDir = "../../lib/app_files/cache_files/";
};
