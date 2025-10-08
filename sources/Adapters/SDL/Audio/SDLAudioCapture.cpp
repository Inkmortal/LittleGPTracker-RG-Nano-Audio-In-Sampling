#include "SDLAudioCapture.h"
#include "System/Console/Trace.h"
#include <cstring>

SDLAudioCapture::SDLAudioCapture()
    : deviceID_(0),
      playbackDeviceID_(0),
      buffer_(NULL),
      bufferSize_(0),
      currentPos_(0),
      isRecording_(false),
      isInitialized_(false),
      monitoringEnabled_(true) {
}

SDLAudioCapture::~SDLAudioCapture() {
    Close();
}

void SDLAudioCapture::captureCallback(void *userdata, Uint8 *stream, int len) {
    SDLAudioCapture *capture = (SDLAudioCapture*)userdata;

    // Passthrough monitoring (hear what you're recording)
    if (capture->monitoringEnabled_ && capture->playbackDeviceID_) {
        SDL_QueueAudio(capture->playbackDeviceID_, stream, len);
    }

    if (!capture->isRecording_) {
        return;
    }

    // Calculate how many samples we're receiving
    int samplesInStream = len / sizeof(short);
    int samplesRemaining = capture->bufferSize_ - capture->currentPos_;

    if (samplesRemaining <= 0) {
        // Buffer is full, stop recording
        capture->isRecording_ = false;
        return;
    }

    // Copy as much as we can fit
    int samplesToCopy = (samplesInStream < samplesRemaining) ? samplesInStream : samplesRemaining;

    memcpy(capture->buffer_ + capture->currentPos_, stream, samplesToCopy * sizeof(short));
    capture->currentPos_ += samplesToCopy;

    // Auto-stop if buffer is full
    if (capture->currentPos_ >= capture->bufferSize_) {
        capture->isRecording_ = false;
    }
}

bool SDLAudioCapture::Init() {
    if (isInitialized_) {
        return true;
    }

    SDL_AudioSpec desired, obtained;

    desired.freq = CAPTURE_SAMPLE_RATE;
    desired.format = AUDIO_S16SYS;
    desired.channels = CAPTURE_CHANNELS;
    desired.samples = 1024;
    desired.callback = captureCallback;
    desired.userdata = this;

    // Open audio capture device (1 = capture, 0 = playback)
    deviceID_ = SDL_OpenAudioDevice(NULL, 1, &desired, &obtained, 0);

    if (deviceID_ == 0) {
        Trace::Error("Failed to open audio capture device: %s\n", SDL_GetError());
        return false;
    }

    Trace::Log("AUDIO", "Audio capture device opened: %d Hz, %d channels",
               obtained.freq, obtained.channels);

    // Open playback device for monitoring (passthrough)
    SDL_AudioSpec playbackSpec;
    playbackSpec.freq = CAPTURE_SAMPLE_RATE;
    playbackSpec.format = AUDIO_S16SYS;
    playbackSpec.channels = CAPTURE_CHANNELS;
    playbackSpec.samples = 512;  // Smaller buffer for lower latency
    playbackSpec.callback = NULL; // Use queue for playback
    playbackSpec.userdata = NULL;

    playbackDeviceID_ = SDL_OpenAudioDevice(NULL, 0, &playbackSpec, NULL, 0);

    if (playbackDeviceID_ == 0) {
        Trace::Log("AUDIO", "Warning: Could not open playback device for monitoring: %s", SDL_GetError());
        Trace::Log("AUDIO", "Recording will work but without passthrough monitoring");
    } else {
        Trace::Log("AUDIO", "Playback device opened for monitoring (latency: ~%dms)",
                   (512 * 1000) / CAPTURE_SAMPLE_RATE);
    }

    isInitialized_ = true;
    return true;
}

bool SDLAudioCapture::StartRecording(int maxSeconds, bool enableMonitoring) {
    if (!isInitialized_) {
        if (!Init()) {
            return false;
        }
    }

    if (isRecording_) {
        StopRecording();
    }

    // Clamp max seconds
    if (maxSeconds > MAX_RECORDING_SECONDS) {
        maxSeconds = MAX_RECORDING_SECONDS;
    }

    // Allocate buffer (stereo samples)
    bufferSize_ = CAPTURE_SAMPLE_RATE * CAPTURE_CHANNELS * maxSeconds;

    if (buffer_) {
        delete[] buffer_;
    }

    buffer_ = new short[bufferSize_];
    memset(buffer_, 0, bufferSize_ * sizeof(short));

    currentPos_ = 0;
    isRecording_ = true;
    monitoringEnabled_ = enableMonitoring;

    // Start the capture device
    SDL_PauseAudioDevice(deviceID_, 0);

    // Start the playback device for monitoring
    if (monitoringEnabled_ && playbackDeviceID_) {
        SDL_ClearQueuedAudio(playbackDeviceID_); // Clear any old audio
        SDL_PauseAudioDevice(playbackDeviceID_, 0);
    }

    Trace::Log("AUDIO", "Started recording, max duration: %d seconds, monitoring: %s",
               maxSeconds, monitoringEnabled_ ? "ON" : "OFF");

    return true;
}

void SDLAudioCapture::StopRecording() {
    if (!isRecording_) {
        return;
    }

    isRecording_ = false;

    if (deviceID_) {
        SDL_PauseAudioDevice(deviceID_, 1);
    }

    // Stop monitoring playback
    if (playbackDeviceID_) {
        SDL_PauseAudioDevice(playbackDeviceID_, 1);
        SDL_ClearQueuedAudio(playbackDeviceID_); // Clear any remaining audio
    }

    Trace::Log("AUDIO", "Stopped recording, captured %d samples (%.2f seconds)",
               currentPos_, GetRecordingTime());
}

short* SDLAudioCapture::GetBuffer(int &sampleCount) {
    sampleCount = currentPos_;
    return buffer_;
}

float SDLAudioCapture::GetRecordingTime() const {
    if (CAPTURE_SAMPLE_RATE == 0 || CAPTURE_CHANNELS == 0) {
        return 0.0f;
    }
    return (float)currentPos_ / (float)(CAPTURE_SAMPLE_RATE * CAPTURE_CHANNELS);
}

void SDLAudioCapture::Close() {
    if (isRecording_) {
        StopRecording();
    }

    if (deviceID_) {
        SDL_CloseAudioDevice(deviceID_);
        deviceID_ = 0;
    }

    if (playbackDeviceID_) {
        SDL_CloseAudioDevice(playbackDeviceID_);
        playbackDeviceID_ = 0;
    }

    if (buffer_) {
        delete[] buffer_;
        buffer_ = NULL;
    }

    bufferSize_ = 0;
    currentPos_ = 0;
    isInitialized_ = false;
}
