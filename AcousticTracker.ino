/**
  @file AcousticTracker.ino
  @brief Records 4 channels of Audio with two SGTL5000 on a Teensy 3.2
  gps, Compass, Environmental Sensor, and 4 microphones.
  @author Quinn Carver
  @date 12112017
*/

#include "TetraAudio.h"
#include "SpiSdLogDisk.h"

// Globals ***********************************************

// Utilities to grab 4 channels of audio
TetraAudioClass audio;

// The file where data is recorded
SpiSdLogDisk * disk = NULL;
uint32_t currentFrame = 0;
uint8_t gotData[512];

/**
  @return void
  @brief Called first when the microcontroller boots.

  Initializes all hardware subsystems.
*/
void setup() {
	disk = new SpiSdLogDisk();
  if (disk->successful()) {
	 audio.init();
	 audio.startRecording();
  }
  else Serial.println("There was an error setting up the disk.");
}

/**
  @return void
  @brief Function that is called repeditly after setup.
*/
void loop() {
	if (disk->successful()) {
		uint8_t * gotData = audio.continueRecording(currentFrame);
		if (gotData) {
		disk->write(gotData);
			currentFrame++;
		}
	}
	else Serial.print(".");
}



