#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <fstream>
#include <sstream>

#include "../parser_cmd_lib/parser.h"
#include "../api_lib/api_manage.h"


struct YandexJsonCodeWords {
    const std::string kCountriesCodeWord = "countries";
    const std::string kRegionsCodeWord = "regions";
    const std::string kSettlementsCodeWord = "settlements";
    const std::string kTittleCodeWord = "title";
    const std::string kShortTittleCodeWord = "short_title";
    const std::string kCodesCodeWord = "codes";
    const std::string kYaCodeCodeWord = "yandex_code";
    const std::string kDepartureCodeWord = "departure";
    const std::string kArrivalCodeWord = "arrival";
    const std::string kSegmentsCodeWord = "segments";
    const std::string kTransportCodeWord = "transport_type";
    const std::string kDepStationCodeWord = "departure_from";
    const std::string kArrStationCodeWord = "arrival_to";
    const std::string kDetailsCodeWord = "details";
    const std::string kDurationCodeWord = "duration";
    const std::string kThreadCodeWord = "thread";
    const std::string kToCodeWord = "to";
    const std::string kFromCodeWord = "from";
    const std::string kHasTransfersCodeWord = "has_transfers";
    const std::string kIsTransferCodeWord = "is_transfer";
    const std::string kStationTypeNameCodeWord = "station_type_name";
    const std::string kSearchCodeWord = "search";
    const std::string kNumberCodeWord = "number";
    const std::string kCarrierCodeWord = "carrier";
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
    const std::string kDepCityCode = "departure_city";
    const std::string kArrCityCode = "arrival_city";
    const std::string kTransCode = "transport";
    const std::string kDepStationCode = "departure_station";
    const std::string kArrStationCode = "arrival_station";
    const std::string kDepTimeCode = "departure_time";
    const std::string kArrTimeCode = "arrival_time";
    const std::string kHasTransCode = "has_transfers";
};


class RoutesGetter {
public:
    RoutesGetter(const ApiMng& api_mng, const ConfigVariables& cfg);

private:
    YandexJsonCodeWords YaJsonWords;
    CsJsonCodeWords CsJsonWords;
    std::string full_file_path_ = "../../lib/app_files/cache_files/";    

private:
    void FillRouteFromSegment(const nlohmann::json& inp_json, const nlohmann::json& segment, RouteInfo& route);
    void CacheToFile(const nlohmann::json& inp_jsn);
    void PrintRoutesWithOutTransfers(RouteInfo& route);
    void PrintRoutesWithTransfers(const nlohmann::json& route_json, RouteInfo& route);
    void PrintAllRoutes();
    bool IsValidSegment(const nlohmann::json& segment);
    bool FileExistence();
};


std::string FormatTime(const std::string& time_str);