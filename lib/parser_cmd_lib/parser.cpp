#include "parser.h"


void HelpCommand::ExecuteCmd(const std::string& command, ConfigVariables& cfg) {
    
    if (command == "--help") {
        std::cout << "Available commands:\n";
        std::cout << "--from <city> - departure city\n";
        std::cout << "--to <city> - arrival city\n";
        std::cout << "--date <date> - date in format YYYY-MM-DD\n";
        std::cout << "--show - shows current configuration\n";
        std::cout << "--quit - finish the programm\n";
    }
}


void QuitCommand::ExecuteCmd(const std::string& command, ConfigVariables& cfg) {
    
    if (command == "--quit") {
        std::cout << "Programm finished\n";
        exit(0);
    }
}


void DepartureTownCommand::ExecuteCmd(const std::string& command, ConfigVariables& cfg) {
    
    if (command.substr(0, 6) == "--from") {
        cfg.departure_town_ = command.substr(7);
    }    
}


void ArrivalTownCommand::ExecuteCmd (const std::string& command, ConfigVariables& cfg) {
    
    if (command.substr(0, 4) == "--to") {
        cfg.arrival_town_ = command.substr(5);
    }
}


void DateCommand::ExecuteCmd(const std::string& command, ConfigVariables& cfg) {
    
    if (command.substr(0, 6) == "--date") {
        
        if (!DataValidility(command.substr(7))) {
            std::cerr << "incorrect date format, try again\n";
        } else {
            cfg.date_ = command.substr(7);
        }    
    }
}


bool DateCommand::DataValidility(const std::string& date) {
    for (size_t i = 0; i < date.size(); ++i) {
        
        if ((i != 4 && i != 7) && !isdigit(date[i])) {
            return false;
        }
    }

    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));
    
    if (date.size() != 10 || date[4] != '-' || date[7] != '-'
        || month < 1 || month > 12 || day < 1) {
        return false;
    }
    
    const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (day > days_in_month[month - 1]) {
        
        if (month == 2) {
            bool is_visokosny = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            return is_visokosny && day == 29;
        }
        
        return false;
    }

    return true;
}


void ShowConfigCommand::ExecuteCmd(const std::string& command, ConfigVariables& cfg) {
    
    if (command == "--show") {
        std::cout << "Current settings:\n";
        std::cout << "Departure: " << cfg.departure_town_ << "\n";
        std::cout << "Arrival: " << cfg.arrival_town_ << "\n";
        std::cout << "Date: " << cfg.date_ << "\n";
    }
}