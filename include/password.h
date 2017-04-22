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

	file		password.h

	date		13.10.2016

	author		Uwe Jantzen (jantzen@klabautermann-software.de)

	brief		encode() codes the password used for identification to the ftp server to
				the string to be saved in the ini file.
				decode() decodes the string from the ini file to tthe password used for
				identification to the ftp server.

	details		

	project		weather23k
	target		Linux
	begin		05.12.2010

	note		

	todo		

*/


#ifndef __PASSWORD_H__
#define __PASSWORD_H__


#include "errors.h"


#define MAX_PASSWORD_LENGTH					128


extern const char * get_pwd_filename( void );
extern ERRNO encode( void );
extern ERRNO decode( char * p_in, char * p_password );


#endif	// __PASSWORD_H__
