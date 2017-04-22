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

	file		ws23k.c

	date		15.10.2016

	author		Uwe Jantzen (jantzen@klabautermann-software.de)

	brief		Handle all access to WS2300 weather station
				Handle some data conversion

	details		

	project		weather23k
	target		Linux
	begin		05.09.2015

	note		

	todo		

*/


#include "rw2300.h"
#include "ws23k.h"
#include "data.h"
#include <string.h>
#include <math.h>
#include <time.h>


//#define HEIGHT				195.0											// Joehlingen
//#define HEIGHT				  5.0											// Hoernum
#define HEIGHT				114.0											// Staffort
#define G0					9.80665
#define R_STAR				287.05
#define	A					0.0065
#define CH					0.12
#define KELVIN				273.15

#define MIN_TEMP_OUT		-29.9
#define MAX_TEMP_OUT		69.9
#define MIN_HUMID			19
#define MAX_HUMID			96
#define MIN_RAIN			0
#define MAX_RAIN			999.9
#define MIN_WIND			0
#define MAX_WIND			50
#define MIN_DIR				0
#define MAX_DIR				337.5
#define MIN_TEMP_IN			-9.9
#define MAX_TEMP_IN			59.9
#define MIN_PRESSURE		300
#define MAX_PRESSURE		1099


static const char * directions[]= {"N  ","NNO","NO ","ONO","O  ","OSO","SO ","SSO", "S  ","SSW","SW ","WSW","W  ","WNW","NW ","NNW"};


static WEATHERDATA the_weatherdata;


/********************************************************************
 * read_safe Read data, retry until success or maxretries
 * Reads data from the WS2300 based on a given address,
 * number of data read, and an already open serial port
 * Uses the read_data function and has same interface
 *
 * Inputs:  ws2300 - device number of the already open serial port
 *          address (interger - 16 bit)
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 *          commanddata - pointer to an array of chars containing
 *                     the commands that were sent to the station
 * 
 * Returns: number of bytes read, -1 if failed
 *								  -2 if reset failed	
 ********************************************************************/
static int read_safe(WEATHERSTATION ws2300, int address, int number,
			  unsigned char *readdata, unsigned char *commanddata)
{
	int j;

	for (j = 0; j < MAXRETRIES; j++)
	{
/*		reset_06(ws2300);	*/
		if( reset_06(ws2300) )
			return -2;											/* UJA 18.12.2011 */
		
		// Read the data. If expected number of bytes read break out of loop.
		if (read_data(ws2300, address, number, readdata, commanddata)==number)
		{
			break;
		}
	}

	// If we have tried MAXRETRIES times to read we expect not to
	// have valid data
	if (j == MAXRETRIES)
	{
		return -1;
	}

	return number;
}


/*	function		WEATHERDATA * get_weatherdata_ptr( void )

	brief			returns tointer to the global weatherdata structue

	return			WEATHERDATA	 *, pointer to the global weatherdata structue
*/
WEATHERDATA * get_weatherdata_ptr( void )
	{
	return &the_weatherdata;
	}


/*	function		double GetRelPressure( void )

	brief			calculates the relativer air pressure from the absolute pressure and temperature
					from the weather station and a giver height

	return			double, relative pressure
*/
double GetRelPressure( void )
	{
	double x;
	double p;
	double e_approx;

	if( the_weatherdata.temperature < 9.1 )
		{
		x = 0.06 * the_weatherdata.temperature;
		e_approx = 5.6402 * (exp(x) - 0.0916);
		}
	else
		{
		x = -0.0666 * the_weatherdata.temperature;
		e_approx = 18.2194 * (1.0463 - exp(x));
		}

	x = R_STAR * ((the_weatherdata.temperature+KELVIN) + CH*e_approx + A*HEIGHT/2);
	x = G0 * HEIGHT / x;
	p = the_weatherdata.pressure * exp(x);

	return p;
	}


/*	function		void OpenWS23K( void )

	brief			opens the connection to the weather station
*/
void OpenWS23K( void )
	{
	ws2300 = open_weatherstation(com_port());
	if( verbose() )
		printf("ws23k.c : OpenWS23k\n");
	}


/*	function		void CloseWS23K( void )

	brief			closes the connection to the weather station
*/
void CloseWS23K( void )
	{
	close_weatherstation(ws2300);
	if( verbose() )
		printf("ws23k.c : CloseWS23k\n");
	}


/*	function		void ReadData( void )

	brief			reads data from the connected weather station and saves them
					to the global weather data structure
					the part commented out by NIX puts sample data to the global
					weather data structure
*/
void ReadData( void )
	{
#ifndef NIX
	time_t basictime;
	struct timespec ts;
	int minimum_code;

	ts.tv_sec = 0;
	ts.tv_nsec = 10000000;

	time(&basictime);
	strftime(the_weatherdata.act_time, sizeof(the_weatherdata.act_time)-1, "%H:%M:%S", localtime(&basictime));
	the_weatherdata.act_time[10] = 0;

	if( verbose() )
		{
		printf("time set\n");
		fflush(stdout);
		}

	the_weatherdata.temperature = temperature_outdoor(ws2300, CELCIUS);			// outdoor temperature
	nanosleep(&ts, 0);
	the_weatherdata.temperature_in = temperature_indoor(ws2300, CELCIUS);		// indoor temperature
	nanosleep(&ts, 0);
	the_weatherdata.humidity = humidity_outdoor(ws2300);
	nanosleep(&ts, 0);
	the_weatherdata.humidity_in = humidity_indoor(ws2300);
	nanosleep(&ts, 0);
	the_weatherdata.dewpoint = dewpoint(ws2300, CELCIUS);
	nanosleep(&ts, 0);
	the_weatherdata.speed[0] = wind_current_flags(ws2300, METERS_PER_SECOND, &the_weatherdata.direction,
												&the_weatherdata.sensor_connected, &minimum_code);
	nanosleep(&ts, 0);
	the_weatherdata.speed[1] = the_weatherdata.speed[0] * KILOMETERS_PER_HOUR;
	the_weatherdata.speed[2] = the_weatherdata.speed[0] * MILES_PER_HOUR;
	if( the_weatherdata.speed[0] < 0.3 )
		the_weatherdata.speed[3] = 0.0;
	else if( the_weatherdata.speed[0] < 1.4 )
		the_weatherdata.speed[3] = 1.0;
	else if( the_weatherdata.speed[0] < 3.1 )
		the_weatherdata.speed[3] = 2.0;
	else if( the_weatherdata.speed[0] < 5.3 )
		the_weatherdata.speed[3] = 3.0;
	else if( the_weatherdata.speed[0] < 7.8 )
		the_weatherdata.speed[3] = 4.0;
	else if( the_weatherdata.speed[0] < 10.5 )
		the_weatherdata.speed[3] = 5.0;
	else if( the_weatherdata.speed[0] < 13.6 )
		the_weatherdata.speed[3] = 6.0;
	else if( the_weatherdata.speed[0] < 16.9 )
		the_weatherdata.speed[3] = 7.0;
	else if( the_weatherdata.speed[0] < 20.5 )
		the_weatherdata.speed[3] = 8.0;
	else if( the_weatherdata.speed[0] < 24.4 )
		the_weatherdata.speed[3] = 9.0;
	else if( the_weatherdata.speed[0] < 28.3 )
		the_weatherdata.speed[3] = 10.0;
	else if( the_weatherdata.speed[0] < 32.5 )
		the_weatherdata.speed[3] = 11.0;
	else if( the_weatherdata.speed[0] < 37.1 )
		the_weatherdata.speed[3] = 12.0;
	else if( the_weatherdata.speed[0] < 41.6 )
		the_weatherdata.speed[3] = 13.0;
	else if( the_weatherdata.speed[0] < 14.3 )
		the_weatherdata.speed[3] = 14.0;
	else if( the_weatherdata.speed[0] < 50.6 )
		the_weatherdata.speed[3] = 15.0;
	else if( the_weatherdata.speed[0] <= 56.1 )
		the_weatherdata.speed[3] = 16.0;
	else
		the_weatherdata.speed[3] = 17.0;
	memcpy(&the_weatherdata.dir, directions[(int)(the_weatherdata.direction/22.5)], 4);
	the_weatherdata.rain_per_hour = rain_1h( ws2300, 1.0);					// mm or l/qm
	nanosleep(&ts, 0);
	the_weatherdata.rain_per_day = rain_24h(ws2300, 1.0);					// mm or l/qm
	nanosleep(&ts, 0);
	the_weatherdata.pressure = abs_pressure(ws2300, HECTOPASCAL);
	nanosleep(&ts, 0);
	the_weatherdata.windchill = windchill(ws2300, CELCIUS);
	nanosleep(&ts, 0);
#else   // NIX
	time_t basictime;
	time(&basictime);
	strftime(the_weatherdata.act_time, sizeof(the_weatherdata.act_time)-1, "%H:%M:%S", localtime(&basictime));
	the_weatherdata.act_time[10] = 0;

	if( verbose() )
		{
		printf("time set\n");
		fflush(stdout);
		}

	the_weatherdata.temperature = 15.5;			// outdoor temperature
	the_weatherdata.temperature_in = 17.4;		// indoor temperature
	the_weatherdata.humidity = 56;
	the_weatherdata.humidity_in = 32;
	the_weatherdata.dewpoint = 16.3;
	the_weatherdata.speed[0] = 10;
	the_weatherdata.direction = 275.0;
	the_weatherdata.sensor_connected = 0;
	the_weatherdata.speed[1] = the_weatherdata.speed[0] * KILOMETERS_PER_HOUR;
	the_weatherdata.speed[2] = the_weatherdata.speed[0] * MILES_PER_HOUR;
	if( the_weatherdata.speed[0] < 0.3 )
		the_weatherdata.speed[3] = 0.0;
	else if( the_weatherdata.speed[0] < 1.4 )
		the_weatherdata.speed[3] = 1.0;
	else if( the_weatherdata.speed[0] < 3.1 )
		the_weatherdata.speed[3] = 2.0;
	else if( the_weatherdata.speed[0] < 5.3 )
		the_weatherdata.speed[3] = 3.0;
	else if( the_weatherdata.speed[0] < 7.8 )
		the_weatherdata.speed[3] = 4.0;
	else if( the_weatherdata.speed[0] < 10.5 )
		the_weatherdata.speed[3] = 5.0;
	else if( the_weatherdata.speed[0] < 13.6 )
		the_weatherdata.speed[3] = 6.0;
	else if( the_weatherdata.speed[0] < 16.9 )
		the_weatherdata.speed[3] = 7.0;
	else if( the_weatherdata.speed[0] < 20.5 )
		the_weatherdata.speed[3] = 8.0;
	else if( the_weatherdata.speed[0] < 24.4 )
		the_weatherdata.speed[3] = 9.0;
	else if( the_weatherdata.speed[0] < 28.3 )
		the_weatherdata.speed[3] = 10.0;
	else if( the_weatherdata.speed[0] < 32.5 )
		the_weatherdata.speed[3] = 11.0;
	else if( the_weatherdata.speed[0] < 37.1 )
		the_weatherdata.speed[3] = 12.0;
	else if( the_weatherdata.speed[0] < 41.6 )
		the_weatherdata.speed[3] = 13.0;
	else if( the_weatherdata.speed[0] < 14.3 )
		the_weatherdata.speed[3] = 14.0;
	else if( the_weatherdata.speed[0] < 50.6 )
		the_weatherdata.speed[3] = 15.0;
	else if( the_weatherdata.speed[0] <= 56.1 )
		the_weatherdata.speed[3] = 16.0;
	else
		the_weatherdata.speed[3] = 17.0;
	memcpy(&the_weatherdata.dir, directions[(int)(the_weatherdata.direction/22.5)], 4);
	the_weatherdata.rain_per_hour = 0.0;				// mm or l/qm
	the_weatherdata.rain_per_day = 0;					// mm or l/qm
	the_weatherdata.pressure = 1005.0;
	the_weatherdata.windchill = 3.8;
#endif  // NIX

	if( verbose() )
		{
		printf("data read\n");
		fflush(stdout);
		}
	}
