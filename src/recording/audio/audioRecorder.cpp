#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "audioRecorder.h"
#include <cmath> // For log10 and sqrt
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <pulse/pulseaudio.h>

BaseRecorder* recorder = nullptr;
uWS::App* pGlobalApp = nullptr;

std::vector<ma_format> supportedFormats = {
        ma_format_u8,
        ma_format_s16,
        ma_format_s24,
        ma_format_s32,
        ma_format_f32
};

std::vector<ma_channel> supportedChannels = {
        MA_CHANNEL_FRONT_LEFT,
        MA_CHANNEL_FRONT_RIGHT,
        MA_CHANNEL_FRONT_CENTER,
        MA_CHANNEL_FRONT_LEFT_CENTER,
        MA_CHANNEL_FRONT_RIGHT_CENTER,
        MA_CHANNEL_BACK_CENTER,
        MA_CHANNEL_SIDE_LEFT,
        MA_CHANNEL_SIDE_RIGHT,
        MA_CHANNEL_AUX_0,
        MA_CHANNEL_AUX_1,
        MA_CHANNEL_AUX_2,
        MA_CHANNEL_AUX_3,
        MA_CHANNEL_AUX_4,
        MA_CHANNEL_AUX_5,
        MA_CHANNEL_AUX_6,
        MA_CHANNEL_AUX_7,
        MA_CHANNEL_AUX_8,
        MA_CHANNEL_AUX_9,
        MA_CHANNEL_AUX_10,
        MA_CHANNEL_AUX_11,
        MA_CHANNEL_AUX_12,
        MA_CHANNEL_AUX_13,
        MA_CHANNEL_AUX_14,
        MA_CHANNEL_AUX_15,
        MA_CHANNEL_AUX_16,
        MA_CHANNEL_AUX_17,
        MA_CHANNEL_AUX_18,
        MA_CHANNEL_AUX_19,
        MA_CHANNEL_AUX_20,
        MA_CHANNEL_AUX_21,
        MA_CHANNEL_AUX_22,
        MA_CHANNEL_AUX_23,
        MA_CHANNEL_AUX_24,
        MA_CHANNEL_AUX_25
};

int channelCount = 32;
ma_format outputFormat = ma_format_f32;
std::vector<float> decibel(channelCount, 0.0f);
RecordingInfo* recordingInfo = new RecordingInfo{};

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    EncoderArray* pEncoders = (EncoderArray*)pDevice->pUserData;
    MA_ASSERT(pEncoders != NULL);



    std::vector<void*> pChannelData(channelCount);

    for (int i = 0; i < channelCount; ++i) {
        pChannelData[i] = malloc(frameCount * ma_get_bytes_per_sample(outputFormat));
    }

    std::fill(decibel.begin(), decibel.end(), 0.0f); // Reset decibel array

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        for (int ch = 0; ch < channelCount; ++ch) {
            const uint8_t* src;
            int32_t sample;
            switch (outputFormat) {
                case ma_format_f32:
                    ((float*)pChannelData[ch])[i] = ((const float*)pInput)[i * channelCount + ch];
                    decibel[ch] += ((const float*)pInput)[i * channelCount + ch] * ((const float*)pInput)[i * channelCount + ch];
                    break;
                case ma_format_u8:
                    ((uint8_t*)pChannelData[ch])[i] = ((const uint8_t*)pInput)[i * channelCount + ch];
                    break;
                case ma_format_s16:
                    ((int16_t*)pChannelData[ch])[i] = ((const int16_t*)pInput)[i * channelCount + ch];
                    break;
                case ma_format_s24:
                    src = (const uint8_t*)pInput + (i * channelCount + ch) * 3;
                    sample = (src[0] << 8) | (src[1] << 16) | (src[2] << 24);
                    sample >>= 8; // Shift down to convert to 24-bit signed value

                    // Store the 24-bit data into the channel data buffer
                    memcpy((uint8_t*)pChannelData[ch] + i * 3, src, 3);

                    // Calculate the decibel level
                    decibel[ch] += sample * sample;
                    break;
                case ma_format_s32:
                    ((int32_t*)pChannelData[ch])[i] = ((const int32_t*)pInput)[i * channelCount + ch];
                    break;
                default:
                    break;
            }
        }
    }

    for (int ch = 0; ch < channelCount; ++ch) {
        if (recordingInfo->isRecording) {
            ma_encoder_write_pcm_frames(&pEncoders->encoders[ch], pChannelData[ch], frameCount, NULL);
        }
        free(pChannelData[ch]);

        // Calculate RMS and convert to decibels
        decibel[ch] = 10 * log10(sqrt(decibel[ch] / frameCount));
    }

    (void)pOutput;
    if (recorder == nullptr) return;
    nlohmann::json j = recorder->queryInformation();
    pGlobalApp->publish("recorder-info", j.dump(), uWS::OpCode::TEXT);}

int AudioRecorder::init() {
    if (channels.empty()) {
        for (int i = 0; i < channelCount; ++i) {
            channels.push_back(Channel{"C" + std::to_string(i), i});
        }
    }

    //encoders.encoders = (ma_encoder*)malloc(channelCount * sizeof(ma_encoder));
    encoders.encoders.resize(channelCount);
    for (int i = 0; i < channelCount; ++i) {
        encoders.encoders[i] = {};
    }

    encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, outputFormat, 1, 48000); // Ensure the sample rate is 96000

    ma_channel* channels = (ma_channel*)malloc(channelCount * sizeof(ma_channel));
    for (int i = 0; i < channelCount; ++i) {
        channels[i] = supportedChannels[i];
    }

    deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format = outputFormat;
    deviceConfig.capture.channels = channelCount;
    deviceConfig.sampleRate = encoderConfig.sampleRate; // Ensure the sample rate matches the encoder
    deviceConfig.dataCallback = data_callback;
    deviceConfig.capture.pChannelMap = channels;
    deviceConfig.pUserData = &encoders;

    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize capture device.\n");
        return -1;
    }

    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&device);
        printf("Failed to start device.\n");
        return -2;
    }

    return 0;
}

int AudioRecorder::startRecording() {
    std::vector<int> initializedEncoders = {};
    for (int i = 0; i < channelCount; ++i) {
        std::cout << "Channel: " << i << std::endl;
        std::string fileName = filePrefix + std::to_string(i) + ".wav";
        if (ma_encoder_init_file(fileName.c_str(), &encoderConfig, &encoders.encoders[i]) != MA_SUCCESS) {
            printf("Failed to initialize output file for channel %d.\n", i);
            for (int j : initializedEncoders) {
                ma_encoder_uninit(&encoders.encoders[j]);
            }
            return -1;
        } else {
            initializedEncoders.push_back(i);
            printf("Initialized encoder for channel %d, file: %s\n", i, fileName.c_str());  // Debug statement
        }
    }
    recordingInfo->isRecording = true;
    return 0;
}

int AudioRecorder::stopRecording() {
    recordingInfo->isRecording = false;
    for (int i = 0; i < channelCount; ++i) {
        ma_encoder_uninit(&encoders.encoders[i]);
    }
    return 0;
}

int AudioRecorder::deinit() {
    if (recordingInfo->isRecording) {
        stopRecording();
    } else {
        ma_device_uninit(&device);
    }
    return 0;
}

nlohmann::json AudioRecorder::queryInformation() {
    nlohmann::json j;
    j["type"] = "audio-info";
    j["volumeLevel"] = decibel;
    j["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();

    return j;
}

std::string AudioRecorder::getName() {
    return "Audio Recorder";
}

nlohmann::json AudioRecorder::queryConfiguration() {
    nlohmann::json j = R"({
        "type": "config",
        "section": {
            "title": "Audio Recorder Settings",
            "type": "tabs",
            "questions": {
                "recording": {
                    "title": "Recording Settings",
                    "type": "body",
                    "questions": {
                        "audioFormat": {
                            "type": "select",
                            "title": "The format to save the audio recordings in",
                            "value": ""
                        },
                        "filePrefix": {
                            "type": "string",
                            "title": "The prefix of the file name to save the audio recordings to",
                            "value": ""
                        },
                        "channelCount": {
                            "type": "number",
                            "title": "The number of channels to record",
                            "value": 0
                        }
                    }
                },
                "channels": {
                    "title": "Channel Settings",
                    "type": "section",
                    "questions": {}
                }
            }
        }
    })"_json;

    // Populate recording settings
    j["section"]["questions"]["recording"]["questions"]["audioFormat"]["value"] = ma_get_format_name(outputFormat);
    j["section"]["questions"]["recording"]["questions"]["filePrefix"]["value"] = filePrefix;
    j["section"]["questions"]["recording"]["questions"]["channelCount"]["value"] = channelCount;

    for (auto format : supportedFormats) {
        j["section"]["questions"]["recording"]["questions"]["audioFormat"]["options"][format] = ma_get_format_name(format);
    }

    // Populate channel settings
    for (int i = 0; i < channelCount; ++i) {
        Channel channel = channels[i];
        std::string channelKey = "channel" + std::to_string(i);
        j["section"]["questions"]["channels"]["questions"][channelKey] = {
                {"title", channel.index},
                {"type", "body"},
                {"questions", {
                                  {"channelName", {
                                                          {"type", "string"},
                                                          {"title", "Name channel "},
                                                          {"value", channel.name}
                                                  }}
                          }}
        };
    }

    return j;
}

int AudioRecorder::configure(nlohmann::json config) {
    if (config.contains("audioFormat")) {
        for (auto format : supportedFormats) {
            if (ma_get_format_name(format) == config["audioFormat"]) {
                outputFormat = format;
                this->deinit();
                this->init();
                break;
            }
        }
    } else if (config.contains("filePrefix")) {
        filePrefix = config["filePrefix"];
    } else if (config.contains("channelCount")) {
        int tempChannelCount = std::stoi(config["channelCount"].get<std::string>());

        if (!pa_channels_valid(tempChannelCount)) {
            return -1;
        }
        channelCount = tempChannelCount;
        this->deinit();
        this->init();
    }
    return 0;
}

nlohmann::json AudioRecorder::sendCommand(nlohmann::json command) {
    std::string cmd = command["command"];
    if (cmd == "query-channels") {
        nlohmann::json j;
        j["type"] = "audio-channels";
        for (int i = 0; i < channelCount; ++i) {
            j["channels"][i]["name"] = channels[i].name;
            j["channels"][i]["index"] = channels[i].index;
            j["channels"][i]["stereoLink"] = channels[i].stereoLink;
            j["channels"][i]["isLeft"] = channels[i].isLeft;
        }
        return j;
    }
}

AudioRecorder::AudioRecorder(uWS::App *app) : BaseRecorder(app) {
    recorder = this;
    pGlobalApp = app;
}
