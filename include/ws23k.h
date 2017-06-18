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


#include "locals.h"
#include <stdint.h>


#define TEMP_OUTDOOR_MIN                         -29.9                          // [°C]
#define TEMP_OUTDOOR_MAX                          69.9                          // [°C]
#define TEMP_INDOOR_MIN                           -9.9                          // [°C]
#define TEMP_INDOOR_MAX                           59.9                          // [°C]
#define HUMID_MIN                                  1                            // [%]
#define HUMID_MAX                                 99                            // [%]
#define RAIN_MIN                                   0.0                          // [mm/m²]
#define RAIN_MAX                                 999.9                          // [mm/m²]
#define RAIN_MAX_TOTAL                          2499.9                          // [mm/m²]
#define WIND_MIN                                   0.0                          // [m/s]
#define WIND_MAX                                  50.0                          // [m/s]
#define ABS_PRESSURE_MIN                         300                            // [hPa]
#define ABS_PRESSURE_MAX                        1099                            // [hPa]

#define RESET_MIN                               0x01
#define RESET_MAX                               0x02

#define KMH                                        3.6                          // * m/s
#define KNOTS                                      1.943844492                  // * m/s


struct timestamp
    {
    int minute;
    int hour;
    int day;
    int month;
    int year;
    };


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
    char act_time[11];    } weatherdata_t;


extern weatherdata_t * get_weatherdata_ptr( void );


// data dirctcly from weather station
extern double temperature_indoor( void );
extern void temperature_indoor_minmax( double * temp_min, double * temp_max, struct timestamp * time_min, struct timestamp * time_max );
extern void temperature_indoor_reset( uint8_t minmax );
extern double temperature_outdoor( void );
extern void temperature_outdoor_minmax( double * temp_min, double * temp_max, struct timestamp * time_min, struct timestamp * time_max );
extern void temperature_outdoor_reset( uint8_t minmax );
extern double dewpoint( void );
extern void dewpoint_minmax( double * dp_min, double * dp_max, struct timestamp * time_min, struct timestamp * time_max );
extern void dewpoint_reset( uint8_t minmax );
extern int humidity_indoor( void );
extern int humidity_indoor_all( int * hum_min, int * hum_max, struct timestamp * time_min, struct timestamp * time_max );
extern void humidity_indoorr_reset( uint8_t minmax );
extern int humidity_outdoor( void );
extern int humidity_outdoor_all( int * hum_min, int * hum_max, struct timestamp * time_min, struct timestamp * time_max );
extern void humidity_outdoor_reset( uint8_t minmax );
extern double wind_current( double * winddir );
extern double wind_current_flags( double * winddir, int * sensor_connected, int * minimum_code );
extern double wind_all( int * winddir_index, double * winddir );
extern double wind_minmax( double * wind_min, double * wind_max, struct timestamp * time_min, struct timestamp * time_max );
extern void wind_reset( uint8_t minmax );
extern double windchill( void );
extern void windchill_minmax( double * wc_min, double * wc_max, struct timestamp * time_min, struct timestamp * time_max );
extern void windchill_reset( uint8_t minmax );
extern double rain_1h( void );
extern double rain_1h_all( double * rain_max, struct timestamp * time_max );
extern void rain_1h_max_reset( void );
extern void rain_1h_reset( void );
extern double rain_24h( void );
extern double rain_24h_all( double * rain_max, struct timestamp * time_max );
extern void rain_24h_max_reset( void );
extern void rain_24h_reset( void );
extern double rain_total( void );
extern double rain_total_all( struct timestamp * time_since );
extern void rain_total_reset( void );
extern double rel_pressure( void );
extern void rel_pressure_minmax( double * pres_min, double * pres_max, struct timestamp * time_min, struct timestamp * time_max );
extern double abs_pressure( void );
extern void abs_pressure_minmax( double * pres_min, double * pres_max, struct timestamp * time_min, struct timestamp * time_max );
extern void pressure_reset( char minmax );
extern double pressure_correction( void );
extern void tendency_forecast( int * tendency, int * forecast );
extern void light( int set );

// data recalculated
extern double GetRelPressure( void );
//  data pressed into one structure (get it using get_weatherdata_ptr())
extern void ReadData( void );


#endif  // __WS23K_H__
