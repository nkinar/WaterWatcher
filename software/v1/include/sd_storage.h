#pragma once

#include <stdint.h>
#include "./fatfs/ff.h"

void set_startup_setup_sd();
void sd_clear_buffer();
void sd_clear_buffer();
FRESULT scan_files(char* path);
bool scan_files_cli();
void check_sdcard_mounted();
void invalidate_sd_mount();
bool sd_print_file(const char *fname);
bool sd_write_text_to_file(char *filename, const char *text);
void get_name_of_file(char *filename);
bool write_text_to_obs_file(const char *text);
void cancel_printing_file();
bool remove_file(const char *fname);
bool scan_files_default_path();
bool format_sd();

