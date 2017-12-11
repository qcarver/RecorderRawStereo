#include "SpiSdLogDisk.h"

// Use these SPI pins with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
const uint8_t chipSelect = SDCARD_CS_PIN;

// Initialize at highest supported speed not over 50 MHz.
// Reduce max speed if errors occur.
#define SPI_SPEED SD_SCK_MHZ(50)

// Print extra info for debug if DEBUG_PRINT is nonzero
#define DEBUG_PRINT 1
#include <SPI.h>
#include "SdFat.h"
#if DEBUG_PRINT
#include "FreeStack.h"
#endif  // DEBUG_PRINT

#define BLOCKSINAGIG 209152
#define BLOCKSINAMEG 2048

// Serial output stream
ArduinoOutStream cout(Serial);

Sd2Card card;
uint32_t cardSizeBlocks;
uint32_t cardCapacityMB;
uint32_t nextBlock;

// helps us debug disk geometry and fat data
uint32_t relSector;
uint32_t partSize;
uint8_t numberOfHeads;
uint8_t sectorsPerTrack;
uint16_t reservedSectors;
uint8_t sectorsPerCluster;
uint32_t fatStart;
uint32_t fatSize;
uint32_t dataStart;

#define BLOCKS_PER_BUFFER  8
#define BUFFER_SIZE  (BLOCKS_PER_BUFFER * BLOCK_SIZE)
uint8_t buffer[BUFFER_SIZE];

bool success = true;

//------------------------------------------------------------------------------
#define sdError(msg) {cout << F("error: ") << F(msg) << endl; sdErrorHalt(); success = false;}
//------------------------------------------------------------------------------
void SpiSdLogDisk::sdErrorHalt() {
	if (card.errorCode()) {
		cout << F("SD error: ") << hex << int(card.errorCode());
		cout << ',' << int(card.errorData()) << dec << endl;
	}
}
//------------------------------------------------------------------------------
#if DEBUG_PRINT
void SpiSdLogDisk::debugPrint() {
	cout << F("blocks: ") << card.cardSize() << endl;
	cout << F("FreeStack: ") << FreeStack() << endl;
	cout << F("partStart: ") << relSector << endl;
	cout << F("partSize: ") << partSize << endl;
	cout << F("reserved: ") << reservedSectors << endl;
	cout << F("fatStart: ") << fatStart << endl;
	cout << F("fatSize: ") << fatSize << endl;
	cout << F("dataStart: ") << dataStart << endl;
	cout << F("sectors: ") << (int)sectorsPerCluster << endl;
	cout << F("clusterCount: ");
	cout << ((relSector + partSize - dataStart) / sectorsPerCluster) << endl;
	cout << endl;
	cout << F("Heads: ") << int(numberOfHeads) << endl;
	cout << F("Sectors: ") << int(sectorsPerTrack) << endl;
	cout << F("Num Blocks:") << int(cardSizeBlocks) << endl;
	cout << F("Cylinders: ");
	cout << cardSizeBlocks / (numberOfHeads*sectorsPerTrack) << endl;
}
#endif  // DEBUG_PRINT

uint32_t SpiSdLogDisk::numBlocks() {
	return cardSizeBlocks;
}

//Write 512 bytes (1 Block) of data
bool SpiSdLogDisk::write(uint8_t * dest) {
	//cardSizeBlocks = 512; //just for a test
	boolean rv = false;

	//Copy a blocks worth of data into the next offset in the blocks-to-send buffer
	rv = memcpy(buffer + (nextBlock%BLOCKS_PER_BUFFER) * BLOCK_SIZE, (char *)dest, BLOCK_SIZE);

	//Serial.print(".");

#if DEBUG_PRINT
	if (rv == 0) cout << F("Failed to memcpy new block in");
#endif

	//is it time to write?
	if (nextBlock%BLOCKS_PER_BUFFER == BLOCKS_PER_BUFFER - 1){
#if DEBUG_PRINT
		//cout << F("Writing Blocks ") << nextBlock - (BLOCKS_PER_BUFFER - 1) << " to " << nextBlock - (BLOCKS_PER_BUFFER - 1) + BLOCKS_PER_BUFFER;
		if ((nextBlock + 1) %BLOCKSINAMEG == 0) cout << F("Wrote MegaByte #") << (nextBlock + 1)/BLOCKSINAMEG << endl;
#endif
		//write the size of the blocks-to-send buffer starting from where the first block should be
		rv = card.writeBlocks(nextBlock - (BLOCKS_PER_BUFFER - 1), buffer, BLOCKS_PER_BUFFER);

	}

	nextBlock++;
	nextBlock = (nextBlock%cardSizeBlocks);

#if DEBUG_PRINT 
	if (rv == 0) cout << F("rv zero in write");
#endif

	return (rv > 0);
}

bool SpiSdLogDisk::read(uint8_t * dest, uint32_t whichBlock) {

	boolean rv = false;
	if (whichBlock >= cardSizeBlocks) {
		rv = card.readBlocks(whichBlock, dest, 1);
	}

	return rv;
}

//------------------------------------------------------------------------------
// initialize appropriate sizes for SD capacity
void SpiSdLogDisk::initSizes() {
	if ((cardCapacityMB <= 1008) || (cardCapacityMB > 32768)) {
		cout << F("Card capacity is: ") << cardCapacityMB;
		sdError("Application requires a 16GB Ultra Plus SD HC (#152)");
	}
	//(cardCapacityMB <= 32768)
	else{
		sectorsPerCluster = 64;
	}

	if (cardCapacityMB <= 4032) {
		sdError("Application requires a 16GB Ultra Plus SD HC (#160)");
	}
	else {
		// set fake disk geometry
		numberOfHeads = 255;
		sectorsPerTrack = 63;
	}
}

SpiSdLogDisk::SpiSdLogDisk()
{
	Serial.println("Ctor called");
	nextBlock = 0;
	success = false;
	SPI.setMOSI(SDCARD_MOSI_PIN); //audio shield
	SPI.setSCK(SDCARD_SCK_PIN); //audio shield
	if (!card.begin(chipSelect, SPI_SPEED)) {
		//card !inserted? MOSI MISO chipSelect and clock pins correct?
		sdError("card.begin failed");
	}
	cardSizeBlocks = card.cardSize();
	if (cardSizeBlocks == 0) {
		sdError("cardSize is zero?");
	}
	else if (BLOCKSINAMEG % BLOCKS_PER_BUFFER != 0) {
		sdError("MegaBytes in block divided by ioBuffer size must zero"); //try 4096?
	}
	else if (BUFFER_SIZE % BLOCK_SIZE != 0) {
		sdError("ioBuffer size must be a multiple of blocks size"); //try 4096?
	}
	else {
		cardCapacityMB = (cardSizeBlocks + 2047) / 2048;

		cout << F("Card Size: ") << setprecision(0) << 1.048576*cardCapacityMB;
		cout << F(" MB, (MB = 1,000,000 bytes)") << endl;
		success = true;
		initSizes();
		debugPrint();
	}
	Serial.println("end Ctor");
}

bool SpiSdLogDisk::successful()
{
	return success;
}

SpiSdLogDisk::~SpiSdLogDisk()
{
}
