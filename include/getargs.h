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

	file		getargs.h

	date		13.10.2016

	author		Uwe Jantzen (jantzen@klabautermann-software.de)

	brief		Read commnd line arguments

	details		

	project		weather23k
	target		Linux
	begin		25.04.2011

	note		

	todo		

*/


#ifndef __GETARGS_H__
#define __GETARGS_H__


#include "errors.h"


extern ERRNO handle_arg( char * str );


#endif	// __GETARGS_H__
