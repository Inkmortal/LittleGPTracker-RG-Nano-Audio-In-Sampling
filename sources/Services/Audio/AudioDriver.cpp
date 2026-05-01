
#include "AudioDriver.h"
#include "System/System/System.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"
#include <stdlib.h>

#ifdef PLATFORM_RGNANO_SIM
static int gSimAudioPeak = 0;
static unsigned long gSimAudioNonSilentBytes = 0;
#endif

AudioDriver::AudioDriver(AudioSettings &settings) {
	settings_=settings ;
}

AudioDriver::~AudioDriver() {
}

bool AudioDriver::Init() {

  // Clear all buffers
	
   for (int i=0;i<SOUND_BUFFER_COUNT;i++) {
     pool_[i].buffer_=0 ;
     pool_[i].size_=0 ;
   } ;
   isPlaying_=false;	 

   return InitDriver() ;
}

void AudioDriver::Close() {
	CloseDriver() ;
};

bool AudioDriver::Start() {

    isPlaying_=true ; 

    for (int i=0;i<SOUND_BUFFER_COUNT;i++) {
  	  SAFE_FREE(pool_[i].buffer_) ;
    } ;
	 
    poolQueuePosition_=0 ;
    poolPlayPosition_=0 ;
	hasData_=false ;

    return StartDriver() ;
};

void AudioDriver::Stop() {
     isPlaying_=false ;
	hasData_=false ;
     StopDriver() ;
}

void AudioDriver::AddBuffer(short *buffer,int samplecount) {
  
  int len=samplecount*2*sizeof(short) ;

  if (!isPlaying_) return ;

  if (len>SOUND_BUFFER_MAX) {
      Trace::Error("Alert: buffer size exceeded") ;
  }

  if (pool_[poolQueuePosition_].buffer_!=0) {
  NInvalid ;
  Trace::Error("Audio overrun, please report") ;
  SAFE_FREE(pool_[poolQueuePosition_].buffer_) ;
  return ;
  }	

  pool_[poolQueuePosition_].buffer_=(char*) ((short *)SYS_MALLOC(len)) ;

  SYS_MEMCPY(pool_[poolQueuePosition_].buffer_,(char *)buffer,len) ;
#ifdef PLATFORM_RGNANO_SIM
  int sampleCount = len / (int)sizeof(short);
  for (int i = 0; i < sampleCount; i++) {
    int value = abs((int)buffer[i]);
    if (value > gSimAudioPeak) {
      gSimAudioPeak = value;
    }
    if (value > 0) {
      gSimAudioNonSilentBytes += sizeof(short);
    }
  }
#endif
  pool_[poolQueuePosition_].size_=len ;
  poolQueuePosition_=(poolQueuePosition_+1)%SOUND_BUFFER_COUNT ;
	hasData_=true ;
}

void AudioDriver::OnNewBufferNeeded() {
  SetChanged() ;
  Event event(Event::ADET_BUFFERNEEDED);
  NotifyObservers(&event) ;
} ;

void AudioDriver::onAudioBufferTick()
{
  SetChanged() ;
  Event event(Event::ADET_DRIVERTICK);
  NotifyObservers(&event) ;
}

bool AudioDriver::hasData() {
	return hasData_ ;
}  ;

AudioSettings AudioDriver::GetAudioSettings() {
	return settings_ ;
} ;

#ifdef PLATFORM_RGNANO_SIM
void AudioDriver::ResetSimAudioStats() {
  gSimAudioPeak = 0;
  gSimAudioNonSilentBytes = 0;
}

int AudioDriver::GetSimAudioPeak() { return gSimAudioPeak; }

unsigned long AudioDriver::GetSimAudioNonSilentBytes() {
  return gSimAudioNonSilentBytes;
}
#endif
