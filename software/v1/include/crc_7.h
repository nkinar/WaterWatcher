#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CRC_TABLE_NUM   256

void GenerateCRCTable();
uint8_t CRCAdd(uint8_t CRC, uint8_t message_byte);
uint8_t getCRC(uint8_t message[], int length);
