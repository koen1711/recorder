#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "audioRecorder.h"
#include <cmath> // For log10 and sqrt
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <pulse/pulseaudio.h>

// Other necessary global variables and structs
BaseRecorder* recorder = nullptr;
uWS::SSLApp* pGlobalApp = nullptr;
uWS::Loop* pGlobalLoop = nullptr;
ma_encoder callbackEncoder{};

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

uWS::HttpResponse<true>* listener = nullptr;
Channel listeningChannel = {};

int channelCount = 2;
ma_format outputFormat = ma_format_f32;
std::vector<float> decibel(channelCount, 0.0f);
RecordingInfo* recordingInfo = new RecordingInfo{};

// make a timestamp since the last message was sent
std::chrono::time_point<std::chrono::system_clock> lastMessageTime = std::chrono::system_clock::now();

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

    if (listener != nullptr) {
        ma_encoder_write_pcm_frames(&callbackEncoder, pChannelData[listeningChannel.index], frameCount, NULL);
    }


    for (int ch = 0; ch < channelCount; ++ch) {
        if (recordingInfo->isRecording) {
            ma_encoder_write_pcm_frames(&pEncoders->encoders[ch], pChannelData[ch], frameCount, NULL);
        }
        free(pChannelData[ch]);

        // Calculate RMS and convert to decibels
        decibel[ch] = 10 * log10(sqrt(decibel[ch] / frameCount));
    }

    if (lastMessageTime + std::chrono::milliseconds(500) < std::chrono::system_clock::now()) {
        lastMessageTime = std::chrono::system_clock::now();
        if (recorder == nullptr) return;
        nlohmann::json j = recorder->queryInformation();
        pGlobalLoop->defer([j = std::move(j.dump())]() {
            pGlobalApp->publish("recorder-info", j, uWS::OpCode::TEXT);
        });
    }
    (void)pOutput;
}



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
    deviceConfig.periodSizeInMilliseconds = 1;
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
        // get the current time and date in this format: YYYY-MM-DD-HH-MM-SS
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        std::string fileDir = recordingPathPrefix +
                recordingDirectory + "-" +
                std::to_string(tm.tm_year + 1900) + "-" +
                std::to_string(tm.tm_mon + 1) +
                "-" + std::to_string(tm.tm_mday) +
                "-" + std::to_string(tm.tm_hour) +
                ":" + std::to_string(tm.tm_min) +
                ":" + std::to_string(tm.tm_sec) + "/";
        // make the directory
        std::filesystem::create_directories(fileDir);
        std::string fileName = fileDir + filePrefix + std::to_string(i) + ".wav";
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
                        "recordingPathPrefix": {
                            "type": "string",
                            "title": "The prefix of the path to save the audio recordings to",
                            "value": ""
                        },
                        "recordingDirectory": {
                            "type": "string",
                            "title": "The directory to save the audio recordings to",
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
    j["section"]["questions"]["recording"]["questions"]["recordingDirectory"]["value"]  = recordingDirectory;
    j["section"]["questions"]["recording"]["questions"]["recordingPathPrefix"]["value"] = recordingPathPrefix;
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
                                  {"channelName-" + std::to_string(i), {
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
    } else if (config.contains("recordingPathPrefix")) {
        recordingPathPrefix = config["recordingPathPrefix"];
    } else if (config.contains("recordingDirectory")) {
        recordingDirectory = config["recordingDirectory"];
    } else if (config.contains("channelCount")) {
        int tempChannelCount = std::stoi(config["channelCount"].get<std::string>());

        if (!pa_channels_valid(tempChannelCount)) {
            return -1;
        }
        channels.resize(channelCount);
        // fill all the new channels with default values
        for (int i = channelCount; i < tempChannelCount; ++i) {
            channels.push_back(Channel{"C" + std::to_string(i), i});
        }
        channelCount = tempChannelCount;
        decibel.resize(channelCount, 0.0f);

        this->deinit();
        this->init();
    } else {
        for (int i = 0; i < channelCount; ++i) {
            std::string channelKey = "channelName-" + std::to_string(i);
            if (config.contains(channelKey)) {
                channels[i].name = config[channelKey];

                nlohmann::json j;
                j["type"] = "audio-channels";
                for (int i = 0; i < channelCount; ++i) {
                    j["channels"][i]["name"] = channels[i].name;
                    j["channels"][i]["index"] = channels[i].index;
                    j["channels"][i]["stereoLink"] = channels[i].stereoLink;
                    j["channels"][i]["isLeft"] = channels[i].isLeft;
                }
                pGlobalLoop->defer([j = std::move(j)]() {
                    pGlobalApp->publish("recorder-info", j.dump(), uWS::OpCode::TEXT);
                });
            }
        }


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

#pragma pack(push, 1) // exact fit - align at byte-boundary, no padding

typedef struct wavHeader
{
    uint8_t chunckID[4];
    uint32_t chunckSize;
    uint8_t format[4];
    uint8_t subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint8_t subchunk2ID[4];
    uint32_t subchunk2Size;
} wavHeader;

#pragma pack(pop)

void onWrite(ma_encoder* pEncoder, const void* pBufferIn, size_t bytesToWrite, size_t* pBytesWritten) {
    auto* res = (uWS::HttpResponse<true>*)pEncoder->pUserData;
    std::string buffer(reinterpret_cast<const char*>(pBufferIn), bytesToWrite);
    pGlobalLoop->defer([res, buffer = std::move(buffer)]() {
        res->write(buffer);
    });
    *pBytesWritten = bytesToWrite;
}

void onSeek(ma_encoder* pEncoder, ma_int64 offset, ma_seek_origin origin) {
    // Do nothing
}

bool AudioRecorder::registerListener(uWS::HttpResponse<true> *res, uWS::HttpRequest *req) {
    if (listener != nullptr) {
        res->writeStatus("400 Bad Request");
        res->end("Listener already exists");
        return false;
    }
    wavHeader header = {
            {'R', 'I', 'F', 'F'},
            0,
            {'W', 'A', 'V', 'E'},
            {'f', 'm', 't', ' '},
            16,
            0x0003,
            1,
            48000,
            static_cast<uint32_t>(48000 * (ma_get_bytes_per_sample(outputFormat) * 8) * 1 / 8),
            static_cast<uint16_t>(ma_get_bytes_per_sample(outputFormat) * 8),
            static_cast<uint16_t>(ma_get_bytes_per_sample(outputFormat) * 8),
            {'d', 'a', 't', 'a'},
            0
    };
    std::string headerStr(reinterpret_cast<char*>(&header), sizeof(header));
    pGlobalLoop->defer([res, headerStr = std::move(headerStr)]() {
        res->write(headerStr);
    });
    // wait for 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    listener = res;
    // from the request, get the channel index
    std::string_view channelIndex = req->getQuery("channel");
    if (channelIndex.empty()) {
        std::cout << "No channel index provided" << std::endl;
        return false;
    }
    int index = std::stoi(std::string(channelIndex));
    std::cout << "Registering listener for channel " << index << std::endl;
    listeningChannel = channels[index];

    callbackEncoder = {};
    // give the res to the encoder
    encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, outputFormat, 1, 48000);
    void* pUserData = res;
    result = ma_encoder_init(reinterpret_cast<ma_encoder_write_proc>(onWrite),
                             reinterpret_cast<ma_encoder_seek_proc>(onSeek), pUserData, &encoderConfig, &callbackEncoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize encoder.\n");
        return false;
    }


    res->onAborted([](){
        // remove the listener from the list at the index
        listener = nullptr;
        listeningChannel = {};
    });
    return true;
}
#include <pwd.h>


AudioRecorder::AudioRecorder(uWS::SSLApp *app, uWS::Loop *loop) : BaseRecorder(app, loop) {
    static auto whoAmI = [](){ struct passwd *tmp = getpwuid (geteuid ());
        return tmp ? tmp->pw_name : "RECORDER";
    };
    std::string user = whoAmI();
    recordingPathPrefix = "/home/" + user + "/recordings/";

    recorder = this;
    pGlobalApp = app;
    pGlobalLoop = loop;
}
