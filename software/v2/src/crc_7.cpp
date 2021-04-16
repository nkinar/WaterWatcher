#include <stdint.h>
#include "crc_7.h"

/*
 * Code to generate CRC-7 for sd card 
 * REFERENCE: https://github.com/hazelnusse/crc7/blob/master/crc7.cc
 */

static struct crc_table
{
    uint8_t CRCTable[ CRC_TABLE_NUM ];
}ct;


void GenerateCRCTable()
{
  int i, j;
  uint8_t CRCPoly = 0x89;  // the value of our CRC-7 polynomial

  // generate a table value for all 256 possible byte values
  for (i = 0; i < 256; ++i)
  {
    ct.CRCTable[i] = (i & 0x80) ? i ^ CRCPoly : i;
    for (j = 1; j < 8; ++j)
    {
        ct.CRCTable[i] <<= 1;
        if (ct.CRCTable[i] & 0x80)
        {
            ct.CRCTable[i] ^= CRCPoly;
        }
    }
  }
} // end


uint8_t CRCAdd(uint8_t CRC, uint8_t message_byte)
{
    return ct.CRCTable[(CRC << 1) ^ message_byte];
} // end


uint8_t getCRC(uint8_t message[], int length)
{
  int i;
  uint8_t CRC = 0;

  for (i = 0; i < length; ++i)
  {
    CRC = CRCAdd(CRC, message[i]);
  }

  return CRC;
} // end


