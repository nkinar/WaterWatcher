#pragma once 

/*
NOTE:
This file contains responses from the main server that are sent back to indicate
success or failure of sending data.
*/
#define XBEE_CELL_DEBUG
// Possible strings sent back from the server
static const char CELL_RECEIVED_STR[] = "RECEIVED";
static const char CELL_ERR_STR[] = "ERROR";

