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

    file        debug.c

    date        08.12.2018

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       debugging gadgets

    details

    project     weather23k
    target      Linux
    begin       18.06.2017

    note

    todo

*/


#include <stdarg.h>
#include <stdio.h>
#include "data.h"
#include "debug.h"


void debug( char const * format, ... )
    {
    va_list arg;

    if( is_debug() )
        {
        va_start(arg, format);
        vfprintf(stderr, format, arg);
        va_end(arg);
        fflush(stderr);
        }
    }
