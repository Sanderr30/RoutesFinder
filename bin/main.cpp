#include <cstdlib>
#include <iostream>
#include <string>

#include <lib/api_lib/api_manage.h>
#include <lib/parser_cmd_lib/setter.h>
#include <lib/json_parser_lib/city_mapper.h>
#include <lib/json_parser_lib/json_parser.h>
#include <lib/parser_cmd_lib/parser.h>

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cerr << "incorrect parameters to start the programm \n";
        return -1;
    }

    std::string api_key = argv[1];
    ConfigVariables cfg;
    Setter setter = {cfg};
    CityMapper city_mapper = {api_key, cfg};
    ApiMng api_manage = {api_key, cfg};
    RoutesGetter routes = {api_manage, cfg};
    return 0;
}
