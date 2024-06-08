#ifndef AUDIORECORDER_DASHBOARD_H
#define AUDIORECORDER_DASHBOARD_H


class Dashboard {
private:
public:
    Dashboard();

    void startServer();
    void stopServer();

    void sendQueryPacket();
    void registerWebsocketCallback(void (*callback)(void) );
};


#endif //AUDIORECORDER_DASHBOARD_H
