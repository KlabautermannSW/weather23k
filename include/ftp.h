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

    file        ftp.h

    date        13.10.2016

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       establish a ftp connection to the internet server
                push the weather data to a file on the server
                kill the connection

    details     

    project     weather23k
    target      Linux
    begin       18.12.2011

    note        

    todo        

*/


#ifndef __FTP_H__
#define __FTP_H__


#include "data.h"
#include "errors.h"


extern ERRNO FtpInit( void );
extern void FtpCleanup( void );
extern ERRNO PushFile( void );
extern ERRNO AppendFile( char * logfile, char * line );


#endif  // __FTP_H__
