#ifndef _SPI_SD_LOG_DISK_h
#define _SPI_SD_LOG_DISK_h
#define BLOCK_SIZE 512

#include <Arduino.h>
class SpiSdLogDisk
{
public:
	SpiSdLogDisk();
	~SpiSdLogDisk();

	bool successful();
	bool write(uint8_t * dest);
	bool read(uint8_t * src, uint32_t whichBlock);
	uint32_t numBlocks();
	void debugPrint();

private:
	void sdErrorHalt();
	void initSizes();
};
#endif
