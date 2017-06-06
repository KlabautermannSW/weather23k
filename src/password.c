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

    file        password.c

    date        15.10.2016

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       encode() codes the password used for identification to the ftp server to
                the string to be saved in the ini file.
                decode() decodes the string from the ini file to tthe password used for
                identification to the ftp server.

    details     

    project     weather23k
    target      Linux
    begin       08.12.2010

    note        

    todo        

*/


#include "password.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>


static const char * the_password_file_name = "password.txt";


/*  function        const char * get_pwd_filename( void )

    brief           returns the pointer to the global password file name

    return          char *, pointer to the_passowrd_file_name
*/
const char * get_pwd_filename( void )
    {
    return the_password_file_name;
    }


/*  function        ERRNO encode( void )

    brief           encodes the password typed in by the user to the password file

    return          ERRNO
*/
ERRNO encode( void )
    {
    printf("Please enter your ftp passowrd :\n");

    return NOERR;
    }


/*  function        ERRNO decode( char * p_in, char * p_password )

    brief           decodes the encoded password

    param[in]       char * p_in, pointer to string containing the coded password
    param[out]      char * pointer to the string to hold the decoded password

    return          ERRNO
*/
ERRNO decode( char * p_in, char * p_password )
    {
    return NOERR;
    }
