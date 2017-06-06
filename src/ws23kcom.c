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

#define WRITEACK                                0x10
#define SETACK                                  0x04
#define UNSETACK                                0x0C


/*  function        int sample( int a, char * b, char * c )

    brief           <brief description of function content>

    param[in]       int a, Description of a
    param[in/out]   char * b, Description of b
    param[out]      char * c, Description of c

    return          int
*/


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
    unsigned char cmd = 0x06;
    unsigned char answer;

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


/*  function        int perform_read( unsigned char *data, unsigned char *cmd, int addr, int n )

    brief           Read a number of date form a give address into the buffer data using
                    the command cmd.

    param[out]      unsigned char *data, buffer to read into
    param[in]       unsigned char *cmd, command to perform
    param[in]       int addr, read the data from here
    param[in]       int n number of bytes to read

    return          int, number of bytes read
*/
static int perform_read( unsigned char *data, unsigned char *cmd, int addr, int n )
    {

    unsigned char answer;
    int i;

    address_encoder(addr, cmd);                                                 // First 4 bytes are populated with converted address range 0000-13B0
    cmd[4] = numberof_encoder(number);                                          // Last populate the 5th byte with the converted number of bytes

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


/*  function        int read_data( unsigned char *data, unsigned char *cmd, int addr, int n )

    brief           Read a number of date form a give address into the buffer data using
                    the command cmd. Retry until all data is read or max retries are done.

    param[out]      unsigned char *data, buffer to read into
    param[in]       unsigned char *cmd, command to perform
    param[in]       int addr, read the data from here
    param[in]       int n number of bytes to read

    return          int, number of bytes read
*/
int read_data( unsigned char *data, unsigned char *cmd, int addr, int n )
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


/*  function        int perform_write( unsigned char *data, unsigned char *cmd, int addr, int n, unsigned char encode_constant )

    brief           Write a number of bytes to the WS2300 weather station.

    param[in]       unsigned char *data, buffer to write out
    param[in]       unsigned char *cmd, command to perform
    param[in]       int addr, read the data from here
    param[in]       int n number of bytes to read
    param[in]       unsigned char encode_constant

    return          int, number of bytes written
*/
static int perform_write( unsigned char *data, unsigned char *cmd, int addr, int n, unsigned char encode_constant )
    {
    unsigned char answer;
    unsigned char encoded_data[80];
    int i;
    unsigned char ack_constant = WRITEACK;
    
    if( encode_constant == SETBIT )
        ack_constant = SETACK;
    else if( encode_constant == UNSETBIT )
        ack_constant = UNSETACK;

    address_encoder(addr, cmd);                                                 // First 4 bytes are populated with converted address range 0000-13XX
    data_encoder(n, encode_constant, data, encoded_data);                       // populate the encoded_data array

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


/*  function        int perform_write( unsigned char *data, unsigned char *cmd, int addr, int n, unsigned char encode_constant )

    brief           Write a number of bytes to the WS2300 weather station until all
                    bytes are written or max retires are done.

    param[in]       unsigned char *data, buffer to write out
    param[in]       unsigned char *cmd, command to perform
    param[in]       int addr, read the data from here
    param[in]       int n number of bytes to read
    param[in]       unsigned char encode_constant

    return          int, number of bytes written
*/
int write_data( unsigned char *data, unsigned char *cmd, int addr, int n, unsigned char encode_constant )
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
