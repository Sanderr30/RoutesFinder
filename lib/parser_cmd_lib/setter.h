#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include "parser.h"


class Setter {
public:
    Setter(ConfigVariables& cfg); 
    void SetConfig(ConfigVariables& cfg);

private:
    std::vector<std::unique_ptr<BaseParse>> commands_creator;
};