#include "Adapters/ALSA/Audio/ALSAAudioCapture.h"
#include <string.h>

ALSAAudioCapture::ALSAAudioCapture() :
    captureHandle_(NULL),
    playbackHandle_(NULL),
    buffer_(NULL),
    bufferSize_(0),
    currentPos_(0),
    maxDuration_(5),
    isRecording_(false),
    isInitialized_(false),
    monitoringEnabled_(false),
    sampleRate_(44100),
    channels_(2),
    deviceName_("simulated"),
    waveformPos_(0),
    peakLevel_(0.0f),
    lastError_("RG Nano simulator has no live audio input") {
    memset(waveformBuffer_, 0, sizeof(waveformBuffer_));
}

ALSAAudioCapture::~ALSAAudioCapture() {
    Close();
}

std::vector<ALSADeviceInfo> ALSAAudioCapture::EnumerateCaptureDevices() {
    std::vector<ALSADeviceInfo> devices;
    ALSADeviceInfo dev;
    dev.name = "simulated";
    dev.description = "Simulator: no live input";
    devices.push_back(dev);
    return devices;
}

void ALSAAudioCapture::SetCaptureDevice(const std::string& deviceName) {
    deviceName_ = deviceName;
}

bool ALSAAudioCapture::Init() {
    isInitialized_ = true;
    lastError_ = "RG Nano simulator has no live audio input";
    return false;
}

bool ALSAAudioCapture::StartRecording(int maxSeconds, bool enableMonitoring) {
    maxDuration_ = maxSeconds;
    monitoringEnabled_ = enableMonitoring;
    isRecording_ = false;
    lastError_ = "RG Nano simulator has no live audio input";
    return false;
}

void ALSAAudioCapture::StopRecording() {
    isRecording_ = false;
}

short* ALSAAudioCapture::GetBuffer(int &sampleCount) {
    sampleCount = 0;
    return buffer_;
}

float ALSAAudioCapture::GetRecordingTime() const {
    return 0.0f;
}

void ALSAAudioCapture::Update() {
}

void ALSAAudioCapture::Close() {
    if (buffer_) {
        delete[] buffer_;
        buffer_ = NULL;
    }
    isRecording_ = false;
    isInitialized_ = false;
}
