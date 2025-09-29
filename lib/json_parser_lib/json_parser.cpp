#include "json_parser.h"


std::string GetCurrentDate() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << t.tm_year + 1900
        << std::setw(2) << std::setfill('0') << t.tm_mon + 1
        << std::setw(2) << std::setfill('0') << t.tm_mday;

    return oss.str();
}


std::string FormatTime(const std::string& time_str) {
    std::istringstream ss(time_str);
    std::tm tm = {};
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    std::ostringstream out;
    out << std::put_time(&tm, "%d.%m.%Y %H:%M");
    return out.str();
}


bool RoutesGetter::CacheValidity() {
    const std::string cache_date = full_file_path_.substr(0, 8);
    const std::string curr_date = GetCurrentDate();

    if (curr_date.substr(0, 6) != cache_date.substr(0, 6)) {
        return false;
    }

    const int cache_day_date = std::stoi(cache_date.substr(5));
    const int curr_day_date = std::stoi(curr_date.substr(5));

    if (curr_day_date - cache_day_date > 1) {
        return false;
    }

    return true;
}


bool RoutesGetter::FileExistence() {
    std::ifstream file(full_file_path_);
    return file.is_open();
}


bool RoutesGetter::IsValidSegment(const nlohmann::json& segment) {
    return
        (
            segment.contains(YaJsonWords.kArrivalCodeWord)
            && segment.contains(YaJsonWords.kDepartureCodeWord)
            && segment.contains(YaJsonWords.kFromCodeWord)
            && segment.contains(YaJsonWords.kToCodeWord)
            && segment[YaJsonWords.kFromCodeWord].contains(YaJsonWords.kTransportCodeWord)
        )
        || (segment.contains("details") && !segment["details"].empty());
}


RoutesGetter::RoutesGetter(const ApiMng& api_mng, const ConfigVariables& cfg) {
    const std::string current_date = GetCurrentDate();
    const std::string kAddedPath = current_date + + "__" + cfg.date_ + "_"
                                + cfg.departure_town_ + "_"
                                + cfg.arrival_town_ + ".json";
    full_file_path_ += kAddedPath;
    if (!FileExistence() && CacheValidity()) {
        CacheToFile(api_mng.GetJson());
    }
    PrintAllRoutes();
}


void RoutesGetter::FillRouteFromSegment(const nlohmann::json& inp_json,
                        const nlohmann::json& segment, RouteInfo& route)
{

    const auto& search = inp_json[YaJsonWords.kSearchCodeWord];
    const auto& from_city = search[YaJsonWords.kFromCodeWord];
    const auto& to_city = search[YaJsonWords.kToCodeWord];

    route.departure_city =
        from_city[YaJsonWords.kTittleCodeWord].get<std::string>();

    route.arrival_city =
        to_city[YaJsonWords.kTittleCodeWord].get<std::string>();

    if (!segment.contains(YaJsonWords.kDetailsCodeWord)) {

        const auto& departure_info = segment[YaJsonWords.kFromCodeWord];
        const auto& arrival_info = segment[YaJsonWords.kToCodeWord];

        route.transport =
            departure_info[YaJsonWords.kTransportCodeWord].get<std::string>();

        const std::string departure_type =
            departure_info[YaJsonWords.kStationTypeNameCodeWord];

        const std::string departure_title =
            departure_info[YaJsonWords.kTittleCodeWord];

        route.departure_station = departure_type
                                + std::string(" - ")
                                + departure_title;

        const std::string arrival_type =
            arrival_info[YaJsonWords.kStationTypeNameCodeWord];

        const std::string arrival_title =
            arrival_info[YaJsonWords.kTittleCodeWord];

        route.arrival_station = arrival_type + " - " + arrival_title;

        route.departure_time = FormatTime(segment[YaJsonWords.kDepartureCodeWord]);

        route.arrival_time = FormatTime(segment[YaJsonWords.kArrivalCodeWord]);

        route.has_transfers = segment[YaJsonWords.kHasTransfersCodeWord].get<bool>();

    } else {
        route.has_transfers = true;

        const auto& details = segment[YaJsonWords.kDetailsCodeWord];
        const auto& first_detail = details.front();
        const auto& last_detail = details.back();

        const auto& first_departure = first_detail[YaJsonWords.kFromCodeWord];

        route.departure_station = first_departure[YaJsonWords.kTittleCodeWord];
        route.departure_time = FormatTime(first_detail[YaJsonWords.kDepartureCodeWord]);

        const auto& last_arrival = last_detail[YaJsonWords.kToCodeWord];

        route.arrival_station = last_arrival[YaJsonWords.kTittleCodeWord];
        route.arrival_time = FormatTime(last_detail[YaJsonWords.kArrivalCodeWord]);

        for (const auto& detail : details) {
            const bool is_transfer = detail.contains(YaJsonWords.kIsTransferCodeWord)
                                && detail[YaJsonWords.kIsTransferCodeWord].get<bool>();

            if (!is_transfer) { route.segments.push_back(detail); }
        }
    }
}


void RoutesGetter::CacheToFile(const nlohmann::json& inp_json) {

    std::vector<RouteInfo> routes;

    for (const auto& segment : inp_json[YaJsonWords.kSegmentsCodeWord]) {
        if (!IsValidSegment(segment)) {
            continue;
        }

        RouteInfo route;
        FillRouteFromSegment(inp_json, segment, route);
        routes.push_back(route);
    }

    std::ofstream file(full_file_path_);

    if (!file.is_open()) {
        std::cerr << "Unable to open the file\n";
        return;
    }

    nlohmann::json cache_json;

    for (const auto& route : routes) {
        nlohmann::json route_json;
        route_json[CsJsonWords.kDepCityCode] = route.departure_city;
        route_json[CsJsonWords.kArrCityCode] = route.arrival_city;
        route_json[CsJsonWords.kTransCode] = route.transport;
        route_json[CsJsonWords.kDepStationCode] = route.departure_station;
        route_json[CsJsonWords.kDepTimeCode] = route.departure_time;
        route_json[CsJsonWords.kArrStationCode] = route.arrival_station;
        route_json[CsJsonWords.kArrTimeCode] = route.arrival_time;
        route_json[CsJsonWords.kHasTransCode] = route.has_transfers;

        if (route.has_transfers) {
            route_json[YaJsonWords.kSegmentsCodeWord] = route.segments;
        }

        cache_json.push_back(route_json);
    }

    file << cache_json.dump(4);
    file.close();
}


void RoutesGetter::PrintRoutesWithOutTransfers(RouteInfo& route) {
    std::cout << "Маршрут без пересадок: " <<
            route.departure_city << " - " << route.arrival_city  << "\n" << "\n";

    std::cout << "Транспорт: " << route.transport << "\n";

    std::cout << "Станция отправления: " << route.departure_station << "\n";

    std::cout << "Время отправления: " << route.departure_time << "\n";

    std::cout << "Станция прибытия: " << route.arrival_station << "\n";

    std::cout << "Время прибытия: " << route.arrival_time << "\n\n";

}


void RoutesGetter::PrintRoutesWithTransfers(const nlohmann::json& route_json, RouteInfo& route) {
    std::cout << "Маршрут с пересадками: "
                << route.departure_city << " - " << route.arrival_city << "\n";

    std::cout << "Общее время в пути: " << route.departure_time << " - "
                << route.arrival_time << "\n\n";

    if (route_json.contains(YaJsonWords.kSegmentsCodeWord)) {
        int segment_num = 1;

        for (const auto& segment : route_json[YaJsonWords.kSegmentsCodeWord]) {
            std::cout << "  Рейс номер " << segment_num++ << ":\n";

            std::string dep_time = FormatTime(segment[YaJsonWords.kDepartureCodeWord].get<std::string>());

            std::string arr_time = FormatTime(segment[YaJsonWords.kArrivalCodeWord].get<std::string>());

            std::cout << "  Транспорт: " << segment[YaJsonWords.kThreadCodeWord]
                                            [YaJsonWords.kTransportCodeWord].get<std::string>() << "\n";

            std::cout << "  Станция отправления: " << segment[YaJsonWords.kFromCodeWord]
                                            [YaJsonWords.kTittleCodeWord].get<std::string>();

            std::cout << "  Время отправления: " << dep_time << '\n';

            std::cout << "  Станция прибытия: " << segment[YaJsonWords.kToCodeWord]
                                            [YaJsonWords.kTittleCodeWord].get<std::string>();

            std::cout << "  Время прибытия: " << arr_time << '\n';

            std::cout << "  Рейс: " << segment[YaJsonWords.kThreadCodeWord]
                                            [YaJsonWords.kNumberCodeWord].get<std::string>() << " Компания - "
                                    << segment[YaJsonWords.kThreadCodeWord][YaJsonWords.kCarrierCodeWord]
                                    [YaJsonWords.kTittleCodeWord].get<std::string>() << "\n\n";
        }
    }
}


void RoutesGetter::PrintAllRoutes() {
    std::ifstream file(full_file_path_);
    nlohmann::json data_;
    file >> data_;
    file.close();

    std::cout << "Все маршруты:\n\n\n\n";

    for (const auto& route_json : data_) {

        RouteInfo route;
        route.departure_city = route_json[CsJsonWords.kDepCityCode];
        route.arrival_city = route_json[CsJsonWords.kArrCityCode];
        route.transport = route_json[CsJsonWords.kTransCode];
        route.departure_station = route_json[CsJsonWords.kDepStationCode];
        route.departure_time = route_json[CsJsonWords.kDepTimeCode];
        route.arrival_station = route_json[CsJsonWords.kArrStationCode];
        route.arrival_time = route_json[CsJsonWords.kArrTimeCode];
        route.has_transfers = route_json[CsJsonWords.kHasTransCode];

        if (!route.has_transfers) {
            PrintRoutesWithOutTransfers(route);

        } else {
            PrintRoutesWithTransfers(route_json, route);
        }
        std::cout << "/////////////////////////////////////////////////////////////\n\n";
    }
}
