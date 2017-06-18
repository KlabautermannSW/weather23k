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

    file        data.h

    date        18.06.2017

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       Read and store data from .ini-file
                Store global data
                Hold some global types

    details     

    project     weather23k
    target      Linux
    begin       05.09.2015

    note        

    todo        

*/


#ifndef __DATA_H__
#define __DATA_H__


#include <stdio.h>
#include "errors.h"


#define VAR_UNKNOWN                             0
#define VAR_TEMP                                1
#define VAR_PRESS                               2
#define VAR_HUM                                 3
#define VAR_WINDDIR                             4
#define VAR_SPEED_M                             5
#define VAR_SPEED_KMH                           6
#define VAR_SPEED_KN                            7
#define VAR_SPEED_BF                            8
#define VAR_DEW                                 9
#define VAR_CHILL                              10
#define VAR_RPH                                11
#define VAR_RPD                                12
#define VAR_DIRSTR                             13
#define VAR_TIME                               14
#define VAR_NUM_OF_VARS                        14


extern void set_verbose( char set );
extern char verbose( void );
extern void set_debug( char set );
extern char is_debug( void );
extern void set_ini_file( char * ini_file_name );
extern char * com_port( void );
extern char * log_path( void );
extern char * ftp_server( void );
extern char * user_name( void );
extern char * user_key( void );
extern char * ftp_file( void );
extern char * ftp_string( void );
extern char * get_first_token( int * len );
extern char * get_next_token( int * len );
extern ERRNO Remove( char * p_str, char chr );
extern ERRNO Init( void );
extern void DeInit( void );
extern ERRNO PrintVariable( int var, char * dst );
extern void SetFtpString( void );


#endif  // __DATA_H__
