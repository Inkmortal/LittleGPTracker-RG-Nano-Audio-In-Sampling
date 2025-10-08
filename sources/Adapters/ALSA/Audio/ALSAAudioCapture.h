#ifndef ALSA_AUDIO_CAPTURE_H
#define ALSA_AUDIO_CAPTURE_H

#include <alsa/asoundlib.h>
#include <string>
#include <vector>

#define MAX_RECORDING_SECONDS 30
#define WAVEFORM_BUFFER_SIZE 320  // Width of screen for waveform display

struct ALSADeviceInfo {
    std::string name;        // Device ID (e.g., "hw:1,0")
    std::string description; // Human-readable name
};

class ALSAAudioCapture {
public:
    ALSAAudioCapture();
    ~ALSAAudioCapture();

    // Device enumeration
    static std::vector<ALSADeviceInfo> EnumerateCaptureDevices();

    // Device selection
    void SetCaptureDevice(const std::string& deviceName);
    std::string GetCurrentDevice() const { return deviceName_; }

    bool Init();
    bool StartRecording(int maxSeconds, bool enableMonitoring = true);
    void StopRecording();
    short* GetBuffer(int &sampleCount);
    float GetRecordingTime() const;
    bool IsRecording() const { return isRecording_; }
    void Update(); // Call this periodically to capture audio
    void Close();

    // Waveform visualization
    const short* GetWaveformBuffer() const { return waveformBuffer_; }
    int GetWaveformSize() const { return WAVEFORM_BUFFER_SIZE; }
    float GetPeakLevel() const { return peakLevel_; }

    // Status and error reporting
    std::string GetLastError() const { return lastError_; }
    bool IsMonitoringEnabled() const { return monitoringEnabled_; }

private:
    snd_pcm_t *captureHandle_;
    snd_pcm_t *playbackHandle_;
    short *buffer_;
    int bufferSize_;
    int currentPos_;
    int maxDuration_;
    bool isRecording_;
    bool isInitialized_;
    bool monitoringEnabled_;

    unsigned int sampleRate_;
    unsigned int channels_;
    std::string deviceName_;

    // Waveform visualization
    short waveformBuffer_[WAVEFORM_BUFFER_SIZE];
    int waveformPos_;
    float peakLevel_;

    // Status and error tracking
    std::string lastError_;
};

#endif // ALSA_AUDIO_CAPTURE_H
