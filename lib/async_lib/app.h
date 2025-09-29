#pragma once

#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "task_scheduler.h"
#include "input_handler.h"
#include "api_manager.h"
#include "routes_handler.h"
#include "../parser_cmd_lib/parser.h"
#include "city_mapper.h"
#include <functional>


class App {
public:
    App(const std::string& api_key);

    ~App();

    void Run();

    void Stop();

    void ValidateApiKeyAsync(std::function<void(bool)> callback);

private:
    void HandleUserInput(const std::string& input);
    void ProcessCommand(const std::string& command);
    void ShowHelp();
    void ShowConfig();
    void HandleRouteRequest();

    bool ValidateConfig() const;

private:
    std::unique_ptr<TaskScheduler> scheduler_;
    std::unique_ptr<AsyncInputHandler> input_handler_;
    std::unique_ptr<AsyncApiManager> api_manager_;
    std::unique_ptr<AsyncRoutesHandler> routes_handler_;
    std::unique_ptr<AsyncCityMapper> city_mapper_;

    ConfigVariables config_;
    std::string api_key_;
    bool running_;
};
