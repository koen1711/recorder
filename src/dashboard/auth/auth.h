#ifndef AUDIORECORDER_AUTH_H
#define AUDIORECORDER_AUTH_H

#include <uWebSockets/HttpResponse.h>

bool checkAuth(uWS::HttpResponse<true> *res, uWS::HttpRequest *req, bool allowUnregisteredAccess, const std::string& redirect_uri, bool return403 = false);

#endif //AUDIORECORDER_AUTH_H
