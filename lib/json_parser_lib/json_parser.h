#pragma once

#include <nlohmann/json.hpp>

#include <ctime>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <iomanip>
#include <cmath>

#include "../parser_cmd_lib/parser.h"
#include "../api_lib/api_manage.h"


std::string FormatTime(const std::string& time_str);


struct YandexJsonCodeWords {
    static constexpr const char* kCountriesCodeWord = "countries";
    static constexpr const char* kRegionsCodeWord = "regions";
    static constexpr const char* kSettlementsCodeWord = "settlements";
    static constexpr const char* kTittleCodeWord = "title";
    static constexpr const char* kShortTittleCodeWord = "short_title";
    static constexpr const char* kCodesCodeWord = "codes";
    static constexpr const char* kYaCodeCodeWord = "yandex_code";
    static constexpr const char* kDepartureCodeWord = "departure";
    static constexpr const char* kArrivalCodeWord = "arrival";
    static constexpr const char* kSegmentsCodeWord = "segments";
    static constexpr const char* kTransportCodeWord = "transport_type";
    static constexpr const char* kDepStationCodeWord = "departure_from";
    static constexpr const char* kArrStationCodeWord = "arrival_to";
    static constexpr const char* kDetailsCodeWord = "details";
    static constexpr const char* kDurationCodeWord = "duration";
    static constexpr const char* kThreadCodeWord = "thread";
    static constexpr const char* kToCodeWord = "to";
    static constexpr const char* kFromCodeWord = "from";
    static constexpr const char* kHasTransfersCodeWord = "has_transfers";
    static constexpr const char* kIsTransferCodeWord = "is_transfer";
    static constexpr const char* kStationTypeNameCodeWord = "station_type_name";
    static constexpr const char* kSearchCodeWord = "search";
    static constexpr const char* kNumberCodeWord = "number";
    static constexpr const char* kCarrierCodeWord = "carrier";
};


struct RouteInfo {
    std::string departure_city;
    std::string arrival_city;
    std::string transport;
    std::string departure_station;
    std::string departure_time;
    std::string arrival_station;
    std::string arrival_time;

    bool has_transfers;
    std::vector<nlohmann::json> segments;
};


struct CsJsonCodeWords {
    static constexpr const char* kDepCityCode = "departure_city";
    static constexpr const char* kArrCityCode = "arrival_city";
    static constexpr const char* kTransCode = "transport";
    static constexpr const char* kDepStationCode = "departure_station";
    static constexpr const char* kArrStationCode = "arrival_station";
    static constexpr const char* kDepTimeCode = "departure_time";
    static constexpr const char* kArrTimeCode = "arrival_time";
    static constexpr const char* kHasTransCode = "has_transfers";
};


class RoutesGetter {
public:
    RoutesGetter(const ApiMng& api_mng, const ConfigVariables& cfg);

private:
    void FillRouteFromSegment(const nlohmann::json& inp_json
                , const nlohmann::json& segment, RouteInfo& route);

    void CacheToFile(const nlohmann::json& inp_jsn);

    void PrintRoutesWithOutTransfers(RouteInfo& route);

    void PrintRoutesWithTransfers(const nlohmann::json& route_json
                , RouteInfo& route);

    void PrintAllRoutes();

    bool IsValidSegment(const nlohmann::json& segment);

    bool FileExistence();

    bool CacheValidity();

private:
    YandexJsonCodeWords YaJsonWords;
    CsJsonCodeWords CsJsonWords;
    std::string full_file_path_ = "../../lib/app_files/cache_files/";
};
