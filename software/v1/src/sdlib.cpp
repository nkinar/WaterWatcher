#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>

#include "main.h"
#include "sdlib.h"
#include "crc_7.h"
#include "string_helper.h"
#include "spi_local.h"

/*
---------------------------------------------------------------------------------------------------------------------------------
SD CARD LIBRARY
---------------------------------------------------------------------------------------------------------------------------------

Note that SD cards are MODE0, and the data is captured on the rising edge and changes on the falling edge of the clock.

---------------------------------------------------------------------------------------------------------------------------------
NOTES
---------------------------------------------------------------------------------------------------------------------------------

1. To init the SPI, a function needs to be provided so that the clockrate can be initialized.
bool init_spi(uint32_t clockrate)

A function should be provided to change the clockrate:
void spi_change_clockrate(uint32_t clockrate);

The following functions need to be provided to transfer data to SPI.

// Function to write a byte to the SPI without bringing down the \CS line
write_byte_spi_without_cs(uint8_t byte, uint32_t repeats)

// Function to write a command and then wait for a response
bool write_bytes_spi_read_resp(const uint8_t *cmd, uint32_t cmd_siz, uint8_t *resp, uint32_t resp_siz, uint32_t timeout)

// Function to write to SD card with options for the \CS line
bool spi_write_sdcard(uint8_t *buffer, uint32_t siz, uint32_t options);

// Function to read from the SD card with options for the \CS line
bool spi_read_sdcard(uint8_t *buffer, uint32_t siz, uint32_t options);

// Function to read and then write to the SD card
bool spi_read_write_sdcard(uint8_t *in, uint8_t *out, uint32_t siz, uint32_t options);

2. The functions that transmit data over the SPI bus must use MSB-first bus ordering.

3. Although rare, there are some cards that always require a valid CRC in SPI mode.  Therefore, a valid
CRC is always sent when sending commands.  This will increase the processing time slightly, but it is better to be safe and send
a valid CRC.  A valid CRC is not sent in the code when reading and writing data.  This is similar to other implementations, and it should work.

4. This code only supports SD 2.0 version cards.  Test each card before use.

5. For more information:
See pg. 246 of the SD card standard doc for the tokens.
pg. 225-227 of the SD card standard doc for the responses.
pg. 219 for the detailed command description for SD cards.

REFERENCES:
[1] https://electronics.stackexchange.com/questions/77417/what-is-the-correct-command-sequence-for-microsd-card-initialization-in-spi/238217
[2] http://nerdclub-uk.blogspot.ca/2012/11/reading-and-writing-to-single-sector-on.html
[3] https://www.8051projects.net/mmc-sd-interface-fat16/final-code.php
[4] SPI Standard version 2.0
[5] http://elm-chan.org/docs/mmc/mmc_e.html
[6] Di Jasio, L. 2011. Programming 16-Bit PIC Microcontrollers in C: Learning to Fly the PIC 24
(Some material more suitable for SD version 1.0 cards) 
[7] http://my-cool-projects.blogspot.ca/2013/02/success-talking-to-sd-card.html
[8] https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/18983

*/ 


static struct sd_card_data
{
	uint8_t command[BYTES_SD_COMMAND];
	uint8_t resp7[R7_RESP_SIZE];
	uint8_t resp3[R3_RESP_SIZE];
	uint8_t data_resp[DATA_BYTES_READ_SD];

	bool is_version2;		// true if the SD card supports version 2 
	bool is_setup;			// true if the SD card is setup
	bool is_hc;				// true if the card is high capacity
} sd;


/*
Close the SD card.
*/
bool close_sdcard_sdlib()
{
	invalidate_sd_init();
	return true;
} // end


bool check_if_sdcard_init()
{
	return sd.is_setup;
} // end


void invalidate_sd_init()
{
	sd.is_setup = false;
} // end


/* 
This function is called prior to any access to the library
*/  
bool init_sdcard_sdlib()
{
	sd.is_setup = false; 
	sd.is_version2 = false;
	sd.is_hc = false;  
	GenerateCRCTable();  // CRC requires table generation

	// ensure that the SD card is on by checking the power
	check_sd_power();

	// write 10 bytes of 0xFF without \CS to bring the SD card into SPI mode
	bool rv = write_byte_spi_without_cs(SD_IDLE_BYTE, SD_REPEATS_REQUIRED_AT_BEGINNING);
	if (!rv) return false;

	// send the CMD0 to bring the card into IDLE mode
	// print_uart("Sending CMD0");
	uint8_t r1;
	uint32_t arg = 0;
	bool send_crc = true;
	rv = send_sdcard_command(&r1, R1_RESP_SIZE, SD_CMD0, arg, send_crc);
	if (!rv)  { cleanup_sd(); return false; }
	// print_string_then_unsigned_number("r1 = ", r1);
	if (r1 != 0x01)  { cleanup_sd(); return false; } // card is not in idle state and an error occurred
	
	// CMD8
	// print_uart("Sending CMD8");
	arg = 0x1AA;
	rv = send_sdcard_command(sd.resp7, R7_RESP_SIZE, SD_CMD8, arg, send_crc);
	if (!rv)  { cleanup_sd(); return false; }
	if (sd.resp7[0] != 0x01)  { cleanup_sd(); return false; } // card must be in idle state
	uint32_t r7 = convert_to_32(sd.resp7[1], sd.resp7[2], sd.resp7[3], sd.resp7[4]);
	if (r7 != 426)  { cleanup_sd(); return false; }// nominal response with 3.3V voltage and check pattern of 0x1AA

	// print_uart("R7 = ");
	//for (int k = 0; k < R7_RESP_SIZE; k++)
	//{
	//	print_string_then_unsigned_number("", sd.resp7[k]);
	//}
	// print_string_then_unsigned_number("reg = ", r7);

	// print_uart("Sending CMD58");
	arg = 0x00;
	rv = send_sdcard_command(sd.resp3, R3_RESP_SIZE, SD_CMD58, arg, send_crc);
	if (!rv)  { cleanup_sd(); return false; }

	if (sd.resp3[0] != 0x01)  { cleanup_sd(); return false; } // card must still be in idle state
	uint32_t r3 = convert_to_32(sd.resp3[1], sd.resp3[2], sd.resp3[3], sd.resp3[4]);
	// print_uart("R3 = ");
	//for (int k = 0; k < R3_RESP_SIZE; k++)
	//{
	//    print_string_then_unsigned_number("", sd.resp3[k]);
	//}
	//print_string_then_unsigned_number("reg = ", r3);

	// Bit 20 is for 3.3V usage. Check and see if it is set.
	if (!(r3 & (1UL << 20)))
	{
	   // print_uart("Card cannot be used for 3.3V");
	   cleanup_sd(); return false;
	}
	else
	{
	    // print_uart("Card can be used for 3.3V");
	}

	//-------------------------------------------------------------------------------
	// print_uart("Entering loop to take SD card out of IDLE state");
	uint32_t timeout = TIMEOUT_CYCLES_ACMD;
	while (--timeout)
	{
	    delay(10);

		// print_uart("Sending CMD55");
		arg = 0x00;
		rv = send_sdcard_command(&r1, R1_RESP_SIZE, SD_CMD55, arg, send_crc);
		if (!rv) { cleanup_sd(); return false; }
		if (r1 == 0x04) return false; // illegal command
		// print_string_then_unsigned_number("r1 (cmd55) = ", r1);

		// \CS needs to be deselected between responses

		// print_uart("Sending ACMD41");
		arg = 0x40000000;
		rv = send_sdcard_command(&r1, R1_RESP_SIZE, SD_ACMD41, arg, send_crc);
		if (!rv) { cleanup_sd(); return false; }
		// print_string_then_unsigned_number("r1 (acmd41) = ", r1);
		if (r1 == 0x00)
		{
			// print_uart("(breaking out of loop)");
			break;
		}
	}

	// print_uart("SD now out of IDLE state");
	//print_uart("Sending CMD58 again");
	arg = 0x00;
	rv = send_sdcard_command(sd.resp3, R3_RESP_SIZE, SD_CMD58, arg, send_crc);
	if (!rv) return false;

	r3 = convert_to_32(sd.resp3[1], sd.resp3[2], sd.resp3[3], sd.resp3[4]);
	// print_uart("R3 = ");
	//for (int k = 0; k < R7_RESP_SIZE; k++)
	//{
	//	print_string_then_unsigned_number("", sd.resp3[k]);
	//}
	//print_string_then_unsigned_number("reg = ", r3);

	// if bit 31 is high, the card has finished the power-up init
	if (!(r3 & (1UL << 31))) return false;

	// check to see if the card is high capacity
	if (r3 & (1UL << 30))
	{
		sd.is_hc = true;
		//print_uart("The card is high capacity");
	}
	else
	{
		sd.is_hc = false;
		//print_uart("The card is not high capacity");
	}

	// \CS needs to be deselected here

	// By default, the block length should be 512 bytes, so we do not have to explicitly set it.

	// Change the clock rate to the MHz range if supported by the processor.
	rv = spi_change_clockrate(SDLIB_NORMAL_CLOCKRATE);
	if (!rv) return false;

	sd.is_setup = true;

	// the operation worked well
	return true;
} // end


/*
Set the address location based on the type of card
*/
uint32_t get_rw_address_loc(uint32_t sector)
{
	// pg. 106 of the standard: the SD card uses sector-based addressing if it is
	// a SDHC card
	if (sd.is_hc)
	{
		// print_uart("is HC card, returning sector");
		return sector; 
	}

	// print_uart("not HC card, returning bytes");
	
	// not SDHC card, so we use byte-based addressing
	return (BYTES_PER_SECTOR_SD * sector);
} // end


/*
Cleanup the SD if an error occurs.  This ensures that the \CS is disabled.
*/
void cleanup_sd()
{
	uint8_t b;
	uint8_t out_b = SD_IDLE_BYTE;
	spi_read_write_sdcard(&b, &out_b, 1, SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
} // end


bool wait_for_response_sd(uint8_t resp)
{
	uint32_t timeout = SD_TIMEOUT_CYCLES;
	uint8_t b;
	uint8_t out_b = SD_IDLE_BYTE;
	bool rv;
	while (--timeout)
	{
		rv = spi_read_write_sdcard(&b, &out_b, 1, SPI_TRANSFER_OPTIONS_NONE);
		// print_string_then_unsigned_number("b = ", b);
		if (rv == false) { cleanup_sd(); return false; }
		if (b == resp) break;
	}
	if (timeout == 0) { cleanup_sd(); return false; }

	return true;
} // end 


bool wait_for_resp_read()
{
	// get the response of 0x00 for the command
	//print_uart("Waiting for 0x00");
	bool rv = wait_for_response_sd(0x00);
	if (rv == false) return false;

	// get the response of 0xFE for the read memory block command
	// print_uart("Waiting for 0xFE");
	rv = wait_for_response_sd(0xFE);
	if (rv == false) return false; 

	return true;
} // end


/*
Function to read multiple sectors from the SD card

buff				= buffer of size 512*num
starting_sector		= starting sector to read
num					= number of sectors to read
*/
bool read_multiple_sector_sd(uint8_t *buff, uint32_t starting_sector, uint32_t num)
{
	// print_uart("Sending CMD18");
	bool send_crc = true;
	uint32_t arg = get_rw_address_loc(starting_sector);
	assemble_sdcard_command(SD_CMD18, arg, send_crc);
	bool rv = spi_write_sdcard(sd.command, BYTES_SD_COMMAND, SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);  // bring \CS down
	if (rv == false) { cleanup_sd(); return false; }
	// print_uart("Done CMD18");
	
	// wait for 0x00
	rv = wait_for_response_sd(0x00);
	if (rv == false) return false;

	uint32_t cnt = 0;
	for (uint32_t k = 0; k < num; k++)	// loop over the number of sectors
	{
		// print_string_then_unsigned_number("sector = ", k);

		// wait for data ready at the next block
		bool rv = wait_for_response_sd(0xFE);
		if (rv == false) return false;

		// read 512 bytes from the block
		rv = spi_read_sdcard(sd.data_resp, DATA_BYTES_READ_SD, SPI_TRANSFER_OPTIONS_NONE);
		if (rv == false) { cleanup_sd(); return false; }

		// read the two crc bytes (but do not bring \CS up)
		uint8_t bcrc0, bcrc1;
		rv = spi_read_sdcard(&bcrc0, 1, SPI_TRANSFER_OPTIONS_NONE);
		if (rv == false) { cleanup_sd(); return false; }
		rv = spi_read_sdcard(&bcrc1, 1, SPI_TRANSFER_OPTIONS_NONE);
		if (rv == false) { cleanup_sd(); return false; }

		// print_uart("Received CRC");
		// print_string_then_unsigned_number("bcrc0 = ", bcrc0);
		// print_string_then_unsigned_number("bcrc1 = ", bcrc1);
		
		// copy into the buffer
		for (uint32_t k = 0; k < DATA_BYTES_READ_SD; k++)
		{
			buff[cnt++] = sd.data_resp[k];
		}
	} // end

	// finish the transmission
	// print_uart("Sending CMD12");
	assemble_sdcard_command(SD_CMD12, arg, send_crc);
	rv = spi_write_sdcard(sd.command, BYTES_SD_COMMAND, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }

	// wait until sd card responds to the CMD12 stop (0x00 byte received)
	// print_uart("Waiting until 0x00 is received");
	rv = wait_for_response_sd(0x00);
	if (rv == false) { cleanup_sd(); return false; }

	// wait until 0xFF is received (sd card is not busy)
	// print_uart("Waiting until 0xFF is received");
	rv = wait_for_response_sd(0xFF);
	if (rv == false) { cleanup_sd(); return false; }

	// send last 0xFF byte to finish the transaction and bring \CS up
	// print_uart("Sending finish byte");
	uint8_t finish_byte = 0xFF;
	rv = spi_write_sdcard(&finish_byte, 1, SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	if (rv == false) { cleanup_sd(); return false; }

	// print_uart("Done read");

	/*
	for (uint32_t k = 0; k < DATA_BYTES_READ_SD*num; k++)
	{
		print_string_then_unsigned_number("", buff[k]);
	}
	*/

	return true;

} // end


/*
Function to read only one sector of the SD card

buff				= buffer (must have space for 512 bytes or the total number of bytes requested)
sector				= number of the sector to read (or the starting sector)
multiple_sectors	= true to be able to read multiple sectors
*/
bool read_single_sector_sd(uint8_t *buff, uint32_t sector)
{
	// print_string_then_unsigned_number("READING SECTOR = ", sector);

	// Send CMD17 for a single block read
	//print_uart("Sending CMD17");
	bool send_crc = true;
	uint32_t arg = get_rw_address_loc(sector);
	assemble_sdcard_command(SD_CMD17, arg, send_crc);

	bool rv = spi_write_sdcard(sd.command, BYTES_SD_COMMAND, SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
	if (rv == false) { cleanup_sd(); return false; }
	// print_uart("Done CMD17");

	//print_uart("waiting for 0x00 and 0xFE");
	rv = wait_for_resp_read();
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Done waiting");

	// read the 512 bytes for the sector
	rv = spi_read_sdcard(sd.data_resp, DATA_BYTES_READ_SD, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }

	//print_uart("Done reading sector");

	// read the last two crc bytes
	uint8_t bcrc0, bcrc1;
	rv = spi_read_sdcard(&bcrc0, 1, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }
	rv = spi_read_sdcard(&bcrc1, 1, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }

	// send last 0xFF byte to finish the transaction and bring \CS up
	//print_uart("Sending finish byte");
	uint8_t finish_byte = 0xFF;
	rv = spi_write_sdcard(&finish_byte, 1, SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	if (rv == false) { cleanup_sd(); return false; }

	//print_string_then_unsigned_number("bcrc0 = ", bcrc0);
	//print_string_then_unsigned_number("bcrc1 = ", bcrc1);

	// copy the buffer
	for (uint32_t k = 0; k < DATA_BYTES_READ_SD; k++)
	{
		buff[k] = sd.data_resp[k];
	}

	/*
	print_uart("data read:");
	for (uint32_t k = 0; k < DATA_BYTES_READ_SD; k++)
	{
		print_string_then_unsigned_number("", sd.data_resp[k]);
	}
	print_uart("done reading data");
	*/

	return true;
	
} // end


/* 

Write a single sector to the SD card

*/
bool write_single_sector_sd(uint8_t *buff, uint32_t sector)
{
	// print_uart("Sending CMD24");
	bool send_crc = true;
	uint32_t arg = get_rw_address_loc(sector);  // get the address location to pass to the card
	assemble_sdcard_command(SD_CMD24, arg, send_crc);
	bool rv = spi_write_sdcard(sd.command, BYTES_SD_COMMAND, SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Done CMD24");

	//print_uart("Waiting for 0x00");
	rv = wait_for_response_sd(0x00);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Received 0x00");

	uint8_t cmd = 0xFE; // write command
	//print_uart("Writing 0xFE");
	rv = spi_write_sdcard(&cmd, 1, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Done writing 0xFE");

	// write 512 block data
	//print_uart("Writing 512 block data");
	rv = spi_write_sdcard(buff, BYTES_PER_SECTOR_SD, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }

	// send dummy CRC (two bytes)
	cmd = 0xFF;
	rv = spi_write_sdcard(&cmd, 1, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }
	rv = spi_write_sdcard(&cmd, 1, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }

	// obtain response to see if the sd card accepts the data
	//print_uart("Obtaining response");
	uint8_t b;
	rv = spi_read_sdcard(&b, 1, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }
	if ((b & 0xF) != 0x05) { cleanup_sd(); return false; }
	//print_string_then_unsigned_number("b = ", b);

	// wait for 0xFF
	//print_uart("Waiting for 0xFF");
	rv = wait_for_response_sd(0xFF);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Received 0x00");

	// send a final 0xFF and bring up the \CS line
	//print_uart("Sending finish byte");
	uint8_t finish_byte = 0xFF;
	rv = spi_write_sdcard(&finish_byte, 1, SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	if (rv == false) { cleanup_sd(); return false; }

	return true;

} // end


/*
Write multiple sectors to the SD card.

CMD55 and ACMD23 are sent before the rest of the commands to ensure that the sectors are erased before write.
This ensures that data is transferred quickly from the SD card to the processor.

*/
bool write_multiple_sector_sd(uint8_t *buff, uint32_t starting_sector, uint32_t num)
{
	bool send_crc = true;
	uint32_t arg = get_rw_address_loc(starting_sector);  // get the address location to pass to the card
														 
	//-------------------------------------------------------

	// ensure erase of blocks before write
	//print_uart("Sending CMD55");
	assemble_sdcard_command(SD_CMD55, 0x00, send_crc);
	bool rv = spi_write_sdcard(sd.command, BYTES_SD_COMMAND, SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Done CMD55");

	//print_uart("Waiting for 0x00");
	rv = wait_for_response_sd(0x00);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Received 0x00");

	//print_uart("Sending ACMD23");

	// The num argument is the number of sectors to erase.
	// This command is always given in sectors.  Note that for a standard SD card (not SDHC)
	// the sector size can be changed.

	assemble_sdcard_command(SD_ACMD23, num, send_crc); 
	rv = spi_write_sdcard(sd.command, BYTES_SD_COMMAND, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Done ACMD23");

	//print_uart("Waiting for 0x00");
	rv = wait_for_response_sd(0x00);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Received 0x00");

	//-------------------------------------------------------

	//print_uart("Sending CMD25");
	assemble_sdcard_command(SD_CMD25, arg, send_crc);  // pass the starting address location (in bytes or sectors, depending on the card)
	rv = spi_write_sdcard(sd.command, BYTES_SD_COMMAND, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Done CMD55");

	//print_uart("Waiting for 0x00");
	rv = wait_for_response_sd(0x00);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Received 0x00");

	uint8_t cmd = 0x00;
	for (uint32_t k = 0; k < num; k++)
	{
		//print_uart("---Sending command for multiple write---");
		//print_string_then_unsigned_number("block = ", k);

		// send starting command for multiple write
		cmd = 0xFC;
		rv = spi_write_sdcard(&cmd, 1, SPI_TRANSFER_OPTIONS_NONE);
		if (rv == false) { cleanup_sd(); return false; }

		// write 512-block data
		//print_uart("Writing 512 block data");
		rv = spi_write_sdcard(buff, BYTES_PER_SECTOR_SD, SPI_TRANSFER_OPTIONS_NONE);
		if (rv == false) { cleanup_sd(); return false; }

		// send dummy CRC (two bytes)
		cmd = 0xFF;
		rv = spi_write_sdcard(&cmd, 1, SPI_TRANSFER_OPTIONS_NONE);
		if (rv == false) { cleanup_sd(); return false; }
		rv = spi_write_sdcard(&cmd, 1, SPI_TRANSFER_OPTIONS_NONE);
		if (rv == false) { cleanup_sd(); return false; }

		//print_uart("Obtaining response to see if SD card accepted write");
		uint8_t b;
		rv = spi_read_sdcard(&b, 1, SPI_TRANSFER_OPTIONS_NONE);
		//print_string_then_unsigned_number("b = ", b);
		if (rv == false) { cleanup_sd(); return false; }
		if ((b & 0xF) != 0x05) 
		{ 
			//print_uart("card did not accept write");
			cleanup_sd(); 
			return false;
		}
		else
		{
			//print_uart("card accepted write");
		}

	} // end

	cmd = 0xFD; // stop tran token
	rv = spi_write_sdcard(&cmd, 1, SPI_TRANSFER_OPTIONS_NONE);
	if (rv == false) { cleanup_sd(); return false; }

	// wait for 0xFF
	//print_uart("Waiting for 0xFF");
	rv = wait_for_response_sd(0xFF);
	if (rv == false) { cleanup_sd(); return false; }
	//print_uart("Received 0x00");

	// send a final 0xFF and bring up the \CS line
	//print_uart("Sending finish byte");
	uint8_t finish_byte = 0xFF;
	rv = spi_write_sdcard(&finish_byte, 1, SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	if (rv == false) { cleanup_sd(); return false; }

	//print_uart("Done operation");

	// done operation

	return true;
}// end



 /*
 Assemble the SD card command
 */
void assemble_sdcard_command(uint8_t cb, uint32_t arg, bool send_crc)
{
	sd.command[0] = (uint8_t)0x40 | cb;
	if (arg == 0)
	{
		for (int k = 1; k <= 4; k++)
		{
			sd.command[k] = 0;
		}
	}
	else
	{
		for (int k = 3; k >= 0; k--)
		{
			sd.command[4 - k] = (uint8_t)((arg >> (8 * k)) & 0xFF);
		}
	}
	uint8_t crc = 0xFF;
	if (send_crc) crc = getCRC(sd.command, BYTES_SD_COMMAND - 1);
	sd.command[BYTES_SD_COMMAND - 1] = ((crc << 1) | (uint8_t)0x01);

} // end



/*
Send SD card command

resp		= uint8_t array with the response
resp_siz	= size of the response in bytes
cb			= the command byte
arg			= the argument
send_crc	= true to send the crc

*/
bool send_sdcard_command(uint8_t *resp, uint32_t resp_siz, uint8_t cb, uint32_t arg, bool send_crc)
{
	assemble_sdcard_command(cb, arg, send_crc);
	bool rv = write_bytes_spi_read_resp(sd.command, BYTES_SD_COMMAND, resp, resp_siz, SD_TIMEOUT_CYCLES);
	if (!rv) return false;

	return true;
} // end


/*
Convert to 32 bits
*/
uint32_t convert_to_32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
	uint32_t out = ((uint32_t)b0 << 24) | ((uint32_t)b1 << 16) | ((uint32_t)b2 << 8) | (uint32_t)b3;
	return out;

} // end