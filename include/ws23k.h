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

    file        ws23k.h

    date        13.10.2016

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       Handle all access to WS2300 weather station
                Handle some data conversion

    details     

    project     weather23k
    target      Linux
    begin       30.09.2014

    note        

    todo        

*/


#ifndef __WS23K_H__
#define __WS23K_H__


typedef struct _weatherdata
    {
    double temperature;
    double temperature_in;
    double pressure;
    int humidity;
    int humidity_in;
    double direction;
    double speed[4];                                                            // m/s, kmh, kn, bft
    int sensor_connected;
    double dewpoint;
    double windchill;
    double rain_per_hour;
    double rain_per_day;
    char dir[4];
    char act_time[11];    } WEATHERDATA;


extern WEATHERDATA * get_weatherdata_ptr( void );
extern double GetRelPressure( void );
extern void ReadData( void );


#endif  // __WS23K_H__
