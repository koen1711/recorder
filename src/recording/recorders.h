#ifndef AUDIORECORDER_RECORDERS_H
#define AUDIORECORDER_RECORDERS_H

#include <vector>
#include "baseRecorder.h"
#include "audio/audioRecorder.h"
#include "../dashboard/app.h"

std::vector<BaseRecorder*> recorders = {
        (BaseRecorder*) new AudioRecorder(pApp, pLoop)
};

#endif //AUDIORECORDER_RECORDERS_H
