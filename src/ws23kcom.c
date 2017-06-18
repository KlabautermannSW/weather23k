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

#define ACK_WRITE                               0x10
#define ACK_SET                                 0x04
#define ACK_CLEAR                               0x0C


/*  function        static void enc_address( int src, uint8_t * dst )

    brief           Convert an eeprom address into WS23k telegram format

    param[in]       int src, eeprom address
    param[out]      uint8_t * dst, output buffer - must be 4 bytes long at least,
                                   not checked for enough space here!!
*/
static void enc_address( int src, uint8_t * dst )
    {
    for( int i = 0; i < 4; ++i )
        dst[i] = (uint8_t) (0x82 + (((src >> (4 * (3 - i))) & 0x0F) * 4));
    }


/*  function        static uint8_t checksum_cmd( uint8_t cmd, int n )

    brief           calculates the checksum for the bytes of commnd

    param[in]       uint8_t cmd, command to caculate the checksum for
    param[in]       int n, the n'th byte of command

    return          uint8_t, cjecksum
*/
static uint8_t checksum_cmd( uint8_t cmd, int n )
    {
    return (uint8_t)(n * 16 + (cmd - 0x82) / 4);
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
    uint8_t dst;

    for( int i = 0; i < 100; ++i )
        {
        ws_clear();

        ws_write(&cmd, 1);

        while( ws_read(&dst, 1) == 1 )
            {
            if( dst == 0x02 )
                return NOERR;
            }

        usleep(50000 * i);                                                      //we sleep longer and longer for each retry
        }

    fprintf(stderr, "\nCould not reset\n");
    return ERR_RESET_COMMUNICATION;
    }


/*  function        int perform_read( uint8_t * data, int addr, int n )

    brief           Read a number of data from a given address into the buffer data using
                    the command cmd.

    param[out]      uint8_t * data, buffer to read into
    param[in]       int addr, data array is starting here
    param[in]       int n, number of bytes to read

    return          int, number of bytes read
*/
static int perform_read( uint8_t * data, int addr, int n )
    {
    uint8_t cmd[5];
    uint8_t dst;
    uint8_t checksum = 0;
    int i;

    if( n > 15 )                                                                // we can't read more then 15 bytes at a single blow 
        return -1;

    enc_address(addr, cmd);                                                     // first 4 bytes are address
    cmd[4] = (uint8_t)(0xC2 + n * 4);                                           // last byte contains the number of bytes

    for( i = 0; i < 4; ++i )
        {
        if( ws_write(cmd + i, 1) != 1 )
            return -1;
        if( ws_read(&dst, 1) != 1 )
            return -1;
        if( dst != checksum_cmd(cmd[i], i) )
            return -1;
        }

    // send the final command that asks for 'number' of bytes, check dst
    if( ws_write(cmd + 4, 1) != 1 )
        return -1;
    if( ws_read(&dst, 1) != 1 )
        return -1;
    if( dst != (n + 0x30) )
        return -1;

    // read the data bytes
    for( i = 0; i < n; ++i )
        {
        if( ws_read(data + i, 1) != 1 )
            return -1;
        }

    // read and verify checksum
    if( ws_read(&dst, 1) != 1 )
        return -1;
    for( i = 0; i < n; ++i )
        checksum += data[i];
    checksum &= 0xFF;

    if( dst != checksum )
        return -1;

    return i;
    }


/*  function        int read_data( uint8_t * data, int addr, int n )

    brief           Read a number of data from a given address into the buffer data using
                    the command cmd. Retry until all data is read or max retries are done.

    param[out]      uint8_t * data, buffer to read into
    param[in]       int addr, read the data is starting here
    param[in]       int n number of bytes to read

    return          int, number of bytes read
*/
int read_data( uint8_t * data, int addr, int n )
    {
    for( int j = 0; j < MAX_RETRIES; ++j )
        {
        if( reset() != NOERR )
            return ERR_RESET_COMMUNICATION;

        if( perform_read(data, addr, n) == n )                             // read the data, if expected number of bytes read break out of loop
            return n;
        }

    return -1;                                                                  // could not get enough data
    }


/*  function        int perform_write( uint8_t * data, int addr, int n, uint8_t enc_type )

    brief           Write a number of bytes to the WS2300 weather station.

    param[in]       uint8_t * data, buffer to write out
    param[in]       int addr, read the data from here
    param[in]       int n, number of bytes to read
    param[in]       uint8_t enc_type

    return          int, number of bytes written
*/
static int perform_write( uint8_t * data, int addr, int n, uint8_t enc_type )
    {
    uint8_t dst;
    uint8_t src[80];
    uint8_t cmd[5];
    int i;
    uint8_t ack = ACK_WRITE;
    
    if( enc_type == BIT_SET )
        ack = ACK_SET;
    else if( enc_type == BIT_CLEAR )
        ack = ACK_CLEAR;

    enc_address(addr, cmd);                                                     // First 4 bytes are populated with converted address range 0000-13XX
    for( i = 0; i < n; ++i )
        src[i] = (uint8_t) (enc_type + (data[i] * 4));

    for( i = 0; i < 4; ++i )                                                    // Write the 4 address bytes
        {
        if( ws_write(cmd + i, 1) != 1 )
            return -1;
        if( ws_read(&dst, 1) != 1 )
            return -1;
        if( dst != checksum_cmd(cmd[i], i) )
            return -1;
        }

    for( i = 0; i < n; ++i )                                                    // Write the data nibbles or set/unset the bits
        {
        if( ws_write(src + i, 1) != 1 )
            return -1;
        if( ws_read(&dst, 1) != 1 )
            return -1;
        if( dst != (data[i] + ack) )
            return -1;
        cmd[i + 4] = src[i];
        }

    return i;
    }


/*  function        int write_data( uint8_t * data, int addr, int n, uint8_t enc_type )

    brief           Write a number of bytes to the WS2300 weather station until all
                    bytes are written or max retires are done.

    param[in]       uint8_t * data, buffer to write out
    param[in]       int addr, read the data from here
    param[in]       int n, number of bytes to read
    param[in]       uint8_t enc_type

    return          int, number of bytes written
*/
int write_data( uint8_t * data, int addr, int n, uint8_t enc_type )
    {
    for( int i = 0; i < MAX_RETRIES; ++i )
        {
        if( reset() != NOERR )
            return ERR_RESET_COMMUNICATION;

        if( perform_write(data, addr, n, enc_type) == n )                  // write the data, If all data written break out of loop
            return n;
        }
    
    return -1;                                                                  // could not write all data
    }


/*  function        void handle_comm_error( ERRNO err )

    brief           Closes and reopens the connection to the weather station

    param[in]       ERRNO err, to show the error
*/
void handle_comm_error( ERRNO err )
    {
    error(err);
    
    ws_close();
    usleep(8000);
    ws_open();
    }
