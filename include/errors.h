/*
	Copyright (C)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.
	If not, see <http://www.gnu.org/licenses/>.

	Klabautermann Software
	Uwe Jantzen
	Weingartener Straße 33
	76297 Stutensee
	Germany

	file		errors.h

	date		13.10.2016

	author		Uwe Jantzen (jantzen@klabautermann-software.de)

	brief		Error code handling

	details		Defines the error codes and description strings.
				Implements a verbose error output function.

	project		weather23k
	target		Linux
	begin		09.10.2016

	note		

	todo		

*/


#ifndef __H_ERRORS__
#define __H_ERRORS__


#define NOERR									0

#define ERR_NAME								-1		// port name too long
#define ERR_NO_PORT								-2		// no port given
#define ERR_NO_HANDLE							-3		// opening serial port failed
#define ERR_LOCKED								-4		// serial port is locked
#define ERR_INIT_PORT							-5		// Unable to initialize serial port

#define ERR_COMM_ERR							-10		// general serial communications error

#define ERR_ILLEGAL_STRING_LEGNTH				-21
#define ERR_NO_INIFILE							-22
#define	ERR_EOF									-23
#define	ERR_UNKNOWN								-24
#define ERR_ILLEGAL_KEYLINE						-25
#define ERR_ILLEGAL_KEY_TYPE					-26
#define ERR_OUT_OF_MEMORY						-27
#define ERR_ILLEGAL_STRING_PTR					-28
#define ERR_GET_FILE_LENGTH						-29
#define	ERR_NOT_ENOUGH_MEMORY					-30
#define ERR_VAR_UNKNOWN							-31
#define ERR_OPEN_FILE							-32
#define ERR_PASSWORD_TO_LONG					-33
#define ERR_OPEN_LOG_FILE						-34
#define ERR_ILLEGAL_COMMANDLINE_ARGUMENT		-35
#define ERR_CURL_INIERRNOOR						-36
#define ERR_CURL_EASY_INIERRNOOR				-37
#define ERR_CURL_HEADERLISERRNOOR				-38
#define ERR_CURL_SETOPERRNOOR					-39
#define ERR_CURL_PERFORM_ERROR					-40
#define ERR_RESET_COMMUNICATION					-41


typedef int ERRNO;


extern void error( ERRNO err );


#endif	// __H_ERRORS__
