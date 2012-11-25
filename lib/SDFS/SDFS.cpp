/* mbed SDFileSystem Library, for providing file access to SD cards
 * Copyright (c) 2008-2010, sford
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* Introduction
 * ------------
 * SD and MMC cards support a number of interfaces, but common to them all
 * is one based on SPI. This is the one I'm implmenting because it means
 * it is much more portable even though not so performant, and we already
 * have the mbed SPI Interface!
 *
 * The main reference I'm using is Chapter 7, "SPI Mode" of:
 *  http://www.sdcard.org/developers/tech/sdcard/pls/Simplified_Physical_Layer_Spec.pdf
 *
 * SPI Startup
 * -----------
 * The SD card powers up in SD mode. The SPI interface mode is selected by
 * asserting CS low and sending the reset command (CMD0). The card will
 * respond with a (R1) response.
 *
 * CMD8 is optionally sent to determine the voltage range supported, and
 * indirectly determine whether it is a version 1.x SD/non-SD card or
 * version 2.x. I'll just ignore this for now.
 *
 * ACMD41 is repeatedly issued to initialise the card, until "in idle"
 * (bit 0) of the R1 response goes to '0', indicating it is initialised.
 *
 * You should also indicate whether the host supports High Capicity cards,
 * and check whether the card is high capacity - i'll also ignore this
 *
 * SPI Protocol
 * ------------
 * The SD SPI protocol is based on transactions made up of 8-bit words, with
 * the host starting every bus transaction by asserting the CS signal low. The
 * card always responds to commands, data blocks and errors.
 *
 * The protocol supports a CRC, but by default it is off (except for the
 * first reset CMD0, where the CRC can just be pre-calculated, and CMD8)
 * I'll leave the CRC off I think!
 *
 * Standard capacity cards have variable data block sizes, whereas High
 * Capacity cards fix the size of data block to 512 bytes. I'll therefore
 * just always use the Standard Capacity cards with a block size of 512 bytes.
 * This is set with CMD16.
 *
 * You can read and write single blocks (CMD17, CMD25) or multiple blocks
 * (CMD18, CMD25). For simplicity, I'll just use single block accesses. When
 * the card gets a read command, it responds with a response token, and then
 * a data token or an error.
 *
 * SPI Command Format
 * ------------------
 * Commands are 6-bytes long, containing the command, 32-bit argument, and CRC.
 *
 * +---------------+------------+------------+-----------+----------+--------------+
 * | 01 | cmd[5:0] | arg[31:24] | arg[23:16] | arg[15:8] | arg[7:0] | crc[6:0] | 1 |
 * +---------------+------------+------------+-----------+----------+--------------+
 *
 * As I'm not using CRC, I can fix that byte to what is needed for CMD0 (0x95)
 *
 * All Application Specific commands shall be preceded with APP_CMD (CMD55).
 *
 * SPI Response Format
 * -------------------
 * The main response format (R1) is a status byte (normally zero). Key flags:
 *  idle - 1 if the card is in an idle state/initialising
 *  cmd  - 1 if an illegal command code was detected
 *
 *    +-------------------------------------------------+
 * R1 | 0 | arg | addr | seq | crc | cmd | erase | idle |
 *    +-------------------------------------------------+
 *
 * R1b is the same, except it is followed by a busy signal (zeros) until
 * the first non-zero byte when it is ready again.
 *
 * Data Response Token
 * -------------------
 * Every data block written to the card is acknowledged by a byte
 * response token
 *
 * +----------------------+
 * | xxx | 0 | status | 1 |
 * +----------------------+
 *              010 - OK!
 *              101 - CRC Error
 *              110 - Write Error
 *
 * Single Block Read and Write
 * ---------------------------
 *
 * Block transfers have a byte header, followed by the data, followed
 * by a 16-bit CRC. In our case, the data will always be 512 bytes.
 *
 * +------+---------+---------+- -  - -+---------+-----------+----------+
 * | 0xFE | data[0] | data[1] |        | data[n] | crc[15:8] | crc[7:0] |
 * +------+---------+---------+- -  - -+---------+-----------+----------+
 */

/*
 * Comment: Changes for SDHC support till 32GB
 * Name:    KB
 * Date:    07/24/2010
 * Release: 0.1
 */

#include "SDFS.h"
#include <cstdio>
#include <delay.h>

SDFS::SDFS(SPI& spi, IO& cs) : spi(spi), cs(cs)
{
	isMounted = false;
}

SDFS::~SDFS()
{
	unmount();
}

int32_t SDFS::mount(const char* mountpath)
{
	if(isMounted)
		return -1;
	
	DSTATUS status = FatFS::disk_mount(this, mountpath);
	if(status)
	{
		fprintf(stderr, "disk_mount failed (%i)\r\n", status);
		return -2;
	}
	isMounted = true;
	this->mountpath = mountpath;
	return 0;
}

int32_t SDFS::unmount()
{
	if(!isMounted)
		return -1;
	DSTATUS status = FatFS::disk_umount(this);
	if(status)
	{
		fprintf(stderr, "disk_umount failed (%i)\r\n", status);
		return -2;
	}
	isMounted = false;
	return 0;
}

#define SD_COMMAND_TIMEOUT (5000)
#define SD_COMMAND_RETRY (100)

#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)

// Types
//  - v1.x Standard Capacity
//  - v2.x Standard Capacity
//  - v2.x High Capacity
//  - Not recognised as an SD Card

#define SDCARD_FAIL 0
#define SDCARD_V1   1
#define SDCARD_V2   2
#define SDCARD_V2HC 3

int SDFS::initialise_card() {
	// Set to 100kHz for initialisation, and clock card with cs = 1
	spi.setClockRate(100000);
	spi.setPullup(true);
	cs = true;
	for(int i=0; i<16; i++) {
		spi.readWrite(0xFF);
	}
	
	// send CMD0, should return with all zeros except IDLE STATE set (bit 0)
	if(_cmd(0, 0) != R1_IDLE_STATE) {
		fprintf(stderr, "No disk, or could not put SD card in to SPI idle state\r\n");
		return SDCARD_FAIL;
	}
	
	// send CMD8 to determine whther it is ver 2.x
	int r = _cmd8();
	if(r == R1_IDLE_STATE) {
		return initialise_card_v2();
	} else if(r == (R1_IDLE_STATE | R1_ILLEGAL_COMMAND)) {
		return initialise_card_v1();
	} else {
		fprintf(stderr, "Not in idle state after sending CMD8 (not an SD card?)\r\n");
		return SDCARD_FAIL;
	}
}

int SDFS::initialise_card_v1() {
	for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
		_cmd(55, 0);
		if(_cmd(41, 0) == 0) {
			cdv = 512;
			#ifdef DEBUG
			fprintf(stderr, "Init: SDCARD_V1\n\r");
			#endif
			return SDCARD_V1;
		}
	}
	
	fprintf(stderr, "Timeout waiting for v1.x card\r\n");
	return SDCARD_FAIL;
}

int SDFS::initialise_card_v2() {
	
	for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
		delay(50);
		_cmd58();
		_cmd(55, 0);
		if(_cmd(41, 0x40000000) == 0) {
			_cmd58();
			#ifdef DEBUG
			fprintf(stderr, "Init: SDCARD_V2\n\r");
			#endif
			cdv = 1;
			return SDCARD_V2;
		}
	}
	
	fprintf(stderr, "Timeout waiting for v2.x card\r\n");
	return SDCARD_FAIL;
}

int32_t SDFS::disk_initialise() {
	
	int i = initialise_card();
	#ifdef DEBUG
	fprintf(stderr, "init card = %d\r\n", i);
	#endif

	if(i == SDCARD_FAIL)
		return 3;
	
	_sectors = _sd_sectors();
	
	// Set block length to 512 (CMD16)
	if(_cmd(16, 512) != 0) {
		fprintf(stderr, "Set 512-byte block timed out\r\n");
		return 1;
	}
	
	spi.setClockRate(10000000); // Set to 10MHz for data transfer
	return 0;
}

int32_t SDFS::disk_status()
{
	//FIXME use a different means of status checking that doesn't rely on a pullup on MISO
	
	// Set block length to 512 and check response to make sure a card is present
	if(_cmd(16, 512) != 0)
		return 3;
	
	//TODO make sure the card has not been changed; return 1 if it has
		
	return 0;
}

int32_t SDFS::disk_write(const uint8_t* data, uint32_t startSector, uint32_t count)
{
// 	fprintf(stderr, "SDFS::disk_write(%x %x %x)\r\n", data, startSector, count);
	int32_t sectorsWritten = 0;
	while(count--)
	{
		for(int i = 0; ; i++)
		{
			if(i == SD_COMMAND_RETRY) //TODO try to reinitialize before failing
				return sectorsWritten;

			// set write address for single block (CMD24)
			if(_cmd(24, startSector * cdv) != 0)
				continue;
			// send the data block
			if(_write((const char*)data, 512) != 0)
				continue;
			break;
		}
		
		sectorsWritten++;
		startSector++;
		data += 512;
	}
	return sectorsWritten;
}

int32_t SDFS::disk_read(uint8_t* buffer, uint32_t startSector,	 uint32_t count)
{
// 	fprintf(stderr, "SDFS::disk_read(%x %x %x)\r\n", buffer, startSector, count);
	int32_t sectorsRead = 0;
	while(count--)
	{
		for(int i = 0; ; i++) {
			if(i == SD_COMMAND_RETRY) //TODO try to reinitialize before failing
				return sectorsRead;

			// set read address for single block (CMD17)
			if(_cmd(17, startSector * cdv) != 0)
				continue;
			// receive the data
			if(_read((char*)buffer, 512) != 0)
				continue;
			break;
		}

		sectorsRead++;
		startSector++;
		buffer += 512;
	}
	return sectorsRead;
}

int32_t SDFS::disk_ioctl(uint8_t ctrl, void* buffer) {return 0;}
int SDFS::disk_sync() { return 0; }
int SDFS::disk_sectors() { return _sectors; }

// PRIVATE FUNCTIONS

int SDFS::_cmd(int cmd, int arg) {
	cs = false;
	
	// send a command
	spi.readWrite(0x40 | cmd);
	spi.readWrite(arg >> 24);
	spi.readWrite(arg >> 16);
	spi.readWrite(arg >> 8);
	spi.readWrite(arg >> 0);
	spi.readWrite(0x95);
	
	// wait for the repsonse (response[7] == 0)
	for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
		int response = spi.readWrite(0xFF);
		if(!(response & 0x80)) { //without a pullup on MISO this will return a valid 0 response with a missing sd card
			cs = true;
			spi.readWrite(0xFF);
			return response;
		}
	}
	cs = true;
	spi.readWrite(0xFF);
	return -1; // timeout
}
int SDFS::_cmdx(int cmd, int arg) {
	cs = false;
	
	// send a command
	spi.readWrite(0x40 | cmd);
	spi.readWrite(arg >> 24);
	spi.readWrite(arg >> 16);
	spi.readWrite(arg >> 8);
	spi.readWrite(arg >> 0);
	spi.readWrite(0x95);
	
	// wait for the repsonse (response[7] == 0)
	for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
		int response = spi.readWrite(0xFF);
		if(!(response & 0x80)) {
			return response;
		}
	}
	cs = true;
	spi.readWrite(0xFF);
	return -1; // timeout
}


int SDFS::_cmd58() {
	cs = false;
	int arg = 0;
	
	// send a command
	spi.readWrite(0x40 | 58);
	spi.readWrite(arg >> 24);
	spi.readWrite(arg >> 16);
	spi.readWrite(arg >> 8);
	spi.readWrite(arg >> 0);
	spi.readWrite(0x95);
	
	// wait for the repsonse (response[7] == 0)
	for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
		int response = spi.readWrite(0xFF);
		if(!(response & 0x80)) {
			int ocr = spi.readWrite(0xFF) << 24;
			ocr |= spi.readWrite(0xFF) << 16;
			ocr |= spi.readWrite(0xFF) << 8;
			ocr |= spi.readWrite(0xFF) << 0;
// 			fprintf(stderr, "OCR = 0x%08X\r\n", ocr);
			cs = true;
			spi.readWrite(0xFF);
			return response;
		}
	}
	cs = true;
	spi.readWrite(0xFF);
	return -1; // timeout
}

int SDFS::_cmd8() {
	cs = false;
	
	// send a command
	spi.readWrite(0x40 | 8); // CMD8
	spi.readWrite(0x00);     // reserved
	spi.readWrite(0x00);     // reserved
	spi.readWrite(0x01);     // 3.3v
	spi.readWrite(0xAA);     // check pattern
	spi.readWrite(0x87);     // crc
	
	// wait for the repsonse (response[7] == 0)
	for(int i=0; i<SD_COMMAND_TIMEOUT * 1000; i++) {
		char response[5];
		response[0] = spi.readWrite(0xFF);
		if(!(response[0] & 0x80)) {
			for(int j=1; j<5; j++) {
				response[i] = spi.readWrite(0xFF);
			}
			cs = true;
			spi.readWrite(0xFF);
			return response[0];
		}
	}
	cs = true;
	spi.readWrite(0xFF);
	return -1; // timeout
}

int SDFS::_read(char *buffer, int length) {
	cs = false;
	
	// read until start byte (0xFF)
	for(int i=0; i<=SD_COMMAND_TIMEOUT; i++) {
		if(spi.readWrite(0xFF) == 0xFE)
			break;
		if(i >= SD_COMMAND_TIMEOUT)
			return 1;
	}
			  
	// read data
	for(int i=0; i<length; i++) {
		buffer[i] = spi.readWrite(0xFF);
	}
	spi.readWrite(0xFF); // checksum
	spi.readWrite(0xFF);
	
	cs = true;
	spi.readWrite(0xFF);
	return 0;
}

int SDFS::_write(const char *buffer, int length) {
	cs = false;
	
	// indicate start of block
	spi.readWrite(0xFE);
	
	// write the data
	for(int i=0; i<length; i++) {
		spi.readWrite(buffer[i]);
	}
	
	// write the checksum
	spi.readWrite(0xFF);
	spi.readWrite(0xFF);
	
	// check the repsonse token
	if((spi.readWrite(0xFF) & 0x1F) != 0x05) {
		cs = true;
		spi.readWrite(0xFF);
		return 1;
	}
	
	// wait for write to finish
	for(int i=0; ; i++) {
		if(spi.readWrite(0xFF) != 0) //TODO stricter check
			break;
		if(i >= SD_COMMAND_TIMEOUT * 1000)
		{
			cs = true;
			spi.readWrite(0xFF);
			return 1;
		}
	}
	
	cs = true;
	spi.readWrite(0xFF);
	return 0;
}

static int ext_bits(char *data, int msb, int lsb) {
	int bits = 0;
	int size = 1 + msb - lsb;
	for(int i=0; i<size; i++) {
		int position = lsb + i;
		int byte = 15 - (position >> 3);
		int bit = position & 0x7;
		int value = (data[byte] >> bit) & 1;
		bits |= value << i;
	}
	return bits;
}

int SDFS::_sd_sectors() {
	
	int c_size, c_size_mult, read_bl_len;
	int block_len, mult, blocknr, capacity;
	int blocks, hc_c_size;
	uint64_t hc_capacity;
	
	// CMD9, Response R2 (R1 byte + 16-byte block read)
	if(_cmdx(9, 0) != 0) {
		fprintf(stderr, "Didn't get a response from the disk\r\n");
		return 0;
	}
	
	char csd[16];
	if(_read(csd, 16) != 0) {
		fprintf(stderr, "Couldn't read csd response from disk\r\n");
		return 0;
	}
	
	// csd_structure : csd[127:126]
	// c_size        : csd[73:62]
	// c_size_mult   : csd[49:47]
	// read_bl_len   : csd[83:80] - the *maximum* read block length
	
	int csd_structure = ext_bits(csd, 127, 126);
	
	#ifdef DEBUG
	fprintf(stderr, "CSD_STRUCT = %d\r\n", csd_structure);
	#endif
	
	switch (csd_structure){
		case 0:
		{
			cdv = 512;
			c_size = ext_bits(csd, 73, 62);
			c_size_mult = ext_bits(csd, 49, 47);
			read_bl_len = ext_bits(csd, 83, 80);
			
			block_len = 1 << read_bl_len;
			mult = 1 << (c_size_mult + 2);
			blocknr = (c_size + 1) * mult;
			capacity = blocknr * block_len;
			blocks = capacity / 512;
			#ifdef DEBUG
			fprintf(stderr, "SDCard\n\rc_size: %.4X \n\rcapacity: %.ld MB\n\rsectors: %d\n\r", c_size, capacity/1024/1024, blocks);
			#endif
			break;
		}	
		case 1:
		{
			cdv = 1;
			hc_c_size = ext_bits(csd, 63, 48);
			int hc_read_bl_len = ext_bits(csd, 83, 80);
			hc_capacity = hc_c_size+1;
			blocks = (hc_c_size+1)*1024;
			#ifdef DEBUG
			fprintf(stderr, "SDHC Card \n\rhc_c_size: %.4X\n\rsectors: %d\n\r", hc_c_size, blocks);
			fprintf(stderr, "capacity: %llu MB\r\n", hc_capacity*512/1024);
			#endif
			break;
		}	
		default:
		{
			fprintf(stderr, "This disk tastes funny! I only know about type 0 CSD structures\r\n");
			return 0;
			break;
		}
	};
	return blocks;
}

