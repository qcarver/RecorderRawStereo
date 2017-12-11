// TetraAudio.h

#ifndef _TETRAAUDIO_h
#define _TETRAAUDIO_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "wprogram.h"
#else
	#include "WProgram.h"
#endif

class TetraAudioClass
{
 protected:

 public:
	void init();
	boolean startRecording();
	uint8_t * continueRecording(uint32_t currentFrame);
	void stopRecording();
};

//extern TetraAudioClass TetraAudio;

#endif

