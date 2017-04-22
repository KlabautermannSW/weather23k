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

	file		getargs.c

	date		15.10.2016

	author		Uwe Jantzen (jantzen@klabautermann-software.de)

	brief		Read commnd line arguments

	details		

	project		weather23k
	target		Linux
	begin		05.09.2015

	note		

	todo		

*/


#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "password.h"
#include "getargs.h"


/*	function		ERRNO handle_arg( char * str )

	brief			handle command line arguments

	param[in]		char * str

	return			ERRNO
*/
ERRNO handle_arg( char * str )
	{
	ERRNO error = NOERR;
	int len = 0;
	
	if( *str == '-' )
		{
		len = strlen(str);													// get string length for further error detection
		if( len > 1)
			{
			switch( *(str+1) )
				{
				case 'v' :
					set_verbose(1);											// switch on "verbose"
					break;
				case 'd' :
					set_debug(1);											// switch on debug mode
					break;
				case 'c' :
					error = encode();
					if( error != NOERR )
						printf("password coding failed!\n");
					else
						printf("now you will find the coded password in a file named \"%s\".\n", get_pwd_filename());
					exit(error);
					break;
				default :
					error = ERR_ILLEGAL_COMMANDLINE_ARGUMENT;
					break;
				}
			}
		else
			error = ERR_ILLEGAL_COMMANDLINE_ARGUMENT;	
		}
	else
		set_ini_file(str);

	return error;
	}
