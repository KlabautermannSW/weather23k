/*
    Copyright (C)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

    Klabautermann Software
    Uwe Jantzen
    Weingartener Straße 33
    76297 Stutensee
    Germany

    file        sercom.c

    date        09.10.2016

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       Handle access to a WS2300 compatible weather station.

    details     This driver provides low level access functions to a WS2300
                compatible weather station.
                The API includes
                - ws_init
                - ws_open
                - ws_close
                - ws_read
                - ws_write
                - ws_reset

    project     weather23k
    target      Linux
    begin       14.04.2017

    note        The functionality is based on the work done by Kenneth Larvsen
                (openWS23) but has advanced driver capabilities to prevent
                WS2300 hardware dependent problems when using the serial interface.

    todo        

*/


#include "sercom.h"
#include <errno.h>
#include <sys/file.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>


#define PORTNAME_LEN                    255


static int the_handle = -1;                                                     // the serial port's handle
static char the_port[PORTNAME_LEN+1] = {0 };                                    // the serial port's name


/*  function        int ws_init( char * name )

    brief           Initializes the serial line connection to the WS2300
                    weather station connected to <name>

    param[in]       char * name, a string containing a device name like
                    /dev/ttyS0 or /dev/ttyUSB1

    return          ERRNO
*/
int ws_init( char * name )
    {
    if( strlen(name) > 255 )
        return ERR_NAME;

    strncpy(the_port, name, PORTNAME_LEN);

    return NOERR;
    }


/*  function        int ws_open( void )

    brief           opens a serial connection to a WS2300 weather station using
                    the port given by ws_init()

    return          ERRNO
*/
ERRNO ws_open( void )
    {
    struct termios control;
    int portstatus;
    ERRNO error = NOERR;

    if( strlen(the_port) == 0 )
        return ERR_NO_PORT;

    the_handle = open(the_port, O_RDWR | O_NOCTTY);
    if( the_handle < 0 )
        return ERR_NO_HANDLE;

    if( flock(the_handle, LOCK_EX) < 0 )
        {
        error = ERR_LOCKED;
        goto finish_ws_open;
        }

    memset(&control, 0, sizeof(control));

    // Serial control options
    control.c_cflag |= CS8;                                                     // character size 8 bits
    control.c_cflag |= CREAD;                                                   // enable receiver
    control.c_cflag |= CLOCAL;                                                  // ignore modem control lines

    cfsetispeed(&control, B2400);
    cfsetospeed(&control, B2400);

    control.c_lflag = 0;
    control.c_iflag = IGNBRK|IGNPAR;
    control.c_cc[VTIME] = 10;                                                   // 1 sec timeout
    control.c_cc[VMIN] = 0;                                                     // block read to first char

    if( tcsetattr(the_handle, TCSANOW, &control) < 0 )
        {
        error = ERR_INIT_PORT;
        goto finish_ws_open;
        }

    tcflush(the_handle, TCIOFLUSH);

    // simulate the heavyweather behaviour
    ioctl(the_handle, TIOCMGET, &portstatus);                                   // get current port status
    portstatus |= TIOCM_DTR;
    portstatus &= ~TIOCM_RTS;
    ioctl(the_handle, TIOCMSET, &portstatus);                                   // set current port status
    usleep(10000);
    portstatus |= TIOCM_RTS;
    ioctl(the_handle, TIOCMSET, &portstatus);                                   // set current port status
    usleep(46000);
    portstatus &= ~TIOCM_DTR;
    ioctl(the_handle, TIOCMSET, &portstatus);                                   // set current port status
    sleep(2);

finish_ws_open:
    close(the_handle);
    return error;
    }


/*  function        ERRNO ws_close( void )

    brief           close the 

    return          ERRNO
*/
ERRNO ws_close( void )
    {
    int portstatus;

    // simulate the heavyweather behaviour
    tcflush(the_handle, TCIOFLUSH);

    usleep(100000);

    ioctl(the_handle, TIOCMGET, &portstatus);                                   // get current port status
    portstatus |= TIOCM_DTR;
    ioctl(the_handle, TIOCMSET, &portstatus);                                   // set current port status
    usleep(3000);
    portstatus &= ~TIOCM_DTR;
    portstatus &= ~TIOCM_RTS;
    ioctl(the_handle, TIOCMSET, &portstatus);                                   // set current port status

    tcflush(the_handle, TCIOFLUSH);

    close(the_handle);

    return NOERR;
    }


/*  function        size_t ws_read( uint8_t * dst, size_t n )

    brief           reads bytes from the weather station until no more byte are
                    sent or the destination buffer is full

    param[out]      uint8_t * dst, buffer to read into
    param[in]       size_t n, maximum number of bytes to read (size of destination buffer)

    return          size_t, number of bytes really read
*/
size_t ws_read( uint8_t * dst, size_t n )
    {
    int i;

    do
        i = read(the_handle, dst, n);
    while( i == 0 && errno == EINTR );

    return i;
    }


/*  function        size_t ws_write( uint8_t * src, size_t n )

    brief           writes the contents of src to the weather station

    param[in]       uint8_t * src, buffer to write
    param[in]       size_t n, number of bytes to write

    return          size_t, number of bytes really written
*/
size_t ws_write( uint8_t * src, size_t n )
    {
    n = write(the_handle, src, n);

    tcdrain(the_handle);                                                        // wait for all output written

    return n;
    }


/*  function        ERRNO ws_flush( void )

    brief           flushes output buffer

    return          ERRNO
*/
ERRNO ws_flush( void )
    {
    if( tcflush(the_handle, TCOFLUSH) )
        return ERR_COMM_ERR;

    return NOERR;
    }


/*  function        ERRNO ws_clear( void )

    brief           flushes input buffer

    return          ERRNO
*/
ERRNO ws_clear( void )
    {
    if( tcflush(the_handle, TCIFLUSH) )
        return ERR_COMM_ERR;

    return NOERR;
    }
