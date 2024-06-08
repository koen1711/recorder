#ifndef AUDIORECORDER_AUDIORECORDER_H
#define AUDIORECORDER_AUDIORECORDER_H


#include <string>
#include "../baseRecorder.h"
#include <miniaudio.h>

struct Channel {
    std::string name;
    int index = -1;
    int8_t stereoLink = -1;
    bool isLeft = false;
};

typedef struct
{
    std::vector<ma_encoder> encoders;
} EncoderArray;

class AudioRecorder: public BaseRecorder {
private:
    std::vector<Channel> channels;
    ma_result result;
    ma_encoder_config encoderConfig{};
    EncoderArray encoders{};
    ma_device_config deviceConfig{};
    ma_device device{};
public:
    explicit AudioRecorder(uWS::App *app);

    std::string getName() override;

    std::string filePrefix ="recording";

    int init() override;
    int deinit() override;
    int startRecording() override;
    int stopRecording() override;
    nlohmann::json queryInformation() override;

    nlohmann::json queryConfiguration() override;
    int configure(nlohmann::json config) override;

    nlohmann::json sendCommand(nlohmann::json command) override;

};


#endif //AUDIORECORDER_AUDIORECORDER_H
