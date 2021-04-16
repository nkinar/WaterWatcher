
#include <stdio.h>
#include <string.h>
#include "sd_storage.h"

#include "./fatfs/ff.h"
#include "./fatfs/ffconf.h"
#include "constants.h"
#include "main.h"
#include "sdlib.h"
#include "flash_mem.h"


//-------------------------------------------------------------------------------------------
static struct sd_storage_data
{
    char buff[SD_CHAR_BUFFER];
    char buff2[SD_CHAR_BUFFER];
    bool is_mounted;
    FATFS fs;
    bool print_file_cancel_flag;
} sdd;
//-------------------------------------------------------------------------------------------


/*
 * Call this function to ensure the SD card processor state on startup
 */
void set_startup_setup_sd()
{
    sdd.is_mounted = false;
    sdd.print_file_cancel_flag = false;
    sd_clear_buffer();
} // end



/*
 * Clear the SD card buffer
 */
void sd_clear_buffer()
{
    memset(sdd.buff,0,sizeof(sdd.buff));
    memset(sdd.buff2,0,sizeof(sdd.buff2));
} // end



/*
 * Scan the files on the SD card.
 *
 * NOTE that the *path buffer will be modified.
 *
 * SOURCE: fatfs site examples
 */

FRESULT scan_files(
    char* path
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    check_sdcard_mounted();

    res = f_opendir(&dir, (const TCHAR*)path );
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fattrib & AM_DIR) {
                i = strlen(path);
                sprintf(&path[i], "/%s", (char*)fno.fname);
                res = scan_files(path);
                if (res != FR_OK) break;
                path[i] = 0;
            }
            else
            {
                sprintf(sdd.buff2, "%s/%s\r\n", path, fno.fname);
                printSerialWithoutLineEnding(String(sdd.buff2));
            }
        }
        f_closedir(&dir);
    }
    return res;
} // end


/*
 * Print a file.
 *
 * CLI: cat [filename]
 *
 */
bool sd_print_file(const char *fname)
{
    check_sdcard_mounted();

    FIL fil;       /* File object */
    FRESULT fr;    /* FatFs return code */

    // ensure that cancel flag is false before we begin printing
    sdd.print_file_cancel_flag = false;

    // clear the buffer before the file is printed
    sd_clear_buffer();

    /* Open a text file */
    fr = f_open(&fil, fname, FA_READ);
    if (fr) return false;

    /* Read all lines and display it */
    while (f_gets(sdd.buff, sizeof(sdd.buff), &fil))
    {
        if( sdd.print_file_cancel_flag) break;
        printSerialWithoutLineEnding(String(sdd.buff));
    }

    // ensure that print file cancel flag is reset
    sdd.print_file_cancel_flag = false;

    /* Close the file */
    f_close(&fil);

    return true;
} // end


/**
 * Remove file from the disk
 */
bool remove_file(const char *fname)
{
    check_sdcard_mounted();

    FRESULT fr = f_unlink(fname);
    if (fr != FR_OK) return false;
    return true;
} // end


/**
 * Call this function if we are to cancel the print file operation
 */
void cancel_printing_file()
{
    sdd.print_file_cancel_flag = true;
} // end

/*
 * Call from the CLI to scan files on the SD card.
 *
 * CLI: ls
 * (ls for list files)
 */
bool scan_files_cli()
{
    check_sdcard_mounted();

    sd_clear_buffer();

    strncpy(sdd.buff, "/", 1);  // copy starting string
    FRESULT res = scan_files(sdd.buff);

    // if the system cannot find the path then that is the end of the scan
    if ((res != FR_OK) && (res != FR_NO_PATH)) return false;

    return true;

} // end


/*
 * Check and see if the SD card has been mounted.
 *
 * If it has not been mounted, then mount the card.
 *
 * We try a number of times to mount the card if the card cannot be mounted.
 * The f_mount function will automatically try to mount the card.
 *
 */
void check_sdcard_mounted()
{
    // ensure that the sd card is mounted
    if(sdd.is_mounted == true) return;
    sdd.is_mounted = false;

    for(int i = 0; i < TIMES_TO_TRY_MOUNT_SDCARD; i++)
    {
        FRESULT res = f_mount(&sdd.fs, SD_CARD_DRIVE_LETTER,  NO_DELAY_MOUNT_SD );
        if (res == FR_OK)
        {
            sdd.is_mounted = true;
            return;
        }
    }
    sdd.is_mounted = false;
} // end


/*
 * Function to cause the SD card to be re-mounted at the next access.
 *
 * This will automatically trigger a mount of the SD card the next time that an access occurs.
 *
 */
void invalidate_sd_mount()
{
    sdd.is_mounted = false;

} // end



/*
 * Write text to file.  If the file does not exist, it will be created.
 * The text will be appended to the end of the file.
 *
 * A new file will be created if the first file cannot be written.
 * [filename].txt.V1.txt
 *
 */
bool sd_write_text_to_file(char *filename, const char *text)
{
     FIL fil;       /* File object */
     FRESULT fr;    /* FatFs return code */
     int cnt = 0;

     check_sdcard_mounted();

     while(cnt <= SD_CARD_MAX_TRIES_WRITE)
     {
         check_sdcard_mounted();

         fr = f_open(&fil, filename, FA_OPEN_ALWAYS | FA_WRITE);
         if (fr)
         {
             continue;
         }
         // by here the file is open
         f_lseek(&fil, f_size(&fil));  // place the additional text at the end of the file
         f_puts(text, &fil);
         f_puts(CRLF, &fil);           // terminate with CRLF for readability
         f_sync(&fil);
         f_close(&fil);

         return true;
     }
     return false;  // if we get here, an error has occurred and we could not write to the file
} // end


/*
 * Write the text to the observation file.
 * TODO: update to change the name of the file here
 */
bool write_text_to_obs_file(const char *text)
{
    // obtain the name of the file and write the text to file
    get_name_of_file(sdd.buff);
    return sd_write_text_to_file(sdd.buff, text);
} // end


/*
 * Obtain the name of the file.  The filename is set based on the name and number of the station.
 * This is read from the flash memory of the microcontroller.
 *
 * 
 */
void get_name_of_file(char *filename)
{
    char *name = get_sensor_name_str();
    sprintf(filename, "%s" FILENAME_EXTENSION, name);
} // end


/*
Format the SD card
TOODO: Not active until the card size can be read directly from the hardware
*/ 
bool format_sd()
{
    check_sdcard_mounted();
    BYTE work[FF_MAX_SS]; 
    FRESULT res;
    res = f_mkfs(SD_CARD_DRIVE_LETTER, FM_ANY, 0, work, sizeof work);
    printSerial(String((int)res));
    if (res==FR_OK) return true;
    return false;
} // end