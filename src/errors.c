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

    file        errors.c

    date        09.10.2016

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       Error code handling

    details     Defines the error codes and description strings.
                Implements a verbose error output function.

    project     weather23k
    target      Linux
    begin       09.10.2016

    note        

    todo        

*/


#include <stdio.h>
#include "errors.h"
#include "data.h"


static char * errors[] =
    {
    "No error",                                                                 //   0
    "Port name longer than 255 characters",
    "No port name given",
    "opening serial port failed",
    "serial port is locked",
    "Unable to initialize serial port",
    "",
    "",
    "",
    "",
    "serial port general communication error",                                  //  10
    "reading from serial port failed",
    "writing to serial port failed",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                                                                         //  20
    "illegal string length",
    "no configuration file",
    "unexpected end of file",
    "unknown error",
    "configuration file : illegal key line",
    "configuration file : illegal key",
    "out of memory",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    0
    };


/*  function        void error( ERRNO err )

    brief           Prints a brief description of error "err" to stdout if
                    verbose mode is set.

    param[in]       ERRNO err, code of error to describe.

*/
void error( ERRNO err )
    {
    int idx;
    int count;

    idx = (int)err * -1;                                                        // switch error code to index into the text array
    if( idx < 0 )                                                               // just in case
        {
        fprintf(stderr, "Error %3d --- This is an implementation bug! Error code should NOT be positive!\n", err);
        return;
        }

    if( verbose() )
        {
        for( count = 0; errors[count]; ++count )
            {
            if( idx == count )
                break;
            }

        if( errors[count] )
            fprintf(stderr, "Error %3d : %s\n", err, errors[count]);
        else
            fprintf(stderr, "Error %3d : unknown error code !\n", err);
        }
    }
