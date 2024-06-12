#ifndef AUDIORECORDER_APP_H
#define AUDIORECORDER_APP_H

#include <uWebSockets/App.h>

uWS::App* pApp = new uWS::App();
uWS::Loop* pLoop = uWS::Loop::get();

#endif //AUDIORECORDER_APP_H
