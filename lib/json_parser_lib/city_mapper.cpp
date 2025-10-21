#include "city_mapper.h"


bool CityMapper::ValidKey() const {
    cpr::Response connect = cpr::Get(cpr::Url{api_http_}, cpr::Timeout{10000});

    if (connect.error) {
        std::cerr << "Connection timeout or other network error: " << connect.error.message << '\n';
        return false;
    }

    if (connect.status_code != 200) {
        std::string error = "Unable to connect, you have wrong api_key, error type: " + 
                             std::to_string(connect.status_code);
        std::cerr << error << '\n';
        return false;
    }
    return true;
}


CityMapper::CityMapper(const std::string& api_key_value, ConfigVariables& cfg) {
    api_key_ = api_key_value;
    api_http_ = kDefaultApiHttp + api_key_ + kLanguageFlag + kResponceFormat;
    CityCodeChanger(cfg);
}


void CityMapper::CityCodeChanger(ConfigVariables& cfg) {
    std::ifstream file(kPathToCitiesAndCodes);
    
    if (file.peek() == std::ifstream::traits_type::eof()) {
        file.close();
        LoadCache();
        file.open(kPathToCitiesAndCodes);
    }

    std::string buffer_line;
    bool DepCitySetted = false;
    bool ArrCitySetted = false;
    
    while (std::getline(file, buffer_line)) {
        std::istringstream iss(buffer_line);
        std::string city, word, code;
        
        while (iss >> word) {
            if (!iss.eof()) {
                if (!city.empty()) {
                    city += " ";
                }
                city += word;
            } else {
                code = word;
            }
        }
        
        if (city == cfg.departure_town_) {
            cfg.departure_town_ = code;
            DepCitySetted = true;
        } else if (city == cfg.arrival_town_) {
            cfg.arrival_town_ = code;
            ArrCitySetted = true;
        }
    }
    
    if (!DepCitySetted) {
        std::cout << "No such a city: " << cfg.departure_town_ << "\n";
    } 
    
    if (!ArrCitySetted) {
        std::cout << "No such a city: " << cfg.arrival_town_ << "\n";
    }
}


void CityMapper::LoadCache() {
    std::ofstream file(kPathToCitiesAndCodes);
    
    if (!file.is_open()) {
        std::cerr << "Unable to open the file\n";
        return;
    } else if (!ValidKey()) {
        std::cout << "incorrect api_key\n";
        return;
    }

    cpr::Response connect = cpr::Get(cpr::Url{api_http_});
    nlohmann::json api_json = nlohmann::json::parse(connect.text);
    
    const auto& countries = api_json[JsonWords.kCountriesCodeWord];
    
    for (const auto& country : countries) {
        const auto& regions = country[JsonWords.kRegionsCodeWord];

        for (const auto& region : regions) {
            const auto& settlements = region[JsonWords.kSettlementsCodeWord];

            for (const auto& settlement : settlements) {
                
                if (settlement.contains(JsonWords.kTittleCodeWord) && 
                    settlement.contains(JsonWords.kCodesCodeWord) && 
                    settlement[JsonWords.kCodesCodeWord].contains(JsonWords.kYaCodeCodeWord)) {
                    
                    std::string city = settlement[JsonWords.kTittleCodeWord];
                    std::string code = settlement[JsonWords.kCodesCodeWord][JsonWords.kYaCodeCodeWord];
                    file << city << " " << code << '\n';
                }
            }
        }
    }
    
    file.close();
}