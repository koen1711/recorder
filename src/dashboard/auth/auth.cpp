#include <fstream>
#include "auth.h"
#include "json.hpp"

bool checkAuth(uWS::HttpResponse<true> *res, uWS::HttpRequest *req, bool allowUnregisteredAccess, const std::string& redirect_uri, bool return403) {
    if (allowUnregisteredAccess) {
        return true;
    }

    // print the cookies of the user
    if (req->getHeader("cookie").find("token=") == std::string::npos) {
        if (return403) {
            res->writeStatus("403 Forbidden");
            res->end();
            return false;
        }
        res->writeStatus("302 Found");
//            Tell the browser to redirect to /login and the error type
        res->writeHeader("Location", "/login?redirect_uri=" + redirect_uri + "&error=notoken");
        res->end();
        return false;
    }
    std::string token = std::string{req->getHeader("cookie").substr(req->getHeader("cookie").find("token=") + 6)};
    // parse the file with users at config.json
    nlohmann::json j;
    std::ifstream configFile("config.json");
    if (configFile.is_open()) {
        configFile >> j;
        configFile.close();
    }
    // check if the token is correct by looking in the config.tokens list and for the expiration date
    if (!j["tokens"].contains(token)) {
        if (return403) {
            res->writeStatus("403 Forbidden");
            res->end();
            return false;
        }
        res->writeStatus("302 Found");
        res->writeHeader("Location", "/login?redirect_uri=" + redirect_uri + "&error=invalidtoken");
        res->end();
        return false;
    }
    if (j["tokens"][token]["expiration"] < std::chrono::system_clock::now().time_since_epoch().count()) {
        if (return403) {
            res->writeStatus("403 Forbidden");
            res->end();
            return false;
        }
        res->writeStatus("302 Found");
        res->writeHeader("Location", "/login?redirect_uri=" + redirect_uri + "&error=expiredtoken");
        res->end();
        // delete the token from the config.json
        nlohmann::json json;
        std::ifstream file("config.json");
        if (file.is_open()) {
            file >> json;
            file.close();
        }
        json["tokens"].erase(token);
        std::ofstream outFile("config.json");
        outFile << json.dump(4);
        outFile.close();
        return false;
    }
    return true;
}