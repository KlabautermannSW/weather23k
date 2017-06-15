/*
    Copyright (C)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.
    If not, see <http://www.gnu.org/licenses/>.

    Klabautermann Software
    Uwe Jantzen
    Weingartener Straße 33
    76297 Stutensee
    Germany

    file        ws23kcom.c

    date        16.04.2017

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       Handle all access to WS2300 weather station

    details     

    project     weather23k
    target      Linux
    begin       16.04.2017

    note        <remarks to implementation details>

    todo        <open issues>

*/


#include "sercom.h"
#include "ws23kcom.h"
#include <stdio.h>
#include <unistd.h>


#define MAX_RETRIES                             50

#define WRITE_NIBBBLE                           0x42

#define BIT_SET                                 0x12
#define BIT_CLEAR                               0x32

#define ACK_WRITE                               0x10
#define ACK_SET                                 0x04
#define ACK_CLEAR                               0x0C

#define RESET_MIN                               0x01
#define RESET_MAX                               0x02


/*  function        int sample( int a, char * b, char * c )

    brief           <brief description of function content>

    param[in]       int a, Description of a
    param[in/out]   char * b, Description of b
    param[out]      char * c, Description of c

    return          int
*/


/********************************************************************
 * enc_address converts an 16 bit address to the form needed
 * by the WS-2300 when sending commands.
 *
 * Input:   address_in (interger - 16 bit)
 * 
 * Output:  address_out - Pointer to an uint8_tacter array.
 *          3 bytes, not zero terminated.
 * 
 * Returns: Nothing.
 *
 ********************************************************************/
/*  function        static void enc_address( int src, uint8_t * dst )

    brief           Convert an eeprom address into WS23k telegram format

    param[in]       int src, eeprom address
    param[out]      uint8_t * dst, output buffer - must be 4 bytes long at least
                                   not checked for enough space here!!
*/
static void enc_address( int src, uint8_t * dst )
{
	for( int i = 0; i < 4; ++i )
    	{
		dst[i] = (uint8_t) (0x82 + (((src >> (4 * (3 - i))) & 0x0F) * 4));
	   }
}


/********************************************************************
 * numberof_encoder converts the number of bytes we want to read
 * to the form needed by the WS-2300 when sending commands.
 *
 * Input:   number interger, max value 15
 * 
 * Returns: uint8_t which is the coded number of bytes
 *
 ********************************************************************/
/*  function        int sample( int a, char * b, char * c )

    brief           <brief description of function content>

    param[in]       int a, Description of a
    param[in/out]   char * b, Description of b
    param[out]      char * c, Description of c

    return          int
*/
uint8_t numberof_encoder(int number)
{
	int coded_number;

	coded_number = (uint8_t) (0xC2 + number * 4);
	if (coded_number > 0xfe)
		coded_number = 0xfe;

	return coded_number;
}


/********************************************************************
 * command_check0123 calculates the checksum for the first 4
 * commands sent to WS2300.
 *
 * Input:   pointer to char to check
 *          sequence of command - i.e. 0, 1, 2 or 3.
 * 
 * Returns: calculated checksum as uint8_t
 *
 ********************************************************************/
/*  function        int sample( int a, char * b, char * c )

    brief           <brief description of function content>

    param[in]       int a, Description of a
    param[in/out]   char * b, Description of b
    param[out]      char * c, Description of c

    return          int
*/
static uint8_t command_check0123(uint8_t *command, int sequence)
{
	int response;

	response = sequence * 16 + ((*command) - 0x82) / 4;

	return (uint8_t) response;
}


/********************************************************************
 * command_check4 calculates the checksum for the last command
 * which is sent just before data is received from WS2300
 *
 * Input: number of bytes requested
 * 
 * Returns: expected response from requesting number of bytes
 *
 ********************************************************************/
/*  function        int sample( int a, char * b, char * c )

    brief           <brief description of function content>

    param[in]       int a, Description of a
    param[in/out]   char * b, Description of b
    param[out]      char * c, Description of c

    return          int
*/
static uint8_t command_check4(int number)
{
	int response;

	response = 0x30 + number;

	return response;
}


/********************************************************************
 * data_checksum calculates the checksum for the data bytes received
 * from the WS2300
 *
 * Input:   pointer to array of data to check
 *          number of bytes in array
 * 
 * Returns: calculated checksum as uint8_t
 *
 ********************************************************************/
/*  function        int sample( int a, char * b, char * c )

    brief           <brief description of function content>

    param[in]       int a, Description of a
    param[in/out]   char * b, Description of b
    param[out]      char * c, Description of c

    return          int
*/
static uint8_t data_checksum(uint8_t *data, int number)
{
	int checksum = 0;
	int i;

	for (i = 0; i < number; i++)
	{
		checksum += data[i];
	}

	checksum &= 0xFF;

	return (uint8_t) checksum;
}


/*  function        ERRNO reset( void )

    brief           Reset WS2300 weather station by sending command 0x06.

                    Occasionally 0, then 2 is returned. If zero comes back, continue
                    reading as this is more efficient than sending an out-of sync
                    reset and letting the data reads restore synchronization.
                    Occasionally, multiple 2's are returned. Read with a fast timeout
                    until all data is exhausted, if we got a two back at all, we
                    consider it a success.

    return          ERRNO
*/
static ERRNO reset( void )
    {
    uint8_t cmd = 0x06;
    uint8_t answer;

    for( int i = 0; i < 100; ++i )
        {
        ws_clear();

        ws_write(&cmd, 1);

        while( ws_read(&answer, 1) == 1 )
            {
            if( answer == 0x02 )
                return NOERR;
            }

        usleep(50000 * i);                                                      //we sleep longer and longer for each retry
        }

    fprintf(stderr, "\nCould not reset\n");
    return ERR_RESET_COMMUNICATION;
    }


/*  function        int perform_read( uint8_t *data, uint8_t *cmd, int addr, int n )

    brief           Read a number of data from a given address into the buffer data using
                    the command cmd.

    param[out]      uint8_t *data, buffer to read into
    param[in]       uint8_t *cmd, command to perform
    param[in]       int addr, data array is starting here
    param[in]       int n number of bytes to read

    return          int, number of bytes read
*/
static int perform_read( uint8_t *data, uint8_t *cmd, int addr, int n )
    {

    uint8_t answer;
    int i;

    enc_address(addr, cmd);                                                 // First 4 bytes are populated with converted address range 0000-13B0
    cmd[4] = numberof_encoder(n);                                          // Last populate the 5th byte with the converted number of bytes

    for( i = 0; i < 4; ++i )
        {
        if( ws_write(cmd + i, 1) != 1 )
            return -1;
        if( ws_read(&answer, 1) != 1 )
            return -1;
        if( answer != command_check0123(cmd + i, i) )
            return -1;
        }

    // send the final command that asks for 'number' of bytes, check answer
    if( ws_write(cmd + 4, 1) != 1 )
        return -1;
    if( ws_read(&answer, 1) != 1 )
        return -1;
    if( answer != command_check4(n) )
        return -1;

    // read the data bytes
    for( i = 0; i < n; ++i )
        {
        if( ws_read(data + i, 1) != 1 )
            return -1;
        }

    // read and verify checksum
    if( ws_read(&answer, 1) != 1 )
        return -1;
    if( answer != data_checksum(data, n) )
        return -1;

    return i;
    }


/*  function        int read_data( uint8_t *data, uint8_t *cmd, int addr, int n )

    brief           Read a number of data from a given address into the buffer data using
                    the command cmd. Retry until all data is read or max retries are done.

    param[out]      uint8_t *data, buffer to read into
    param[in]       uint8_t *cmd, command to perform
    param[in]       int addr, read the data is starting here
    param[in]       int n number of bytes to read

    return          int, number of bytes read
*/
int read_data( uint8_t *data, uint8_t *cmd, int addr, int n )
    {
    for( int j = 0; j < MAX_RETRIES; ++j )
        {
        if( reset() != NOERR )
            return ERR_RESET_COMMUNICATION;

        if( perform_read(data, cmd, addr, n) == n )                             // read the data, if expected number of bytes read break out of loop
            return n;
        }

    return -1;                                                                  // could not get enough data
    }


/*  function        int perform_write( uint8_t *data, uint8_t *cmd, int addr, int n, uint8_t encode_constant )

    brief           Write a number of bytes to the WS2300 weather station.

    param[in]       uint8_t *data, buffer to write out
    param[in]       uint8_t *cmd, command to perform
    param[in]       int addr, read the data from here
    param[in]       int n number of bytes to read
    param[in]       uint8_t encode_constant

    return          int, number of bytes written
*/
static int perform_write( uint8_t *data, uint8_t *cmd, int addr, int n, uint8_t encode_constant )
    {
    uint8_t answer;
    uint8_t encoded_data[80];
    int i;
    uint8_t ack_constant = ACK_WRITE;
    
    if( encode_constant == BIT_SET )
        ack_constant = ACK_SET;
    else if( encode_constant == BIT_CLEAR )
        ack_constant = ACK_CLEAR;

    enc_address(addr, cmd);                                                 // First 4 bytes are populated with converted address range 0000-13XX
	for (i = 0; i < n; i++)
	{
		encoded_data[i] = (uint8_t) (encode_constant + (data[i] * 4));
	}

    for( i = 0; i < 4; ++i )                                                    // Write the 4 address bytes
        {
        if( ws_write(cmd + i, 1) != 1 )
            return -1;
        if( ws_read(&answer, 1) != 1 )
            return -1;
        if( answer != command_check0123(cmd + i, i) )
            return -1;
        }

    for( i = 0; i < n; ++i )                                                    // Write the data nibbles or set/unset the bits
        {
        if( ws_write(encoded_data + i, 1) != 1 )
            return -1;
        if( ws_read(&answer, 1) != 1 )
            return -1;
        if( answer != (data[i] + ack_constant) )
            return -1;
        cmd[i + 4] = encoded_data[i];
        }

    return i;
    }


/*  function        int perform_write( uint8_t *data, uint8_t *cmd, int addr, int n, uint8_t encode_constant )

    brief           Write a number of bytes to the WS2300 weather station until all
                    bytes are written or max retires are done.

    param[in]       uint8_t *data, buffer to write out
    param[in]       uint8_t *cmd, command to perform
    param[in]       int addr, read the data from here
    param[in]       int n number of bytes to read
    param[in]       uint8_t encode_constant

    return          int, number of bytes written
*/
int write_data( uint8_t *data, uint8_t *cmd, int addr, int n, uint8_t encode_constant )
{
    int j;

    for (j = 0; j < MAX_RETRIES; j++)
        {
        if( reset() != NOERR )
            return ERR_RESET_COMMUNICATION;

        if( perform_write(data, cmd, addr, n, encode_constant) == n )           // write the data, If all data written break out of loop
            return n;
        }
    
    return -1;                                                                  // could not get enough data
    }


#ifdef NIX
/********************************************************************
 * read_safe Read data, retry until success or maxretries
 * Reads data from the WS2300 based on a given address,
 * number of data read, and an already open serial port
 * Uses the read_data function and has same interface
 *
 * Inputs:  ws2300 - device number of the already open serial port
 *          address (interger - 16 bit)
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 *          commanddata - pointer to an array of chars containing
 *                     the commands that were sent to the station
 * 
 * Returns: number of bytes read, -1 if failed
 *                                  -2 if reset failed    
 ********************************************************************/
static int read_safe(WEATHERSTATION ws2300, int address, int number,
              uint8_t *readdata, uint8_t *commanddata)
{
    int j;

    for (j = 0; j < MAXRETRIES; j++)
    {
/*      reset_06(ws2300);    */
        if( reset_06(ws2300) )
            return -2;                                                          // UJA 18.12.2011

        // Read the data. If expected number of bytes read break out of loop.
        if (read_data(ws2300, address, number, readdata, commanddata)==number)
        {
            break;
        }
    }

    // If we have tried MAXRETRIES times to read we expect not to
    // have valid data
    if (j == MAXRETRIES)
    {
        return -1;
    }

    return number;
}
#endif  // NIX
