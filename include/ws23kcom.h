/*
    Copyright (C)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.
    If not, see <http://www.gnu.org/licenses/>.

    Klabautermann Software
    Uwe Jantzen
    Weingartener Straße 33
    76297 Stutensee
    Germany

    file        ws23kcom.h

    date        16.04.2017

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       Handle all access to WS2300 weather station

    details     

    project     weather23k
    target      Linux
    begin       16.04.2017

    note        

    todo        

*/


#ifndef __H_file__
#define __H_file__


#include <stdint.h>


#define BIT_SET                                 0x12
#define BIT_CLEAR                               0x32


extern int read_data( uint8_t * data, int addr, int n );
extern int write_data( uint8_t * data, int addr, int n, uint8_t encode_constant );
extern void handle_comm_error( ERRNO err );


#endif  // __H_file__
