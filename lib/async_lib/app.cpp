#include "app.h"


App::App(const std::string& api_key)
    : api_key_(api_key)
    , running_(false)
{
    scheduler_ = std::make_unique<TaskScheduler>(4);
    input_handler_ = std::make_unique<AsyncInputHandler>(scheduler_->GetIOContext());
    api_manager_ = std::make_unique<AsyncApiManager>(*scheduler_, api_key_);
    routes_handler_ = std::make_unique<AsyncRoutesHandler>(*scheduler_, *api_manager_);
    city_mapper_ = std::make_unique<AsyncCityMapper>(*scheduler_, api_key_);
}


App::~App() {
    Stop();
}


void App::Run() {
    running_ = true;
    ValidateApiKeyAsync([this](bool is_valid) {
        if (!is_valid) {
            input_handler_->PrintMessage("Некорректный API ключ");
            auto shutdown_task = std::make_shared<CacheReadTask>("shutdown_task", "");
            scheduler_->ScheduleDelayedTask(
                shutdown_task,
                [this](bool, const std::string&) {
                    Stop();
                },
                std::chrono::milliseconds(100)
            );
            return;
        }
        input_handler_->PrintMessage("Введите 'help' для просмотра команд");
        input_handler_->StartReading([this](const std::string& input) {
            HandleUserInput(input);
        });
    });
    scheduler_->Run();
}



void App::Stop() {
    if (running_) {
        running_ = false;
        input_handler_->StopReading();
        scheduler_->Stop();
    }
}


void App::HandleUserInput(const std::string& input) {
    if (!running_) {
        return;
    }
    std::string trimmed_input = input;
    trimmed_input.erase(0, trimmed_input.find_first_not_of(" \t"));
    trimmed_input.erase(trimmed_input.find_last_not_of(" \t") + 1);

    if (trimmed_input.empty()) {
        return;
    }

    ProcessCommand(trimmed_input);
}


void App::ProcessCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    if (cmd == "help") {
        ShowHelp();
    } else if (cmd == "exit" || cmd == "quit") {
        Stop();
    } else if (cmd == "from") {
        std::string city;
        std::getline(iss, city);
        if (!city.empty()) {
            city.erase(0, city.find_first_not_of(" \t"));
            config_.departure_town_ = city;
        }
    } else if (cmd == "to") {
        std::string city;
        std::getline(iss, city);
        if (!city.empty()) {
            city.erase(0, city.find_first_not_of(" \t"));
            config_.arrival_town_ = city;
        }
    } else if (cmd == "date") {
        std::string date;
        iss >> date;
        if (!date.empty()) {
            config_.date_ = date;
        }
    } else if (cmd == "config") {
        ShowConfig();
    } else if (cmd == "search") {
        HandleRouteRequest();
    } else {
        input_handler_->PrintMessage("Неизвестная команда, введите Help");
    }
}

void App::ShowHelp() {
    std::string help_text = R"(
Доступные команды:
  help                    - показать эту справку
  from <город>       - установить город отправления
  to <город>         - установить город прибытия
  date <YYYY-MM-DD>        - установить дату поездки
  config                 - показать текущие настройки
  search                 - найти маршруты
  quit                   - выйти из программы
        )";
    input_handler_->PrintMessage(help_text);
}

void App::ShowConfig() {
    std::ostringstream config_output;
    config_output << "  Город отправления: "
                << (config_.departure_town_.empty() ? "не установлен" : config_.departure_town_)
                << "\n";

    config_output << "  Город прибытия: "
                << (config_.arrival_town_.empty() ? "не установлен" : config_.arrival_town_)
                << "\n";

    config_output << "  Дата: "
                << (config_.date_.empty() ? "не установлена" : config_.date_)
                << "\n";

    input_handler_->PrintMessage(config_output.str());
}


void App::HandleRouteRequest() {
    if (!ValidateConfig()) {
        input_handler_->PrintMessage("Не все поля установлены");
        return;
    }

    city_mapper_->MapCityCodesAsync(config_, [this](bool mapping_success, const std::string& mapping_result) {
        if (mapping_success) {
            routes_handler_->GetRoutesAsync(config_, [this](bool success, const std::string& result) {
                if (success) {
                    input_handler_->PrintMessage(result);
                } else {
                    input_handler_->PrintMessage("Ошибка при получении маршрутов: " + result);
                }
            });
        } else {
            input_handler_->PrintMessage("Ошибка маппинга городов: " + mapping_result);
        }
    });
}


bool App::ValidateConfig() const {
    return (!config_.departure_town_.empty()
            && !config_.arrival_town_.empty()
            && !config_.date_.empty());
}


void App::ValidateApiKeyAsync(std::function<void(bool)> callback) {
    std::string test_url = "http://api.rasp.yandex.net/v3.0/stations_list/?apikey="
                          + api_key_
                          + "&lang=ru_RU&format=json&limit=1";

    std::string task_id = "api_key_validation";
    auto task = std::make_shared<ApiRequestTask>(task_id, test_url, api_key_);

    scheduler_->ScheduleTask(task, [callback](bool success, const std::string& result) {
        if (success) {
            try {
                nlohmann::json test_response = nlohmann::json::parse(result);
                if (test_response.contains("error")) {
                    callback(false);
                } else if (test_response.contains("countries") ||
                          test_response.contains("stations") ||
                          test_response.is_array()) {
                    callback(true);
                } else {
                    callback(false);
                }
            } catch (const nlohmann::json::parse_error&) {
                callback(false);
            }
        } else {
            callback(false);
        }
    });
}
