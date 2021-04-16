
#include <stdio.h>
#include <string.h>
#include <GetFileName.h>
#include "sd_storage.h"

#include "./fatfs/ff.h"
#include "./fatfs/ffconf.h"
#include "constants.h"
#include "main_local.h"
#include "sdlib.h"
#include "flash_mem.h"
#include "experiment.h"
#include "data_storage.h"


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
 * Call this function to ensure the SD card state on startup
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
    char* path, 
    Vector<String> &files,
    bool verbose
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
                res = scan_files(path, files, verbose);
                if (res != FR_OK) break;
                path[i] = 0;
            }
            else
            {
                sprintf(sdd.buff2, "%s/%s\r\n", path, fno.fname);
                files.push_back(String(fno.fname));
                if(verbose) printSerialWithoutLineEnding(String(sdd.buff2));
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
bool scan_files_complete(Vector<String> &files, bool verbose)
{
    check_sdcard_mounted();

    sd_clear_buffer();

    strcpy(sdd.buff, "/");  // copy starting string
    FRESULT res = scan_files(sdd.buff, files, verbose);

    // if the system cannot find the path then that is the end of the scan
    if ((res != FR_OK) && (res != FR_NO_PATH)) return false;

    return true;

} // end


bool scan_files_cli()
{
    bool verbose = true;
    Vector<String> files; 
    return scan_files_complete(files, verbose);
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
 */
bool sd_write_text_to_file(const char *filename, const char *text)
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
 * This code ensures that the observations are split into separate files per month
 * and each file name is assocaited with a UUID that is probably unique.
 */
bool write_text_to_obs_file(const char *text)
{
    String name = get_sensor_name();
    
    // THIS CODE CAN BE REMOVED
    // GetFileName fn;
    // int day, month, year, hour, minute, second, dayNum; 
    // get_time_ints(day, month, year, hour, minute, second, dayNum);

    // fn.set_device_name(name);
    // fn.set_seed(day, month, year, hour, minute, second); // will also set the month used in the file name
    // String sn;
    // sn = obtain_serial_number_string();
    // fn.set_serial_number(sn);

    // Vector<String> files;
    // bool verbose = false;
    // scan_files_complete(files, verbose);
    // String file_name = fn.get_file_name(files);

    String file_name = name + String(FILENAME_EXTENSION);
    printSerial("file name: " + file_name);

    return sd_write_text_to_file(file_name.c_str(), text);
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

