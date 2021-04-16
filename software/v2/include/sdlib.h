#pragma once

#include <stdint.h>

#define SDLIB_STARTING_CLOCKRATE	400000		    // 400 kHz to start the clock
// #define SDLIB_NORMAL_CLOCKRATE   5000000		    // 5 MHz for full-speed clock
#define SDLIB_NORMAL_CLOCKRATE      1000000         // 1 MHz for SD card
#define BYTES_PER_SECTOR_SD			512			    // 512 bytes per sector
#define DATA_BYTES_READ_SD			BYTES_PER_SECTOR_SD

#define SD_IDLE_BYTE						0xFF		// idle byte (MOSI high)
// #define SD_REPEATS_REQUIRED_AT_BEGINNING	10			// number of 0xFF bytes to send to the SD card when the \CS is up
#define SD_REPEATS_REQUIRED_AT_BEGINNING    100         // number of 0xFF bytes to send to the SD card when the \CS is up
#define BYTES_SD_COMMAND					6			// number of bytes in a nominal SD command
#define SD_TIMEOUT_CYCLES					1600	    // max number of timeout cycles to wait for a response
#define R1_RESP_SIZE						1			// size of the R1 response
#define R7_RESP_SIZE						5			// size of the R7 response
#define R3_RESP_SIZE						5			// size of the R3 response

#define TIMEOUT_CYCLES_ACMD					1600	    // max number of cycles to wait until card exits timeout
#define NO_BYTES_SD							""			// argument to pass no bytes
#define NO_BYTES_SD_NUM						0

//-------------------------------------------------------------------------
// COMMANDS
//-------------------------------------------------------------------------

#define SD_CMD0								0	
#define SD_CMD8								8
#define SD_CMD12							12
#define SD_CMD16							16
#define SD_CMD25							25
#define SD_CMD58							58
#define SD_CMD55							55
#define SD_ACMD41							41
#define SD_CMD17							17
#define SD_CMD18							18
#define SD_CMD24							24
#define SD_ACMD23							23

//-------------------------------------------------------------------------
// Chip select definitions
//-------------------------------------------------------------------------

// \CS down
#ifndef SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE
	#define	SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE		0x00000002
#endif

// \CS up
#ifndef SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE
	#define SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE		0x00000004
#endif

// use this define when the \CS is not to be controlled
#define SPI_TRANSFER_OPTIONS_NONE   0x00

#ifndef FT_OK
    #define FT_OK true
#endif

 #ifdef __cplusplus
 #define EXTERNC extern "C"
 #else
 #define EXTERNC
 #endif

EXTERNC bool init_sdcard_sdlib();
EXTERNC bool close_sdcard_sdlib();
EXTERNC bool send_sdcard_command(uint8_t *resp, uint32_t resp_siz, uint8_t cb, uint32_t arg, bool send_crc);
EXTERNC uint32_t convert_to_32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
EXTERNC bool read_single_sector_sd(uint8_t *buff, uint32_t sector); 
EXTERNC void assemble_sdcard_command(uint8_t cb, uint32_t arg, bool send_crc);
EXTERNC void cleanup_sd();
EXTERNC uint32_t get_rw_address_loc(uint32_t sector);
EXTERNC bool read_multiple_sector_sd(uint8_t *buff, uint32_t starting_sector, uint32_t num);
EXTERNC bool wait_for_resp_read();
EXTERNC bool wait_for_response_sd(uint8_t resp);
EXTERNC bool write_single_sector_sd(uint8_t *buff, uint32_t sector);
EXTERNC bool write_multiple_sector_sd(uint8_t *buff, uint32_t starting_sector, uint32_t num);
EXTERNC bool check_if_sdcard_init();
EXTERNC void invalidate_sd_init(); 
