#ifndef AUDIORECORDER_APP_H
#define AUDIORECORDER_APP_H

#include <uWebSockets/App.h>

uWS::SSLApp * pApp = new uWS::SSLApp({
        .key_file_name = "/misc/key.pem",
        .cert_file_name = "/misc/cert.pem",
});
uWS::Loop* pLoop = uWS::Loop::get();
struct Config {
    bool allowUnregisteredAccess = true;
    bool allowUnregisteredRecording = true;
    bool allowUnregisteredStreaming = true;
    bool allowUnregisteredConfiguration = true;
};
Config* globalConfig = new Config();

#endif //AUDIORECORDER_APP_H
