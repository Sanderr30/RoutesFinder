#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

#include "routes_handler.h"


std::string GetCurrentDateString() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << t.tm_year + 1900
        << std::setw(2) << std::setfill('0') << t.tm_mon + 1
        << std::setw(2) << std::setfill('0') << t.tm_mday;

    return oss.str();
}


AsyncRoutesHandler::AsyncRoutesHandler(TaskScheduler& scheduler
                                , AsyncApiManager& api_manager)
    : scheduler_(scheduler)
    , api_manager_(api_manager)
{}


void AsyncRoutesHandler::GetRoutesAsync(const ConfigVariables& config
                                        , RoutesCallback callback)
{
    TryLoadFromCache(config, [this, config, callback](bool cache_success
                                        , const std::string& cache_result)
        {
            if (cache_success) {
                callback(true, cache_result);
            } else {
                LoadFromApiAndCache(config, callback);
            }
        }
    );
}


void AsyncRoutesHandler::TryLoadFromCache(const ConfigVariables& config
                                        , RoutesCallback callback)
{
    std::string cache_path = GetCacheFilePath(config);
    std::string task_id = "cache_read_"
                        + config.departure_town_
                        + "_"
                        + config.arrival_town_
                        + "_"
                        + config.date_;

    auto task = std::make_shared<CacheReadTask>(task_id, cache_path);

    scheduler_.ScheduleTask(task, [this, cache_path, callback](bool success
                                            , const std::string& result)
        {
            if (success && IsCacheValid(cache_path)) {
                try {
                    nlohmann::json cache_data = nlohmann::json::parse(result);
                    std::string formatted_output = FormatRoutesOutput(cache_data);
                    callback(true, formatted_output);
                } catch (const nlohmann::json::parse_error& e) {
                    callback(false, "Cache parse error: "
                            + std::string(e.what()));
                }
            } else {
                callback(false, "Cache not found or invalid");
            }
        }
    );
}


void AsyncRoutesHandler::LoadFromApiAndCache(const ConfigVariables& config
                                            , RoutesCallback callback)
{
    api_manager_.GetRoutesAsync(config, [this, config, callback](bool success
                                            , const nlohmann::json& api_data)
        {
            if (success) {
                ProcessAndPrintRoutes(api_data, config, callback);
            } else {
                callback(false, "Failed to get data from API");
            }
        }
    );
}


std::string AsyncRoutesHandler::GetCacheFilePath(const ConfigVariables& config) const {
    std::string current_date = GetCurrentDateString();
    return std::string(kCacheDir)
            + current_date
            + "__"
            + config.date_
            + "_"
            + config.departure_town_
            + "_"
            + config.arrival_town_
            + ".json";
}


bool AsyncRoutesHandler::IsCacheValid(const std::string& cache_path) const {
    std::size_t last_slash = cache_path.find_last_of("/");

    if (last_slash == std::string::npos) {
        return false;
    }

    std::string filename = cache_path.substr(last_slash + 1);
    std::string cache_date = filename.substr(0, 8);
    std::string curr_date = GetCurrentDateString();

    if (curr_date.substr(0, 6) != cache_date.substr(0, 6)) {
        return false;
    }

    const int cache_day_date = std::stoi(cache_date.substr(6));
    const int curr_day_date = std::stoi(curr_date.substr(6));

    if (curr_day_date - cache_day_date > 1) {
        return false;
    }

    return true;
}


bool AsyncRoutesHandler::IsValidSegment(const nlohmann::json& segment) const {
    return (segment.contains("arrival")
            && segment.contains("departure")
            && segment.contains("from")
            && segment.contains("to")
            && segment["from"].contains("transport_type"))
            || (segment.contains("details") && !segment["details"].empty());
}


void AsyncRoutesHandler::FillRouteFromSegment(const nlohmann::json& inp_json
                                            , const nlohmann::json& segment
                                            , RouteInfo& route) const
{
    if (!inp_json.contains("search")) {
        route.departure_city = "Unknown";
        route.arrival_city = "Unknown";
    } else {
        const auto& search = inp_json["search"];

        if (search.contains("from") && search["from"].contains("title")) {
            route.departure_city = search["from"]["title"].get<std::string>();
        } else {
            route.departure_city = "Unknown";
        }

        if (search.contains("to") && search["to"].contains("title")) {
            route.arrival_city = search["to"]["title"].get<std::string>();
        } else {
            route.arrival_city = "Unknown";
        }
    }

    if (!segment.contains("details")) {
        if (segment.contains("from")) {
            const auto& departure_info = segment["from"];

            if (departure_info.contains("transport_type")) {
                route.transport = departure_info["transport_type"].get<std::string>();
            } else {
                route.transport = "Unknown";
            }

            std::string departure_type = departure_info.value("station_type_name", "");
            std::string departure_title = departure_info.value("title", "Unknown");
            route.departure_station = departure_type.empty() ? departure_title :
                                     departure_type + " - " + departure_title;
        } else {
            route.transport = "Unknown";
            route.departure_station = "Unknown";
        }

        if (segment.contains("to")) {
            const auto& arrival_info = segment["to"];
            std::string arrival_type = arrival_info.value("station_type_name", "");
            std::string arrival_title = arrival_info.value("title", "Unknown");
            route.arrival_station = arrival_type.empty() ? arrival_title :
                                   arrival_type + " - " + arrival_title;
        } else {
            route.arrival_station = "Unknown";
        }

        route.departure_time = segment.contains("departure") ?
                              FormatTime(segment["departure"]) : "Unknown";
        route.arrival_time = segment.contains("arrival") ?
                            FormatTime(segment["arrival"]) : "Unknown";
        route.has_transfers = segment.value("has_transfers", false);

    } else {
        route.has_transfers = true;
        const auto& details = segment["details"];

        if (!details.empty()) {
            const auto& first_detail = details.front();
            const auto& last_detail = details.back();

            if (first_detail.contains("from") && first_detail["from"].contains("title")) {
                route.departure_station = first_detail["from"]["title"].get<std::string>();
            } else {
                route.departure_station = "Unknown";
            }

            route.departure_time = first_detail.contains("departure") ?
                                  FormatTime(first_detail["departure"]) : "Unknown";

            if (last_detail.contains("to") && last_detail["to"].contains("title")) {
                route.arrival_station = last_detail["to"]["title"].get<std::string>();
            } else {
                route.arrival_station = "Unknown";
            }

            route.arrival_time = last_detail.contains("arrival") ?
                                FormatTime(last_detail["arrival"]) : "Unknown";

            for (const auto& detail : details) {
                bool is_transfer = detail.contains("is_transfer") &&
                                 detail["is_transfer"].get<bool>();
                if (!is_transfer) {
                    route.segments.push_back(detail);
                }
            }
        } else {
            route.departure_station = "Unknown";
            route.departure_time = "Unknown";
            route.arrival_station = "Unknown";
            route.arrival_time = "Unknown";
        }
    }
}


void AsyncRoutesHandler::ProcessAndPrintRoutes(const nlohmann::json& json_data,
                                              const ConfigVariables& config,
                                              RoutesCallback callback) {
    try {
        std::vector<RouteInfo> routes;
        if (!json_data.contains("segments")) {
            callback(false, "No segments found in API response");
            return;
        }
        for (const auto& segment : json_data["segments"]) {
            if (!IsValidSegment(segment)) {
                continue;
            }

            RouteInfo route;
            FillRouteFromSegment(json_data, segment, route);
            routes.push_back(route);
        }
        nlohmann::json cache_json;
        for (const auto& route : routes) {
            nlohmann::json route_json;
            route_json["departure_city"] = route.departure_city;
            route_json["arrival_city"] = route.arrival_city;
            route_json["transport"] = route.transport;
            route_json["departure_station"] = route.departure_station;
            route_json["departure_time"] = route.departure_time;
            route_json["arrival_station"] = route.arrival_station;
            route_json["arrival_time"] = route.arrival_time;
            route_json["has_transfers"] = route.has_transfers;

            if (route.has_transfers) {
                route_json["segments"] = route.segments;
            }
            cache_json.push_back(route_json);
        }

        std::string cache_path = GetCacheFilePath(config);
        std::string task_id = "cache_write_"
                            + config.departure_town_
                            + "_"
                            + config.arrival_town_
                            + "_"
                            + config.date_;

        auto cache_task = std::make_shared<CacheWriteTask>(task_id, cache_path, cache_json.dump(4));

        scheduler_.ScheduleTask(cache_task, [](bool success, const std::string& result)
            {
                if (!success) {
                    std::cerr << "Failed to write cache: " << result << std::endl;
                }
            }
        );

        std::string formatted_output = FormatRoutesOutput(cache_json);
        callback(true, formatted_output);

    } catch (const std::exception& e) {
        callback(false, "Error processing routes: " + std::string(e.what()));
    }
}

std::string AsyncRoutesHandler::FormatRoutesOutput(const nlohmann::json& cache_data) const {
    std::ostringstream output;
    output << "Все маршруты:\n\n\n\n";

    for (const auto& route_json : cache_data) {
        RouteInfo route;
        route.departure_city = route_json.value("departure_city", "Unknown");
        route.arrival_city = route_json.value("arrival_city", "Unknown");
        route.transport = route_json.value("transport", "Unknown");
        route.departure_station = route_json.value("departure_station", "Unknown");
        route.departure_time = route_json.value("departure_time", "Unknown");
        route.arrival_station = route_json.value("arrival_station", "Unknown");
        route.arrival_time = route_json.value("arrival_time", "Unknown");
        route.has_transfers = route_json.value("has_transfers", false);

        if (!route.has_transfers) {
            PrintRoutesWithoutTransfers(route, output);
        } else {
            PrintRoutesWithTransfers(route_json, route, output);
        }
        output << "/////////////////////////////////////////////////////////////\n\n";
    }

    return output.str();
}

void AsyncRoutesHandler::PrintRoutesWithoutTransfers(const RouteInfo& route
                                                    , std::ostringstream& output) const
{
    output << "Маршрут без пересадок: " << route.departure_city
           << " - " << route.arrival_city << "\n\n";
    output << "Транспорт: " << route.transport << "\n";
    output << "Станция отправления: " << route.departure_station << "\n";
    output << "Время отправления: " << route.departure_time << "\n";
    output << "Станция прибытия: " << route.arrival_station << "\n";
    output << "Время прибытия: " << route.arrival_time << "\n\n";
}


void AsyncRoutesHandler::PrintRoutesWithTransfers(const nlohmann::json& route_json
                                                , const RouteInfo& route
                                                , std::ostringstream& output) const
{
    output << "Маршрут с пересадками: " << route.departure_city
           << " - " << route.arrival_city << "\n";
    output << "Общее время в пути: " << route.departure_time
           << " - " << route.arrival_time << "\n\n";

    if (route_json.contains("segments")) {
        int segment_num = 1;

        for (const auto& segment : route_json["segments"]) {
            output << "  Рейс номер " << segment_num++ << ":\n";

            std::string dep_time = segment.contains("departure") ?
                                  FormatTime(segment["departure"].get<std::string>()) : "Unknown";
            std::string arr_time = segment.contains("arrival") ?
                                  FormatTime(segment["arrival"].get<std::string>()) : "Unknown";

            std::string transport = "Unknown";
            if (segment.contains("thread") && segment["thread"].contains("transport_type")) {
                transport = segment["thread"]["transport_type"].get<std::string>();
            }

            std::string from_title = "Unknown";
            if (segment.contains("from") && segment["from"].contains("title")) {
                from_title = segment["from"]["title"].get<std::string>();
            }

            std::string to_title = "Unknown";
            if (segment.contains("to") && segment["to"].contains("title")) {
                to_title = segment["to"]["title"].get<std::string>();
            }

            std::string number = "Unknown";
            std::string carrier = "Unknown";
            if (segment.contains("thread")) {
                const auto& thread = segment["thread"];
                number = thread.value("number", "Unknown");
                if (thread.contains("carrier") && thread["carrier"].contains("title")) {
                    carrier = thread["carrier"]["title"].get<std::string>();
                }
            }

            output << "  Транспорт: " << transport << "\n";
            output << "  Станция отправления: " << from_title << "\n";
            output << "  Время отправления: " << dep_time << "\n";
            output << "  Станция прибытия: " << to_title << "\n";
            output << "  Время прибытия: " << arr_time << "\n";
            output << "  Рейс: " << number << " Компания - " << carrier << "\n\n";
        }
    }
}
