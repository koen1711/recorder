#include "dashboard.h"
#include "cpu/CpuInfo.h"
#include "../recording/recorders.h"
#include <uWebSockets/App.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sys/statvfs.h>
#include "app.h"

BaseRecorder* selectedRecorder = nullptr;
bool recording = false;


struct PerSocketData {
    /* Fill with user data */
    std::vector<std::string> topics;
    int nr = 0;
};

/* Middleware to fill out content-type */
inline bool hasExt(std::string_view file, std::string_view ext) {
    if (ext.size() > file.size()) {
        return false;
    }
    return std::equal(ext.rbegin(), ext.rend(), file.rbegin());
}

/* This should be a filter / middleware like app.use(handler) */
template <bool SSL>
uWS::HttpResponse<SSL> *serveFile(uWS::HttpResponse<SSL> *res, uWS::HttpRequest *req) {
    res->writeStatus(uWS::HTTP_200_OK);

    if (hasExt(req->getUrl(), ".svg")) {
        res->writeHeader("Content-Type", "image/svg+xml");
    } else if (hasExt(req->getUrl(), ".css")) {
        res->writeHeader("Content-Type", "text/css");
    } else if (hasExt(req->getUrl(), ".js")) {
        res->writeHeader("Content-Type", "application/javascript");
    } else if (hasExt(req->getUrl(), ".json")) {
        res->writeHeader("Content-Type", "application/json");
    } else if (hasExt(req->getUrl(), ".html")) {
        res->writeHeader("Content-Type", "text/html");
    }
    return res;
}

nlohmann::json previousDeviceInformation;

void publishDeviceInformation(bool newInfo = false) {
    nlohmann::json j;
    j["type"] = "device-info";
    j["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
    // check how big the root filesystem is so / and how much is used
    // so ["storage"]["total/free/available"] and convert it to GB and only send 1 decimal
    struct statvfs stat{};
    statvfs("/", &stat);

    unsigned long long totalBytes = stat.f_blocks * stat.f_frsize;
    unsigned long long freeBytes = stat.f_bfree * stat.f_frsize;
    unsigned long long availableBytes = stat.f_bavail * stat.f_frsize;

    j["storage"]["total"] = totalBytes;
    j["storage"]["free"] = freeBytes;
    j["storage"]["available"] = availableBytes;
    // convert to GB with a single decimal
    j["storage"]["total"] = std::round(j["storage"]["total"].get<unsigned long long>() / 1024.0 / 1024.0 / 1024.0 * 10) / 10;
    j["storage"]["free"] = std::round(j["storage"]["free"].get<unsigned long long>() / 1024.0 / 1024.0 / 1024.0 * 10) / 10;
    j["storage"]["available"] = std::round(j["storage"]["available"].get<unsigned long long>() / 1024.0 / 1024.0 / 1024.0 * 10) / 10;


    // check how much memory, cpu and disk is used
    std::ifstream file("/proc/meminfo");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("MemTotal") != std::string::npos) {
            j["memory"]["total"] = std::stoi(line.substr(line.find(":") + 1));
        } else if (line.find("MemFree") != std::string::npos) {
            j["memory"]["free"] = std::stoi(line.substr(line.find(":") + 1));
        } else if (line.find("MemAvailable") != std::string::npos) {
            j["memory"]["available"] = std::stoi(line.substr(line.find(":") + 1));
        }
    }
    file.close();
    j["memory"]["used"] = j["memory"]["total"].get<int>() - j["memory"]["available"].get<int>();
    // delete free and available keys
    j["memory"].erase("free");
    j["memory"].erase("available");
    // convert to GB with a single decimal
    j["memory"]["total"] = std::round(j["memory"]["total"].get<int>() / 1024.0 / 1024.0 * 10) / 10;
    j["memory"]["used"] = std::round(j["memory"]["used"].get<int>() / 1024.0 / 1024.0 * 10) / 10;

    // find the usage of the cpu
    j["cpu"]["usage"] = GlobalCpuInfo::getCurrentUsage();
    j["cpu"]["processUsage"] = ProcessCpuInfo::getCurrentUsage();

    nlohmann::json currentDeviceInformation = j;

    // check if previousDeviceInformation is empty
    if (!previousDeviceInformation.empty() && !newInfo) {
        // check if the cpu usage is different
        if (previousDeviceInformation["cpu"]["usage"] == j["cpu"]["usage"]) {
            j["cpu"].erase("usage");
        }
        if (previousDeviceInformation["cpu"]["processUsage"] == j["cpu"]["processUsage"]) {
            j["cpu"].erase("processUsage");
        }
        // check if the memory usage is different
        if (previousDeviceInformation["memory"]["used"] == j["memory"]["used"]) {
            j["memory"].erase("used");
        }
        // check if the storage usage is different
        if (previousDeviceInformation["storage"]["free"] == j["storage"]["free"]) {
            j["storage"].erase("free");
        }
        if (previousDeviceInformation["storage"]["available"] == j["storage"]["available"]) {
            j["storage"].erase("available");
        } if (previousDeviceInformation["storage"]["total"] == j["storage"]["total"]) {
            j["storage"].erase("total");
        }
    }

    previousDeviceInformation = currentDeviceInformation;

    pApp->publish("device-info", j.dump(), uWS::OpCode::TEXT);
}

Dashboard::Dashboard() {
}

void Dashboard::startServer() {
    GlobalCpuInfo::init();
    ProcessCpuInfo::init();

    pApp->get("/auth", [](auto *res, auto *req) {
        // return the html content of /index.html
        serveFile(res, req);
        // get content of /index.html
        std::string content = "";
        std::ifstream file("ui/index.html");
        if (file.is_open()) {
            std::string line;
            while (getline(file, line)) {
                content += line;
            }
            file.close();
        }
        res->end(content);
    });


    pApp->get("/", [](auto *res, auto *req) {
        // if a code param is present, rederict to /auth with the same param
        if (req->getQuery().find("code") != std::string::npos) {
            res->writeStatus("302 Found");
            res->writeHeader("Location", std::string("/auth?") + std::string(req->getQuery()));
            res->end();
            return;
        }
        // return the html content of /index.html
        serveFile(res, req);
        // get content of /index.html
        std::string content = "";
        std::ifstream file("ui/index.html");
        if (file.is_open()) {
            std::string line;
            while (getline(file, line)) {
                content += line;
            }
            file.close();
        }
        res->end(content);
    });

    pApp->get("/audiostream", [](auto *res, auto *req) {


        bool re = selectedRecorder->registerListener(reinterpret_cast<uWS::HttpResponse<true> *>(res), req);
        if (re) {
            res->writeStatus(uWS::HTTP_200_OK);
            res->writeHeader("Content-Type", "audio/x-wav");
            res->writeHeader("Connection", "keep-alive");
            res->writeHeader("Cache-Control", "no-cache");
        }
    });


    pApp->ws<PerSocketData>("/ws", {
            /* Settings */
            .compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR_4KB | uWS::DEDICATED_DECOMPRESSOR),
            .maxPayloadLength = 100 * 1024 * 1024,
            .idleTimeout = 60,
            .maxBackpressure = 100 * 1024 * 1024,
            .closeOnBackpressureLimit = false,
            .resetIdleTimeoutOnSend = true,
            .sendPingsAutomatically = false,
            /* Handlers */
            .upgrade = nullptr,
            .open = [](auto *ws) {
                /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
                ws->getUserData()->nr = 0;
                ws->subscribe("device-info");
                ws->subscribe("recorder-selected");
                ws->subscribe("recorder-info");
                ws->subscribe("config");
                ws->subscribe("recording-status");

                nlohmann::json response;
                response["type"] = "recording-status";
                response["recording"] = recording;
                response["selectedRecorder"] = selectedRecorder != nullptr ? selectedRecorder->getName() : "";
                response["recorders"] = nlohmann::json::array();
                for (auto &recorder : recorders) {
                    response["recorders"].push_back(recorder->getName());
                }
                ws->send(response.dump(), uWS::OpCode::TEXT);
                publishDeviceInformation(true);
            },
            .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                // Load the json of the message
                nlohmann::json j = nlohmann::json::parse(message);

                if (j["type"] == "query-recorders") {
                    nlohmann::json response;
                    response["type"] = "recorders";
                    response["recorders"] = nlohmann::json::array();
                    if (selectedRecorder != nullptr) {
                        response["selectedRecorder"] = selectedRecorder->getName();
                    }
                    for (auto &recorder : recorders) {
                        response["recorders"].push_back(recorder->getName());
                    }
                    ws->send(response.dump(), opCode);
                } else if (j["type"] == "select-recorder") {
                    for (auto &recorder : recorders) {
                        if (recorder->getName() == j["recorder"]) {
                            if (selectedRecorder != recorder) {
                                recorder->init();
                                selectedRecorder = recorder;
                            }
                            nlohmann::json response;
                            response["type"] = "recorder-selected";
                            response["recorder"] = recorder->getName();
                            pApp->publish("recorder-selected", response.dump(), uWS::OpCode::TEXT);
                            break;
                        }
                    }
                } else if (j["type"] == "query-config") {
                    if (selectedRecorder != nullptr) {
                        pApp->publish("config", selectedRecorder->queryConfiguration().dump(), opCode);
                    }
                } else if (j["type"] == "configure") {
                    if (selectedRecorder != nullptr) {
                        int re = selectedRecorder->configure(j["config"]);
                        if (re == 0) {
                            nlohmann::json response;
                            response["type"] = "valid-config";
                            ws->send(response.dump(), opCode);
                            pApp->publish("config", selectedRecorder->queryConfiguration().dump(), opCode);
                        } else if (re == -1) {
                            // send to the individual user the config error
                            nlohmann::json response;
                            response["type"] = "invalid-config";
                            response["error"] = "channelCount";
                            ws->send(response.dump(), opCode);
                        }
                    }
                } else if (j["type"] == "start-recording") {
                    if (selectedRecorder != nullptr) {
                        selectedRecorder->startRecording();
                        recording = true;
                        nlohmann::json response;
                        response["type"] = "recording-status";
                        response["recording"] = true;
                        pApp->publish("recording-status", response.dump(), opCode);
                    }
                } else if (j["type"] == "stop-recording") {
                    if (selectedRecorder != nullptr) {
                        selectedRecorder->stopRecording();
                        recording = false;
                        nlohmann::json response;
                        response["type"] = "recording-status";
                        response["recording"] = false;
                        pApp->publish("recording-status", response.dump(), opCode);
                    }
                } else if (j["type"] == "command") {
                    if (selectedRecorder != nullptr) {
                        pApp->publish("recording-status", selectedRecorder->sendCommand(j).dump(), opCode);
                    }
                }
            },
            .dropped = [](auto */*ws*/, std::string_view /*message*/, uWS::OpCode /*opCode*/) {
                /* A message was dropped due to set maxBackpressure and closeOnBackpressureLimit limit */
            },
            .drain = [](auto */*ws*/) {
                /* Check ws->getBufferedAmount() here */
            },
            .ping = [](auto */*ws*/, std::string_view) {
                /* Not implemented yet */
            },
            .pong = [](auto */*ws*/, std::string_view) {
                /* Not implemented yet */
            },
            .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
                /* You may access ws->getUserData() here */
            }
    });

    // loop through ui/assets directory and serve all files at assets/*
    pApp->get("/assets/*", [](auto *res, auto *req) {
        // return the html content of /index.html
        serveFile(res, req);
        // get content of /index.html
        std::string content = "";
        std::ifstream file("ui/assets/" + std::string(req->getUrl().substr(8)));
        if (file.is_open()) {
            std::string line;
            while (getline(file, line)) {
                content += line;
            }
            file.close();
        }
        res->end(content);
    });


    pApp->listen(3000, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 3000 << std::endl;
        }
    });


    struct us_loop_t *loop = (struct us_loop_t *) pLoop;
    struct us_timer_t *delayTimer = us_create_timer(loop, 0, 0);



    us_timer_set(delayTimer, [](struct us_timer_t *timer) {
        publishDeviceInformation();
    }, 1000, 1000);


    pApp->run();
}