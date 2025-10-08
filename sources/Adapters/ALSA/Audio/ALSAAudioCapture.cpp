#include "ALSAAudioCapture.h"
#include "System/Console/Trace.h"
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
    deviceName_("default"),
    waveformPos_(0),
    peakLevel_(0.0f),
    lastError_("") {
    memset(waveformBuffer_, 0, sizeof(waveformBuffer_));
}

ALSAAudioCapture::~ALSAAudioCapture() {
    Close();
}

std::vector<ALSADeviceInfo> ALSAAudioCapture::EnumerateCaptureDevices() {
    std::vector<ALSADeviceInfo> devices;

    // Add default device
    ALSADeviceInfo defaultDev;
    defaultDev.name = "default";
    defaultDev.description = "Default ALSA Device";
    devices.push_back(defaultDev);

    // Enumerate hardware cards
    int cardNum = -1;
    while (snd_card_next(&cardNum) >= 0 && cardNum >= 0) {
        char cardName[32];
        snprintf(cardName, sizeof(cardName), "hw:%d", cardNum);

        snd_ctl_t *cardHandle;
        if (snd_ctl_open(&cardHandle, cardName, 0) < 0) {
            continue;
        }

        snd_ctl_card_info_t *cardInfo;
        snd_ctl_card_info_alloca(&cardInfo);

        if (snd_ctl_card_info(cardHandle, cardInfo) < 0) {
            snd_ctl_close(cardHandle);
            continue;
        }

        const char *cardLongName = snd_ctl_card_info_get_name(cardInfo);

        // Check each device on this card
        int devNum = -1;
        while (snd_ctl_pcm_next_device(cardHandle, &devNum) >= 0 && devNum >= 0) {
            // Check if it's a capture device
            snd_pcm_info_t *pcmInfo;
            snd_pcm_info_alloca(&pcmInfo);
            snd_pcm_info_set_device(pcmInfo, devNum);
            snd_pcm_info_set_subdevice(pcmInfo, 0);
            snd_pcm_info_set_stream(pcmInfo, SND_PCM_STREAM_CAPTURE);

            if (snd_ctl_pcm_info(cardHandle, pcmInfo) >= 0) {
                ALSADeviceInfo dev;
                char devName[64];
                snprintf(devName, sizeof(devName), "hw:%d,%d", cardNum, devNum);
                dev.name = devName;

                char desc[128];
                snprintf(desc, sizeof(desc), "%s (Card %d, Device %d)",
                         cardLongName, cardNum, devNum);
                dev.description = desc;

                devices.push_back(dev);
            }
        }

        snd_ctl_close(cardHandle);
    }

    return devices;
}

void ALSAAudioCapture::SetCaptureDevice(const std::string& deviceName) {
    if (isInitialized_ || isRecording_) {
        Trace::Error("Cannot change device while initialized or recording");
        return;
    }
    deviceName_ = deviceName;
    Trace::Log("ALSA", "Capture device set to: %s", deviceName_.c_str());
}

bool ALSAAudioCapture::Init() {
    if (isInitialized_) {
        lastError_ = "";
        return true;
    }

    int err;
    lastError_ = "";

    // Open PCM device for capture using selected device
    err = snd_pcm_open(&captureHandle_, deviceName_.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        char errBuf[128];
        snprintf(errBuf, sizeof(errBuf), "Cannot open %s: %s",
                 deviceName_.c_str(), snd_strerror(err));
        lastError_ = errBuf;
        Trace::Error("Cannot open audio capture device '%s' (%s)",
                     deviceName_.c_str(), snd_strerror(err));
        return false;
    }

    Trace::Log("ALSA", "Opened capture device: %s", deviceName_.c_str());

    // Set hardware parameters for capture
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(captureHandle_, hw_params);
    snd_pcm_hw_params_set_access(captureHandle_, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(captureHandle_, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(captureHandle_, hw_params, channels_);
    snd_pcm_hw_params_set_rate_near(captureHandle_, hw_params, &sampleRate_, 0);

    err = snd_pcm_hw_params(captureHandle_, hw_params);
    if (err < 0) {
        char errBuf[128];
        snprintf(errBuf, sizeof(errBuf), "Cannot set params: %s", snd_strerror(err));
        lastError_ = errBuf;
        Trace::Error("Cannot set capture hardware parameters (%s)", snd_strerror(err));
        snd_pcm_close(captureHandle_);
        captureHandle_ = NULL;
        return false;
    }

    // Prepare the capture device
    err = snd_pcm_prepare(captureHandle_);
    if (err < 0) {
        char errBuf[128];
        snprintf(errBuf, sizeof(errBuf), "Cannot prepare: %s", snd_strerror(err));
        lastError_ = errBuf;
        Trace::Error("Cannot prepare audio capture interface (%s)", snd_strerror(err));
        snd_pcm_close(captureHandle_);
        captureHandle_ = NULL;
        return false;
    }

    isInitialized_ = true;
    Trace::Log("ALSA", "Audio capture initialized: %d Hz, %d channels", sampleRate_, channels_);
    return true;
}

bool ALSAAudioCapture::StartRecording(int maxSeconds, bool enableMonitoring) {
    if (!isInitialized_ && !Init()) {
        return false;
    }

    if (isRecording_) {
        return true;
    }

    maxDuration_ = maxSeconds;
    if (maxDuration_ > MAX_RECORDING_SECONDS) {
        maxDuration_ = MAX_RECORDING_SECONDS;
    }

    monitoringEnabled_ = enableMonitoring;

    // Allocate buffer for maximum duration
    bufferSize_ = sampleRate_ * channels_ * maxDuration_;
    if (buffer_) {
        delete[] buffer_;
    }
    buffer_ = new short[bufferSize_];
    memset(buffer_, 0, bufferSize_ * sizeof(short));
    currentPos_ = 0;

    // If monitoring enabled, open playback device using dmix for device sharing
    if (monitoringEnabled_) {
        // Use dmix to allow sharing the device with SDL audio
        std::string monitorDevice = deviceName_;
        if (deviceName_.find("hw:") == 0) {
            // Convert hw:X,Y to dmix:X,Y for software mixing
            monitorDevice = "dmix:" + deviceName_.substr(3);
        }

        int err = snd_pcm_open(&playbackHandle_, monitorDevice.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
        if (err < 0) {
            Trace::Error("Cannot open playback device '%s' for monitoring (%s)",
                         monitorDevice.c_str(), snd_strerror(err));
            monitoringEnabled_ = false;
        } else {
            // Set playback parameters
            snd_pcm_hw_params_t *hw_params;
            snd_pcm_hw_params_alloca(&hw_params);
            snd_pcm_hw_params_any(playbackHandle_, hw_params);
            snd_pcm_hw_params_set_access(playbackHandle_, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
            snd_pcm_hw_params_set_format(playbackHandle_, hw_params, SND_PCM_FORMAT_S16_LE);
            snd_pcm_hw_params_set_channels(playbackHandle_, hw_params, channels_);
            snd_pcm_hw_params_set_rate_near(playbackHandle_, hw_params, &sampleRate_, 0);

            err = snd_pcm_hw_params(playbackHandle_, hw_params);
            if (err < 0) {
                Trace::Error("Cannot set playback parameters (%s)", snd_strerror(err));
                snd_pcm_close(playbackHandle_);
                playbackHandle_ = NULL;
                monitoringEnabled_ = false;
            } else {
                snd_pcm_prepare(playbackHandle_);
            }
        }
    }

    isRecording_ = true;
    Trace::Log("ALSA", "Started recording (max %d seconds, monitoring %s)",
               maxDuration_, monitoringEnabled_ ? "ON" : "OFF");
    return true;
}

void ALSAAudioCapture::Update() {
    if (!isRecording_) {
        return;
    }

    // Read frames from capture device
    const int framesPerUpdate = 512;
    short tempBuffer[framesPerUpdate * channels_];

    int frames = snd_pcm_readi(captureHandle_, tempBuffer, framesPerUpdate);

    if (frames < 0) {
        frames = snd_pcm_recover(captureHandle_, frames, 0);
    }

    if (frames < 0) {
        Trace::Error("Read from audio interface failed (%s)", snd_strerror(frames));
        StopRecording();
        return;
    }

    // Copy to buffer
    int samplesToWrite = frames * channels_;
    int samplesAvailable = bufferSize_ - currentPos_;

    if (samplesToWrite > samplesAvailable) {
        samplesToWrite = samplesAvailable;
    }

    if (samplesToWrite > 0) {
        memcpy(buffer_ + currentPos_, tempBuffer, samplesToWrite * sizeof(short));
        currentPos_ += samplesToWrite;

        // Update waveform buffer and peak level for visualization
        peakLevel_ = 0.0f;
        for (int i = 0; i < samplesToWrite; i += channels_) {
            // Take left channel for waveform
            short sample = tempBuffer[i];
            waveformBuffer_[waveformPos_] = sample;
            waveformPos_ = (waveformPos_ + 1) % WAVEFORM_BUFFER_SIZE;

            // Calculate peak level (0.0 to 1.0)
            float level = (float)abs(sample) / 32768.0f;
            if (level > peakLevel_) {
                peakLevel_ = level;
            }
        }

        // Pass through to playback if monitoring enabled
        if (monitoringEnabled_ && playbackHandle_) {
            snd_pcm_writei(playbackHandle_, tempBuffer, frames);
        }
    }

    // Stop if buffer is full
    if (currentPos_ >= bufferSize_) {
        StopRecording();
    }
}

void ALSAAudioCapture::StopRecording() {
    if (!isRecording_) {
        return;
    }

    isRecording_ = false;

    if (playbackHandle_) {
        snd_pcm_drain(playbackHandle_);
        snd_pcm_close(playbackHandle_);
        playbackHandle_ = NULL;
    }

    Trace::Log("ALSA", "Stopped recording (%d samples captured)", currentPos_);
}

short* ALSAAudioCapture::GetBuffer(int &sampleCount) {
    sampleCount = currentPos_;
    return buffer_;
}

float ALSAAudioCapture::GetRecordingTime() const {
    if (channels_ == 0 || sampleRate_ == 0) {
        return 0.0f;
    }
    return (float)currentPos_ / (float)(channels_ * sampleRate_);
}

void ALSAAudioCapture::Close() {
    StopRecording();

    if (captureHandle_) {
        snd_pcm_close(captureHandle_);
        captureHandle_ = NULL;
    }

    if (buffer_) {
        delete[] buffer_;
        buffer_ = NULL;
    }

    bufferSize_ = 0;
    currentPos_ = 0;
    isInitialized_ = false;
}
