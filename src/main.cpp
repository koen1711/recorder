
#include <string>
#include <filesystem>
#include <fstream>
#include "dashboard/dashboard.h"
#include "nlohmann/json.hpp"

int main(int argc, char** argv)
{
    // check if config.json exists
    // if not, create it with default values
    if (!std::filesystem::exists("config.json")) {
        std::ofstream file("config.json");
        nlohmann::json config;
        config["users"] = nlohmann::json::object();
        config["settings"] = {
            {"allow_unregistered_access", true},
            {"allow_unregistered_recording", true},
            {"allow_unregistered_streaming", true},
            {"allow_unregistered_configuration", true},
        };
        file << config.dump(4);
        file.close();
    }

    auto* dashboard = new Dashboard();
    dashboard->startServer();




    return 0;
}
