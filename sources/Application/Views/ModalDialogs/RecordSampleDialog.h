#ifndef _RECORD_SAMPLE_DIALOG_H_
#define _RECORD_SAMPLE_DIALOG_H_

#include "Application/Views/BaseClasses/ModalView.h"
#include "Adapters/ALSA/Audio/ALSAAudioCapture.h"
#include <string>
#include <vector>

enum RecordState {
    RS_SELECT_DEVICE,  // Device selection screen
    RS_READY,          // Ready to record
    RS_RECORDING,      // Currently recording
    RS_STOPPED,        // Recording finished
    RS_SAVING          // Saving to file
};

class RecordSampleDialog : public ModalView {
public:
    RecordSampleDialog(View &view);
    virtual ~RecordSampleDialog();

    virtual void DrawView();
    virtual void OnPlayerUpdate(PlayerEventType type, unsigned int currentTick);
    virtual void OnFocus();
    virtual void ProcessButtonMask(unsigned short mask, bool pressed);

private:
    // Device selection
    void drawDeviceSelection();
    void selectDevice();

    // Recording controls
    void drawRecordingScreen();
    void startRecording();
    void stopRecording();
    void saveRecording();
    void cancel();
    std::string generateFileName();

    // Waveform visualization
    void drawWaveform();

    ALSAAudioCapture *capture_;
    RecordState state_;
    int recordDuration_;    // Duration in seconds
    int targetInstrument_;  // Which instrument to assign to
    std::string fileName_;
    float elapsedTime_;

    // Device selection
    std::vector<ALSADeviceInfo> availableDevices_;
    int selectedDeviceIndex_;

    // Status messages for UI display
    std::string statusMessage_;
};

#endif
