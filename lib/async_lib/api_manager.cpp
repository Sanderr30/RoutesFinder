#include "api_manager.h"


AsyncApiManager::AsyncApiManager(TaskScheduler& scheduler
                            , const std::string& api_key)
    : scheduler_(scheduler)
    , api_key_(api_key)
{}


void AsyncApiManager::GetRoutesAsync(const ConfigVariables& config
                                    , ApiResponseCallback callback)
{
    std::string url = BuildApiUrl(config);
    std::string task_id = "api_request_"
                        + config.departure_town_
                        + "_"
                        + config.arrival_town_
                        + "_" + config.date_;

    if (scheduler_.IsTaskRunning(task_id)) {
        callback(false, nlohmann::json{});
        return;
    }

    auto task = std::make_shared<ApiRequestTask>(task_id, url, api_key_);

    scheduler_.ScheduleTask(task, [callback](bool success, const std::string& result)
        {
            if (success) {
                try {
                    nlohmann::json json_data = nlohmann::json::parse(result);
                    callback(true, json_data);
                } catch (const nlohmann::json::parse_error& e) {
                    std::cerr << "JSON parse error: " << e.what() << std::endl;
                    callback(false, nlohmann::json{});
                }
            } else {
                std::cerr << "API request failed: " << result << std::endl;
                callback(false, nlohmann::json{});
            }
        }
    );
}


std::string AsyncApiManager::BuildApiUrl(const ConfigVariables& config) const {
    return (std::string(kDefaultApiHttp)
            + api_key_
            + kResponseFormat
            + kFromCityFlag
            + config.departure_town_
            + kToCityFlag
            + config.arrival_town_
            + kLanguageFlag
            + kDateFlag
            + config.date_
            + kTransferFlag
            + kLimitFlag);
}
