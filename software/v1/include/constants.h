#pragma once
#include <Arduino.h>

#define CREDIT_STR      "nj kinar @ swsl @ usask"

// #define DEBUG_WATCHDOG               // turn on this define to debug the watchdog
// #define DEBUG_GPS                    // turn on this define to debug the GPS
#define DEBUG_CELLULAR                  // turn on this define to debug the cellular

// PINS 
const int TURBIDITY_SENSOR_PIN = A0;    // A0   as the analog turbidity sensor
const int TDS_SENSOR_PIN = A1;          // A1   as the analog total dissolved solids
const int ONE_WIRE_TEMP_PIN = 0;        // D0   as the 1-wire tempeature sensor
const int ONE_WIRE_BAT_PIN = 1;         // D1   as the 1-wire battery pin
const int EXT_5V_PIN = 2;               // D2   as the external 5V pin
const int RTC_INT_PIN = 4;              // D4   as the RTC interrupt pin
const int SD_CARD_CS = 7;               // D7   as the SD card #CS pin
const int SD_ON = 6;                    // D6   as the SD card on-off pin
const int MISO_PIN = 10;                // D10  as the SD card MISO
const int SCK_PIN = 9;                  // D9   as the SD card SCK
const int MOSI_PIN = 8;                 // D8   as the SD card MOSI

// CONSTANTS
const int BAUD_RATE_DEBUG = 9600;    
const int BAUD_RATE_GPS = 9600;
const float SENSOR_V = 3.3;
const float ANALOG_MAX = 1024.0;
// Turbidity Constants
const int TURBIDITY_SAMPLES = 800;    // Number of samples to take for determining the turbidity before averaging t
const float NTU_LOW_V = 2.5;          // Low voltage threshold for NTU measurement (V)
const float NTU_OFFSET_V = 0.0;       // Offset for NTU voltage (V)
const float NTU_OFFSET_VAL = 0.0;   // Voltage offset to ensure that the NTU value is zero when calculation is performed
// TDS Constants
const int TDS_SAMPLES = 40;           // number of times to sample the TDS sensor
const float TDR_OFFSET_V = 0.0;     // offset voltage to correct for no liquid
// Temperature Constants
const float DEFAULT_TEMPERATURE = 25.0;         // default temperature if the temperature sensor does not work well
// OneWire timeout  
const int WIRE_TIMEOUT = 2000;                  // timeout in ms
// Number of times required for read of GPS data until the data is parsed (each read is a byte)
const int GPS_READ_BYTES = 79*512;              // 79 bytes per line of data 
// ON delay for power
const int ON_DELAY_5V = 2000;                   // 1 second on delay
// serial port timeout
const unsigned int SERIAL_PORT_TIMEOUT = 2000;  // 2 second delay (2000 ms)

// I2C addresses
const uint8_t SERIAL_NUMBER_ADDR = 0x50;
const uint8_t PORT_EXP_ADDR = 0x20;
const uint8_t RTC_ADDR = 0x68;

// stream numbers
static const int USB_STREAM = 1;
static const int CELL_STREAM = 2;

// COMMANDS
static const char INFO_CMD[] = "info";
static const char ON_5V_CMD[] = "on-5v";
static const char OFF_5V_CMD[] = "off-5v";
static const char SAMPLE_CMD_DEBUG[] = "sample-debug";
static const char PRINT_SERIAL_CMD[] = "print-serial"; 
static const char BSTATE_CMD[] = "bstate"; 
static const char RTC_TEMP_CMD[] = "rtemp";
static const char RTC_TIME_CMD[] = "time";
static const char RTC_CTRL_CMD[] = "rtc-control";
static const char RTC_DEFAULT_CMD[] = "rtc-default";
static const char SAM_CMD[] = "sam";
static const char OA_CMD[] = "oa";
static const char IAO_CMD[] = "iao";
static const char PRINT_COEFF_CMD[] = "print-coeff";
static const char SET_NAME_CMD[] = "set-name";
static const char WRITE_FLASH_CMD[] = "write-flash";
static const char READ_FLASH_CMD[] = "read-flash";
static const char MEM_DEFAULTS_CMD[] = "mem-defaults";
static const char BMON_ADDR_CMD[] = "bmon-addr";
static const char PRINT_BMON[] = "print-bmon";
static const char RESET_CMD[] = "reset"; 
static const char GPS_START_POLL_CMD[] = "gps-start-poll";
static const char GPS_END_POLL_CMD[] = "gps-end-poll";
static const char PRINT_GPS_CMD[] = "print-gps";
static const char SAMPLE_GPS_CMD[] = "sample-gps";
static const char ON_SD[] = "on-sd";
static const char OFF_SD[] = "off-sd";
static const char LS_SD_CMD[] = "ls"; 
static const char CAT_CMD[] = "cat";
static const char RM_CMD[] = "rm"; 
static const char ECHO_CMD[] = "echo";
static const char KEY_CMD[] = "key";
static const char CELL_SEND_CMD[] = "cell";
static const char ON_CELL[] = "on-cell";
static const char OFF_CELL[] = "off-cell";
static const char POWERSAVE_ON[] = "powersave-on";
static const char POWERSAVE_OFF[] = "powersave-off"; 
static const char SEND_CELL_ON[] = "sendcell-on";
static const char SEND_CELL_OFF[] = "sendcell-off";
static const char SAMPLE[] = "sample";

// STRINGS
static const String TRUE_STRING = "TRUE"; 
static const String FALSE_STRING = "FALSE";
static const String ERROR_STRING = "ERROR";
static const String SUCCESS_STRING = "SUCCESS";
static const String DONE_STRING = "DONE";

// Time format [dd/mm/yyyy hh:mm:ss]
#define TIME_FORMAT "%02d/%02d/%4d %02d:%02d:%02d"
static const size_t MAX_TIME_STR_SIZ = 20;

// upper bound for minutes for set alarm minutely
const int MAX_NUM_MINUTES_RTC = 10080;

// number of characters to set the name (31 in total)
const size_t MAX_NUM_CHARS_SENSOR_NAME = 32;

// number of characters in the key name
const size_t MAX_NUM_CHARS_KEY_NAME = 32;

// battery monitor sense resistor (ohms)
const float BMON_SENSE_RESISTOR = 0.05;
// do not use parasite power for the battery monitor
const bool BMON_PP = false;

// To store or not to store memory (...that is the question)
const bool DO_NOT_STORE = false; 
const bool STORE_MEM = true;
// states
const bool ON_STATE = true;
const bool OFF_STATE = false;

// SPI clock
const uint32_t SPI_CLOCK_SLOW = 400000;         // 400 kHz
const uint32_t SPI_CLOCK_NOMINAL = 1000000;     // 1 MHz

// SD CARD VARIABLES
// sd storage buffer to be used with characters
#define SD_CHAR_BUFFER          512
// times to try to mount the SD card
#define TIMES_TO_TRY_MOUNT_SDCARD   100
// Drive letters
#define SD_CARD_DRIVE_LETTER    "0:"
#define SD_CARD_DRIVE_NUM       0
// no delay mount of SD card
#define NO_DELAY_MOUNT_SD          1
// max number of times to write to the SD card
#define SD_CARD_MAX_TRIES_WRITE     16
// Filename extension for the file on the SD card
#define FILENAME_EXTENSION          ".txt"
// CRLF
#define CRLF "\r\n"

//---------------------------------------------------------------
// CELLULAR 
#define CELL_TOPIC  "SWSL-SWS"      // topic used for Hologram.io network
#define CELL_BUFFER_SIZ     8       // buffer size used to obtain data from the Hologram.io network
#define CELL_CLOSING_BRACKET ']'    // Closing bracket
#define CELL_RESP_GOOD  "[0,0]"     // indicates that the cellular response has been sent and received by the server
//---------------------------------------------------------------

const int EXPERIMENT_TICK = 3000;       // ms between timer ticks
const int GPS_RETRIES = 2;              // gps retries
const int CELL_SEND_POLL = 100000;      // number of times to poll for cellular send
const int MAX_VECTOR_SIZ = 50;          // 50 elements to be stored in the vector as the maximum size

//---------------------------------------------------------------
const int DEC_PLACES_PRINT = 7;     // number of decimal places to print float 
//---------------------------------------------------------------   

// Default name of the sensor
static const char DEFAULT_NAME_SENSOR[] = "NONAME";
//------------------------------------------------------------

