#ifndef AUDIORECORDER_BASERECORDER_H
#define AUDIORECORDER_BASERECORDER_H

#include <string>
#include <uWebSockets/App.h>
#include "nlohmann/json.hpp"

struct RecordingInfo {
    bool isRecording = false;
};

class BaseRecorder {
private:
public:
    bool isRecording = false;

    virtual std::string getName() = 0;

    explicit BaseRecorder(uWS::App* app) {};
    virtual int init() = 0;
    virtual int deinit() = 0;
    virtual int startRecording() = 0;
    virtual int stopRecording() = 0;
    virtual nlohmann::json queryInformation() = 0;

    virtual nlohmann::json queryConfiguration() = 0;
    virtual int configure(nlohmann::json config) = 0;

    virtual nlohmann::json sendCommand(nlohmann::json command) = 0;


};


#endif //AUDIORECORDER_BASERECORDER_H
