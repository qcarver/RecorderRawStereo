#include "TetraAudio.h"

#define BLOCK_SIZE 512
#define SAMPLE_SIZE sizeof(uint16_t)
#define Q_READ_SIZE 128 * SAMPLE_SIZE
const int numAudioChannels = 4;
uint8_t audioData[BLOCK_SIZE];

#ifdef MOCK_AUDIO

void init() {
	return;
}

boolean startRecording() {
	return true;
}

uint8_t * continueRecording(uint32_t currentFrame) {
	return audioData;
}

void stopRecording() {
	return;
}


#else	
//#include <Audio.h>
//#include <Wire.h>


// Audio **************************************************
// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14


unsigned long int bufferOverruns = 0;



//You can WYSIWYG these pjrc objects here: https://www.pjrc.com/teensy/gui/
// hw objects
#include "input_i2s_quad.h"
#include "output_i2s_quad.h"
#include "control_sgtl5000.h"
#include "record_queue.h"
//#include "AudioStream.h"
AudioInputI2SQuad i2sQuadIn;
AudioOutputI2SQuad i2sQuadOut;
AudioControlSGTL5000 audioShield0;
AudioControlSGTL5000 audioShield1;
//queues which will chache audio before saving to files
AudioRecordQueue queue[numAudioChannels];
//inputs from mics to queues
AudioConnection patchCord2(i2sQuadIn, 0, queue[0], 0);
AudioConnection patchCord3(i2sQuadIn, 1, queue[1], 0);
AudioConnection patchCord4(i2sQuadIn, 2, queue[2], 0);
AudioConnection patchCord5(i2sQuadIn, 3, queue[3], 0);
//end WYSIWYG

// which input on the audio shield will be used?
const int myInput = AUDIO_INPUT_LINEIN;

// *******************************************************
/**
@return void
@brief Allocates special pieces of memory and configures the two audio shields
*/
void TetraAudioClass::init()
{
		// Allocate twice as much audio queue memory as given by PJRC in stereo example
		AudioMemory(120);

		// Enable the audio shield, select input, and enable output
		audioShield0.setAddress(LOW);
		audioShield0.enable();
		audioShield0.inputSelect(myInput);
		audioShield0.volume(0.5);
		audioShield1.setAddress(HIGH);
		audioShield1.enable();
		audioShield1.inputSelect(myInput);
		audioShield1.volume(0.5);
}

/**
@brief Gets the file and queues ready for recording
@return void
*/
boolean TetraAudioClass::startRecording() {
	boolean rv = true;
	bufferOverruns = 0;

		for (int i = 0; i < numAudioChannels; i++) {
			queue[i].begin();
			queue[i].clear();
		}

	return rv;
}


/**
@brief returns data for current frame
@currentFrame the absolute frame caller is requesting data for where
@return null if no audio data for currentFrame passed in, otw a pointer to a 512 byte array
*/
uint8_t * TetraAudioClass::continueRecording(uint32_t currentFrame) {
	uint8_t * pData = NULL;
	if (queue[currentFrame % 4].available() >= 2) {
		if (queue[currentFrame % 4].available() == 30) bufferOverruns++;
		//write a block
		memcpy(audioData, queue[currentFrame % 4].readBuffer(), Q_READ_SIZE);
		memcpy(audioData + Q_READ_SIZE, queue[currentFrame % 4].readBuffer(), Q_READ_SIZE);
		//disk.write((uint8_t *)queue[currentFrame % 4].readBuffer());
		pData = audioData;
	}
	return pData;
}

/**
@return void
@brief writes any last samples out of the queue and closes files and frees buffers
*/
void TetraAudioClass::stopRecording() {
	Serial.println("stoppingRecording");
	for (int i = 0; i < numAudioChannels; i++) {
		queue[i].end();
		queue[i].clear();
	}
	Serial.print("buffer overruns: ");
	Serial.println(bufferOverruns);
}
#endif

