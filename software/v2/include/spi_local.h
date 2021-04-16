#pragma once
#include <stdint.h>

void setup_sd();
void check_sd_power();
void on_sd_card();
void off_sd_card();
bool is_sd_on();
bool spi_change_clockrate(const uint32_t rate);
bool write_byte_spi_without_cs(uint8_t byte, uint32_t repeats);
void send_bytes_spi_sdcard(uint8_t *bytes, uint32_t num);
bool SPI_Write(uint8_t *dat, uint32_t siz, uint32_t *sizeTransferred, uint32_t transferOptions);
bool SPI_ReadWrite(uint8_t *in, uint8_t *out, uint32_t siz, uint32_t *sizeTransferred, uint32_t transferOptions);
bool SPI_Read(uint8_t *dat, uint32_t siz, uint32_t *sizeTransferred, uint32_t transferOptions);
void receive_bytes_spi_sdcard(uint8_t *bytes, uint32_t num);
bool write_bytes_spi_read_resp(uint8_t *cmd, uint32_t cmd_siz, uint8_t *resp, uint32_t resp_siz, uint32_t timeout);
bool spi_read_write_sdcard(uint8_t *in, uint8_t *out, uint32_t siz, uint32_t options);
bool spi_write_sdcard(uint8_t *buffer, uint32_t siz, uint32_t options);
bool spi_read_sdcard(uint8_t *buffer, uint32_t siz, uint32_t options);

