#ifndef _SDL_AUDIO_CAPTURE_H_
#define _SDL_AUDIO_CAPTURE_H_

#include <SDL/SDL.h>

#define MAX_RECORDING_SECONDS 60  // Increased from 30 to 60 seconds (~10MB RAM)
#define CAPTURE_SAMPLE_RATE 44100
#define CAPTURE_CHANNELS 2

class SDLAudioCapture {
public:
    SDLAudioCapture();
    ~SDLAudioCapture();

    // Initialize the capture device
    bool Init();

    // Start recording (maxSeconds = maximum recording duration, enableMonitoring = hear input while recording)
    bool StartRecording(int maxSeconds = 10, bool enableMonitoring = true);

    // Stop recording
    void StopRecording();

    // Get the recorded buffer and sample count
    // Returns pointer to buffer, sets sampleCount to number of samples recorded
    short* GetBuffer(int &sampleCount);

    // Check if currently recording
    bool IsRecording() const { return isRecording_; }

    // Get current recording position in seconds
    float GetRecordingTime() const;

    // Enable/disable passthrough monitoring
    void SetMonitoring(bool enabled) { monitoringEnabled_ = enabled; }
    bool IsMonitoringEnabled() const { return monitoringEnabled_; }

    // Close and cleanup
    void Close();

private:
    // SDL audio callback for capturing
    static void captureCallback(void *userdata, Uint8 *stream, int len);

    SDL_AudioDeviceID deviceID_;        // Capture device
    SDL_AudioDeviceID playbackDeviceID_; // Playback device for monitoring
    short *buffer_;
    int bufferSize_;        // Total buffer size in samples
    int currentPos_;        // Current position in buffer (in samples)
    bool isRecording_;
    bool isInitialized_;
    bool monitoringEnabled_; // Passthrough monitoring flag
};

#endif
