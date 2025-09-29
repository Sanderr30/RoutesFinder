#include <iostream>
#include <string>
#include <signal.h>

#include "../lib/async_lib/app.h"


App* g_app = nullptr;

void SignalHandler(int signal) {
    if (g_app && (signal == SIGINT || signal == SIGTERM)) {
        g_app->Stop();
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "введите API_KEY для использования программы" << std::endl;
        return -1;
    }

    std::string api_key = argv[1];
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    try {
        App app(api_key);
        g_app = &app;

        app.Run();

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
