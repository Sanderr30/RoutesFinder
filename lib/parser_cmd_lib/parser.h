#pragma once

#include <iostream>
#include <string>
#include <vector>


struct ConfigVariables {
    std::string date_;
    std::string departure_town_;
    std::string arrival_town_;
};


class BaseParse {
public:
    virtual void ExecuteCmd(const std::string& command, ConfigVariables& cfg) = 0;
    virtual ~BaseParse() {}
};


class HelpCommand : public BaseParse {
public:
    void ExecuteCmd(const std::string& command, ConfigVariables& cfg) override;       
};


class QuitCommand : public BaseParse {
public:
    void ExecuteCmd(const std::string& command, ConfigVariables& cfg) override;
};


class DepartureTownCommand : public BaseParse {
public:
    void ExecuteCmd(const std::string& command, ConfigVariables& cfg) override;
};


class ArrivalTownCommand : public BaseParse {
public:
    void ExecuteCmd (const std::string& command, ConfigVariables& cfg) override;
};


class DateCommand : public BaseParse {
public:
    void ExecuteCmd(const std::string& command, ConfigVariables& cfg) override;
private:
    bool DataValidility(const std::string& command);
};


class ShowConfigCommand : public BaseParse {
public:
    void ExecuteCmd(const std::string& command, ConfigVariables& cfg) override;
};