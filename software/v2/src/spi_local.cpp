#include <Arduino.h>
#include <SPI.h>
#include "spi_local.h"
#include "gpio.h"
#include "constants.h"
#include "sdlib.h"
#include "sd_storage.h"


#define SPI_LOWSPEED_NUM    0
#define SPI_HIGHSPEED_NUM   1

struct sd_data
{
    bool sd_on;
    int speed;
}sdd;

// Settings objects
SPISettings spi_lowspeed(SPI_CLOCK_SLOW, MSBFIRST, SPI_MODE0);
SPISettings spi_normalspeed(SPI_CLOCK_NOMINAL, MSBFIRST, SPI_MODE0);

//---------------------------------------------------------------

/*
Ensure state of the SD card on startup
*/
void setup_sd()
{
    sdd.sd_on = false;
    sdd.speed = SPI_LOWSPEED_NUM;
} // end 


/*
Call this function to check and see if the power is ON
*/
void check_sd_power()
{
    if(sdd.sd_on==false)
    {
        on_sd_card();
    }
} // end


bool is_sd_on()
{
    return sdd.sd_on;
} // end


/*
Turn on the SD card power and ensure state of data lines when ON 
CLI: on-sd
*/
void on_sd_card()
{
    on_sd();
    delay(400);  // wait for 400 ms to ensure that card is powered and ready
    SPI.begin();
    digitalWrite(SD_CARD_CS, HIGH);
    pinMode(SD_CARD_CS, OUTPUT);
    sdd.sd_on = true;
    sdd.speed = SPI_LOWSPEED_NUM; 
} // end


/*
Turn off the SD card power and ensure state of data lines when OFF
CLI: off-sd
*/
void off_sd_card()
{
    SPI.end();
    pinMode(MISO_PIN, INPUT);
    pinMode(SCK_PIN, INPUT);
    pinMode(MOSI_PIN, INPUT);
    pinMode(SD_CARD_CS, INPUT);
    off_sd();
    invalidate_sd_init();
    invalidate_sd_mount();
    sdd.sd_on = false;
} // end

//---------------------------------------------------------------------

bool spi_change_clockrate(const uint32_t rate)
{
    if (rate == SPI_CLOCK_SLOW) sdd.speed = SPI_LOWSPEED_NUM;
    else if (rate == SPI_CLOCK_NOMINAL) sdd.speed = SPI_HIGHSPEED_NUM;
    else return false;
    return true;
} // end


void begin_spi()
{
    if (sdd.speed == SPI_LOWSPEED_NUM)
    {
        SPI.beginTransaction(spi_lowspeed);
    }
    else if (sdd.speed == SPI_HIGHSPEED_NUM)
    {
        SPI.beginTransaction(spi_normalspeed);
    }
    else  // something went wrong, so choose the lower speed by default
    {
        SPI.beginTransaction(spi_lowspeed);
    }
} // end


bool write_byte_spi_without_cs(uint8_t byte, uint32_t repeats)
{
    begin_spi();
    for(uint32_t k = 0; k < repeats; k++)
    {
        SPI.transfer(byte);
    }
    SPI.endTransaction();
    return FT_OK;
} // end


void send_bytes_spi_sdcard(uint8_t *bytes, uint32_t num)
{
    begin_spi();
       for(uint32_t k = 0; k < num; k++)
    {
        SPI.transfer(bytes[k]);
    }
    SPI.endTransaction();
} // end


bool SPI_Write(uint8_t *dat, uint32_t siz, uint32_t *sizeTransferred, uint32_t transferOptions)
{
    if (transferOptions == SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE)  digitalWrite(SD_CARD_CS, LOW);
    send_bytes_spi_sdcard(dat, siz);
    *sizeTransferred = siz;
    if (transferOptions == SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) digitalWrite(SD_CARD_CS, HIGH);
    return FT_OK;
} // end


bool SPI_ReadWrite(uint8_t *in, uint8_t *out, uint32_t siz, uint32_t *sizeTransferred, uint32_t transferOptions)
{
    begin_spi();
    if (transferOptions == SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE) digitalWrite(SD_CARD_CS, LOW);
    for(uint32_t k = 0; k < siz; k++)
    {
        in[k] = SPI.transfer(out[k]);
    }
    SPI.endTransaction();
    *sizeTransferred = siz;
    if (transferOptions == SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) digitalWrite(SD_CARD_CS, HIGH);
    return FT_OK;
} // end


bool SPI_Read(uint8_t *dat, uint32_t siz, uint32_t *sizeTransferred, uint32_t transferOptions)
{
    if (transferOptions == SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE) digitalWrite(SD_CARD_CS, LOW);
    receive_bytes_spi_sdcard(dat, siz);
    *sizeTransferred = siz;
    if (transferOptions == SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE) digitalWrite(SD_CARD_CS, HIGH);
    return FT_OK;
} // end


void receive_bytes_spi_sdcard(uint8_t *bytes, uint32_t num)
{
    for(uint32_t k = 0; k < num; k++)
    {
        bytes[k] = SPI.transfer(0xFF);
    }
    SPI.endTransaction();
} // end


bool spi_read_write_sdcard(uint8_t *in, uint8_t *out, uint32_t siz, uint32_t options)
{
    uint32_t sizeTransferred;
    return SPI_ReadWrite(in, out, siz, &sizeTransferred, options);
} // end


bool spi_write_sdcard(uint8_t *buffer, uint32_t siz, uint32_t options)
{
    uint32_t sizeTransferred;
    return SPI_Write(buffer, siz, &sizeTransferred, options);
} // end


bool spi_read_sdcard(uint8_t *buffer, uint32_t siz, uint32_t options)
{
    uint32_t sizeTransferred;
    return SPI_Read(buffer, siz, &sizeTransferred, options);
} // end


/*
Write bytes to the SPI bus and then read the response

cmd         = command to send to the SPI bus
cmd_siz     = size of the command in bytes
resp        = response to command
resp_siz    = size of the response
timeout     = number of tries to send the 0xFF byte before returning
*/
bool write_bytes_spi_read_resp(uint8_t *cmd, uint32_t cmd_siz, uint8_t *resp, uint32_t resp_siz, uint32_t timeout)
{
    uint32_t sizeTransferred;
    uint32_t transferOptions = SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE;
    bool rv = false;

    // Write the command sequence
    if (cmd_siz > 0)
    {
        rv = SPI_Write(cmd, cmd_siz, &sizeTransferred, transferOptions);
        if (rv != FT_OK) return false;
        if (sizeTransferred != cmd_siz) return false;
        transferOptions = 0;
    }
    else
    {  // cmd_siz == 0
    }

    uint32_t tt = timeout;
    uint8_t b = 0xFF;
    uint8_t IDLE_BYTE_SEND = 0xFF;
    while (--tt)
    {
        rv = SPI_ReadWrite(&b, &IDLE_BYTE_SEND, 1, &sizeTransferred, transferOptions);
        if (rv != FT_OK) return false;
        if (sizeTransferred != 1) return false;
        if (b != 0xFF) break;
    }
    if (tt == 0) return false;
    if (b == 0xFF) return false;

    // assign the first byte to the response
    resp[0] = b;

    // read the remaining bytes from the response
    if (resp_siz > 1)
    {
        rv = SPI_Read(&resp[1], resp_siz-1, &sizeTransferred, 0x00);
        if (rv != FT_OK) return false;
        if (sizeTransferred != resp_siz-1) return false;
    }

    // send one last 0xFF for completion
    delayMicroseconds(50);

    b = 0xFF;
    rv = SPI_Write(&b, 1, &sizeTransferred, SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE); // disable CS
    if (rv != FT_OK) return false;
    if (sizeTransferred != 1) return false;

    return true;
} // end


