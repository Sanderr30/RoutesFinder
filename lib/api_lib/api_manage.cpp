#include "api_manage.h"


nlohmann::json ApiMng::GetJson() const {
    cpr::Response connect = cpr::Get(cpr::Url{api_http_});
    if (connect.status_code != 200) {
        std::cerr << "HTTP request failed with status code: " << connect.status_code << '\n';
        return nlohmann::json{};
    }
    nlohmann::json json = nlohmann::json::parse(connect.text);
    return json;
}


ApiMng::ApiMng(const std::string& api_key_value,  const ConfigVariables& cfg) { 
    api_key_ = api_key_value;
    api_http_ = kDefaultApiHttp + api_key_ + kResponceFormat + kFromCityFlag
                + cfg.departure_town_ + kToCityFlag + cfg.arrival_town_ + 
                kLanguageFlag + kDateFlag + cfg.date_ + kTransferFlag + kLimitFlag;
    Encode();
    SaveKey();
}


void ApiMng::SaveKey() const {
    std::fstream api_key_file(kDefaultApiKeyFilePath, std::ios::in | std::ios::out | std::ios::trunc);
    
    if (!api_key_file.is_open()) {
        std::cerr << "Unable to open the file\n";
        return;
    }

    api_key_file.clear();
    api_key_file.seekg(0, std::ios::beg);
    api_key_file << Encode();
    api_key_file.close();
}


std::string ApiMng::Encode() const {
    std::string encoded_key;
    
    for (size_t i = 0; i < api_key_.size(); ++i) {
        encoded_key += api_key_[i];
        
        if (i % 2 == 0) {
            char enc_symb = 'a' + (i % 26);
            encoded_key += enc_symb;
        }
    }

    return encoded_key;
}


std::string ApiMng::Decode(const std::string& encoded_api_key) const {
    std::string decoded_api_key;
    decoded_api_key += encoded_api_key[0];
    size_t ind = 2;

    while (ind < encoded_api_key.size()) {
        decoded_api_key += encoded_api_key[ind];
        
        if (ind + 1 < encoded_api_key.size()) {
            decoded_api_key += encoded_api_key[ind + 1];
        }

        ind += 3;
    }
    
    return decoded_api_key;
}