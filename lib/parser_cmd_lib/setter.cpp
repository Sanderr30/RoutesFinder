#include "setter.h"


Setter::Setter(ConfigVariables& cfg) {
    commands_creator.push_back(std::make_unique<HelpCommand>());
    commands_creator.push_back(std::make_unique<QuitCommand>());
    commands_creator.push_back(std::make_unique<DepartureTownCommand>());
    commands_creator.push_back(std::make_unique<ArrivalTownCommand>());
    commands_creator.push_back(std::make_unique<DateCommand>());
    commands_creator.push_back(std::make_unique<ShowConfigCommand>());
    SetConfig(cfg);
}


void Setter::SetConfig(ConfigVariables& cfg) {
    std::string input;    
    
    while (true) {
        std::getline(std::cin, input);
        
        for (size_t i = 0; i < commands_creator.size(); ++i) {
            commands_creator[i]->ExecuteCmd(input, cfg);
        }
        
        if (!cfg.date_.empty() && !cfg.arrival_town_.empty() && 
            !cfg.departure_town_.empty()) {
            break;
        }
    }
}