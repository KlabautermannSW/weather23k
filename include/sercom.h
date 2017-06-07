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

    file        sercom.h

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

    project     weather23k
    target      Linux
    begin       14.04.2017

    note        The functionality is based on the work done by Kenneth Larvsen
                (openWS23) but has advanced driver capabilities to prevent
                WS2300 hardware dependent problems when using the serial interface.

    todo        

*/


#ifndef __H_SERCOM__
#define __H_SERCOM__


#include <stddef.h>
#include <stdint.h>
#include "errors.h"


extern ERRNO ws_init( char * name );
extern ERRNO ws_open( void );
extern ERRNO ws_close( void );
extern size_t ws_read( uint8_t * dst, size_t n );
extern size_t ws_write( uint8_t * src, size_t n );
extern ERRNO ws_flush( void );
extern ERRNO ws_clear( void ); 



#endif  // __H_SERCOM__
