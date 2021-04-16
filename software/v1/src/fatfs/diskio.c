/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

#include "ff.h"				/* Obtains integer types */
#include "diskio.h"			/* Declarations of disk functions */
#include "sdlib.h"			/* SD card interfacing functions */

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

// default drive number for single drive (does not change)
#define DRIVE_NUMBER		0 

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if (pdrv != DRIVE_NUMBER) return STA_NOINIT;
	if (check_if_sdcard_init()) return RES_OK;

	return STA_NOINIT;
} // end


/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */ 
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	if (pdrv != DRIVE_NUMBER) return STA_NOINIT;

	bool rv = init_sdcard_sdlib();
	if (rv) return RES_OK;
		
	return STA_NOINIT;
} // end


/*-----------------------------------------------------------------------*/
/* Read Sectors(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (pdrv != DRIVE_NUMBER) return RES_PARERR;
	if (!check_if_sdcard_init()) return RES_NOTRDY;

	bool rv = false;
	if (count == 0) return RES_PARERR;
	else if (count == 1)
	{
		rv = read_single_sector_sd(buff, sector);
	}
	else // count > 1
	{
		rv = read_multiple_sector_sd(buff, sector, count);
	}
	if (rv == false) return RES_ERROR;

	return RES_OK; 
} // end



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (pdrv != DRIVE_NUMBER) return RES_PARERR;
	if (!check_if_sdcard_init()) return RES_NOTRDY;

	bool rv = false;
	if (count == 0) return RES_PARERR;
	else if (count == 1)
	{
		rv = write_single_sector_sd((uint8_t*)buff, sector);
	}
	else // count > 1
	{
		rv = write_multiple_sector_sd((uint8_t*)buff, sector, count);
	}
	if (rv == false) return RES_ERROR; 

	return RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
extern void print_debug_c(char *s);
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if (pdrv != DRIVE_NUMBER) return RES_ERROR;

	// nothing to do for this function since the work is done within the write function
	if (cmd == CTRL_SYNC) return RES_OK;

	return RES_PARERR;
} // end 


extern void get_time_rtc_cbind(int *day, int *month, int *year, int *hour, int *minute, int *second, int *dayNum);
uint32_t get_time_fatfs()
{
	int day, month, year, hour, min, second, dayNum;
	get_time_rtc_cbind(&day, &month, &year, &hour, &min, &second, &dayNum);
	uint32_t year_since_1980 = (uint32_t)year - 1980;  // year is given in actual years (such as 2019)
	uint32_t m = (uint32_t)month;
	uint32_t d = (uint32_t)day;
	uint32_t h = (uint32_t)hour;
	uint32_t minute = (uint32_t)min;
	uint32_t s = (uint32_t)second;

    DWORD word = ((DWORD)((DWORD)(year_since_1980) << 25)
            | ((DWORD)m << 21)
            | ((DWORD)d << 16)
            | ((DWORD)h << 11)
            | ((DWORD)minute << 5)
            | ((DWORD)s >> 1));
   return word;
} // end



/*
Required time for sd card
*/
DWORD get_fattime()
{
	return get_time_fatfs();

} // end

