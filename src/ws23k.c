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

    file        ws23k.c

    date        22.10.2017

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       Handle all access to WS2300 weather station
                Handle some data conversion

    details 

    project     weather23k
    target      Linux
    begin       05.09.2015

    note    

    todo        

*/


#include "debug.h"
#include "data.h"
#include "ws23kcom.h"
#include "ws23k.h"
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>


#define WRITE_NIBBBLE                           0x42
#define MAXWINDRETRIES                          20


static weatherdata_t the_weatherdata;


/*  function        weatherdata_t * get_weatherdata_ptr( void )

    brief           returns tointer to the global weatherdata structue

    return          weatherdata_t     *, pointer to the global weatherdata structue
*/
weatherdata_t * get_weatherdata_ptr( void )
    {
    return &the_weatherdata;
    }


/*  function        double GetRelPressure( void )

    brief           calculates the relativer air pressure from the absolute pressure and temperature
                    from the weather station and a giver height

    return          double, relative pressure
*/
double GetRelPressure( void )
    {
    const double G0 = 9.80665;
    const double R_STAR = 287.05;
    const double A = 0.0065;
    const double CH = 0.12;
    const double KELVIN = 273.15;
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


/*  function        static double temperature_indoor( void )

    brief           Read current indoor temperature

    return          double, teperature [°C]
*/
double temperature_indoor( void )
    {
    uint8_t data[2];
    int address = 0x346;
    int bytes = 2;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (((data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0);
    }


/*  function        void temperature_indoor_minmax( double * temp_min, double * temp_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read indoor minimum and maximum temperatures with timestamps

    param[out]      double * temp_min
    param[out]      double * temp_max
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max
*/
void temperature_indoor_minmax( double * temp_min, double * temp_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[15];
    int address = 0x34B;
    int bytes = 15;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *temp_min = ((data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4)/10.0 + (data[0] & 0xF) / 100.0) - 30.0;
    *temp_max = ((data[4] & 0xF) * 10 + (data[3] >> 4) + (data[3] & 0xF)/10.0 + (data[2] >> 4) / 100.0) - 30.0;

    time_min->minute = ((data[5] & 0xF) * 10) + (data[4] >> 4);
    time_min->hour = ((data[6] & 0xF) * 10) + (data[5] >> 4);
    time_min->day = ((data[7] & 0xF) * 10) + (data[6] >> 4);
    time_min->month = ((data[8] & 0xF) * 10) + (data[7] >> 4);
    time_min->year = 2000 + ((data[9] & 0xF) * 10) + (data[8] >> 4);
    
    time_max->minute = ((data[10] & 0xF) * 10) + (data[9] >> 4);
    time_max->hour = ((data[11] & 0xF) * 10) + (data[10] >> 4);
    time_max->day = ((data[12] & 0xF) * 10) + (data[11] >> 4);
    time_max->month = ((data[13] & 0xF) * 10) + (data[12] >> 4);
    time_max->year = 2000 + ((data[14] & 0xF) * 10) + (data[13] >> 4);
    }


/*  function        void temperature_indoor_reset( uint8_t minmax )

    brief           Reset indoor minimum and maximum temperatures with timestamp

    param[in]       uint8_t minmax, bit field to control which temperature entry is reseted
*/
void temperature_indoor_reset( uint8_t minmax )
    {
    uint8_t data_read[6];
    uint8_t data_value[4];
    uint8_t data_time[10];
    int address;
    int number;

    // first read current temperature into data_value
    address = 0x346;
    number = 2;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_value[0] = data_read[0] & 0xF;
    data_value[1] = data_read[0] >> 4;
    data_value[2] = data_read[1] & 0xF;
    data_value[3] = data_read[1] >> 4;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    if( minmax & RESET_MIN )
        {
        // Set min value to current value
        address = 0x34B;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set min value timestamp to current time
        address = 0x354;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }

    if( minmax & RESET_MAX )
        {
        // Set max value to current value
        address = 0x350;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set max value timestamp to current time
        address = 0x35E;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }
    }


/*  function        double temperature_outdoor( void )

    brief           Read current outdoor temperature

    return          double, teperature [°C]
*/
double temperature_outdoor( void )
    {
    uint8_t data[2];
    int address = 0x373;
    int bytes = 2;

debug("+%s \n", __func__);
    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

debug("-%s \n", __func__);
    return (((data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0);
    }


/*  function        void temperature_outdoor_minmax( double * temp_min, double * temp_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read outdoor minimum and maximum temperatures with timestamps

    param[out]      double * temp_min
    param[out]      double * temp_max
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max
*/
void temperature_outdoor_minmax( double * temp_min, double * temp_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[15];
    int address = 0x378;
    int bytes = 15;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *temp_min = ((data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4)/10.0 + (data[0] & 0xF) / 100.0) - 30.0;
    *temp_max = ((data[4] & 0xF) * 10 + (data[3] >> 4) + (data[3] & 0xF)/10.0 + (data[2] >> 4) / 100.0) - 30.0;

    time_min->minute = ((data[5] & 0xF) * 10) + (data[4] >> 4);
    time_min->hour = ((data[6] & 0xF) * 10) + (data[5] >> 4);
    time_min->day = ((data[7] & 0xF) * 10) + (data[6] >> 4);
    time_min->month = ((data[8] & 0xF) * 10) + (data[7] >> 4);
    time_min->year = 2000 + ((data[9] & 0xF) * 10) + (data[8] >> 4);

    time_max->minute = ((data[10] & 0xF) * 10) + (data[9] >> 4);
    time_max->hour = ((data[11] & 0xF) * 10) + (data[10] >> 4);
    time_max->day = ((data[12] & 0xF) * 10) + (data[11] >> 4);
    time_max->month = ((data[13] & 0xF) * 10) + (data[12] >> 4);
    time_max->year = 2000 + ((data[14] & 0xF) * 10) + (data[13] >> 4);
    }


/*  function        void temperature_outdoor_reset( uint8_t minmax )

    brief           Reset outdoor minimum and maximum temperatures with timestamp

    param[in]       uint8_t minmax, bit field to control which temperature entry is reseted
*/
void temperature_outdoor_reset( uint8_t minmax )
    {
    uint8_t data_read[6];
    uint8_t data_value[4];
    uint8_t data_time[10];
    int address;
    int number;

    // First read current temperature into data_value
    address = 0x373;
    number = 2;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_value[0] = data_read[0] & 0xF;
    data_value[1] = data_read[0] >> 4;
    data_value[2] = data_read[1] & 0xF;
    data_value[3] = data_read[1] >> 4;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);
    
    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    if( minmax & RESET_MIN )
        {
        // Set min value to current value
        address = 0x378;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set min value timestamp to current time
        address = 0x381;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }

    if( minmax & RESET_MAX )
        {
        // Set max value to current value
        address = 0x37D;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set max value timestamp to current time
        address = 0x38B;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }
    }


/*  function        double dewpoint( void )

    brief           Read current dewpoint

    return          double dewpoint
*/
double dewpoint( void )
    {
    uint8_t data[2];
    int address = 0x3CE;
    int bytes = 2;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (((data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0);
    }


/*  function        void dewpoint_minmax( double * temp_min, double * temp_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read outdoor minimum and maximum dewpoint with timestamps

    param[out]      double * dp_min
    param[out]      double * dp_max
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max
*/
void dewpoint_minmax( double * dp_min, double * dp_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[15];
    int address = 0x3D3;
    int bytes = 15;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *dp_min = ((data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4)/10.0 + (data[0] & 0xF) / 100.0) - 30.0;
    *dp_max = ((data[4] & 0xF) * 10 + (data[3] >> 4) + (data[3] & 0xF)/10.0 + (data[2] >> 4) / 100.0) - 30.0;

    time_min->minute = ((data[5] & 0xF) * 10) + (data[4] >> 4);
    time_min->hour = ((data[6] & 0xF) * 10) + (data[5] >> 4);
    time_min->day = ((data[7] & 0xF) * 10) + (data[6] >> 4);
    time_min->month = ((data[8] & 0xF) * 10) + (data[7] >> 4);
    time_min->year = 2000 + ((data[9] & 0xF) * 10) + (data[8] >> 4);

    time_max->minute = ((data[10] & 0xF) * 10) + (data[9] >> 4);
    time_max->hour = ((data[11] & 0xF) * 10) + (data[10] >> 4);
    time_max->day = ((data[12] & 0xF) * 10) + (data[11] >> 4);
    time_max->month = ((data[13] & 0xF) * 10) + (data[12] >> 4);
    time_max->year = 2000 + ((data[14] & 0xF) * 10) + (data[13] >> 4);
    }


/*  function        void dewpoint_reset( uint8_t minmax )

    brief           Reset outdoor minimum and maximum dewpoints with timestamp

    param[in]       uint8_t minmax, bit field to control which dewpoint entry is reseted
*/
void dewpoint_reset( uint8_t minmax )
    {
    uint8_t data_read[6];
    uint8_t data_value[4];
    uint8_t data_time[10];
    int address;
    int number;

    // First read current dewpoint into data_value
    address = 0x3CE;
    number = 2;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_value[0] = data_read[0] & 0xF;
    data_value[1] = data_read[0] >> 4;
    data_value[2] = data_read[1] & 0xF;
    data_value[3] = data_read[1] >> 4;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);
    
    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    if( minmax & RESET_MIN )
        {
        // Set min value to current value
        address = 0x3D3;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set min value timestamp to current time
        address = 0x3DC;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }

    if( minmax & RESET_MAX )
        {
            // Set max value to current value
        address = 0x3D8;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set max value timestamp to current time
        address = 0x3E6;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }
    }


/*  function        int humidity_indoor( void )

    brief           Read current indoor relative humidity

    return          int, relative humidity [%]
*/
int humidity_indoor( void )
    {
    uint8_t data;
    int address = 0x3FB;
    int bytes = 1;

    if( read_data(&data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (data >> 4) * 10 + (data & 0xF);
    }


/*  function        int humidity_indoor_all( int * hum_min, int * hum_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read both current indoor humidity and minimum and maximum values with timestamps

    param[out]      double * hum_min [%]
    param[out]      double * hum_max [%]
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max

    return          int, relative humidity [%]
*/
int humidity_indoor_all( int * hum_min, int * hum_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[13];
    int address = 0x3FB;
    int bytes = 13;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *hum_min = (data[1] >> 4) * 10 + (data[1] & 0xF);
    *hum_max = (data[2] >> 4) * 10 + (data[2] & 0xF);

    time_min->minute = ((data[3] >> 4) * 10) + (data[3] & 0xF);
    time_min->hour = ((data[4] >> 4) * 10) + (data[4] & 0xF);
    time_min->day = ((data[5] >> 4) * 10) + (data[5] & 0xF);
    time_min->month = ((data[6] >> 4) * 10) + (data[6] & 0xF);
    time_min->year = 2000 + ((data[7] >> 4) * 10) + (data[7] & 0xF);

    time_max->minute = ((data[8] >> 4) * 10) + (data[8] & 0xF);
    time_max->hour = ((data[9] >> 4) * 10) + (data[9] & 0xF);
    time_max->day = ((data[10] >> 4) * 10) + (data[10] & 0xF);
    time_max->month = ((data[11] >> 4) * 10) + (data[11] & 0xF);
    time_max->year = 2000 + ((data[12] >> 4) * 10) + (data[12] & 0xF);

    return (data[0] >> 4) * 10 + (data[0] & 0xF);
    }


/*  function        void humidity_indoor_reset( uint8_t minmax )

    brief           Reset indoor minimum and maximum humidity with timestamp

    param[in]       uint8_t minmax, bit field to control which humidity entry is reseted
*/
void humidity_indoorr_reset( uint8_t minmax )
    {
    uint8_t data_read[6];
    uint8_t data_value[4];
    uint8_t data_time[10];
    int address;
    int number;

    // First read current humidity into data_value
    address = 0x3FB;
    number = 1;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_value[0] = data_read[0] & 0xF;
    data_value[1] = data_read[0] >> 4;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);
    
    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    if( minmax & RESET_MIN )
        {
        // Set min value to current value
        address = 0x3FD;
        number = 2;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set min value timestamp to current time
        address = 0x401;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }

    if( minmax & RESET_MAX )
        {
        // Set max value to current value
        address = 0x3FF;
        number = 2;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set max value timestamp to current time
        address = 0x40B;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);    
        }
    }


/*  function        int humidity_outdoor( void )

    brief           Read current outdoor relative humidity

    return          int, relative humidity [%]
*/
int humidity_outdoor( void )
    {
    uint8_t data[20];
    int address = 0x419;
    int bytes = 1;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (data[0] >> 4) * 10 + (data[0] & 0xF);
    }


/*  function        int humidity_outdoor_all( int * hum_min, int * hum_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read both current outdoor humidity and minimum and maximum values with timestamps

    param[out]      double * hum_min [%]
    param[out]      double * hum_max [%]
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max

    return          int, relative humidity [%]
*/
int humidity_outdoor_all( int * hum_min, int * hum_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[13];
    int address = 0x419;
    int bytes = 13;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *hum_min = (data[1] >> 4) * 10 + (data[1] & 0xF);
    *hum_max = (data[2] >> 4) * 10 + (data[2] & 0xF);

    time_min->minute = ((data[3] >> 4) * 10) + (data[3] & 0xF);
    time_min->hour = ((data[4] >> 4) * 10) + (data[4] & 0xF);
    time_min->day = ((data[5] >> 4) * 10) + (data[5] & 0xF);
    time_min->month = ((data[6] >> 4) * 10) + (data[6] & 0xF);
    time_min->year = 2000 + ((data[7] >> 4) * 10) + (data[7] & 0xF);

    time_max->minute = ((data[8] >> 4) * 10) + (data[8] & 0xF);
    time_max->hour = ((data[9] >> 4) * 10) + (data[9] & 0xF);
    time_max->day = ((data[10] >> 4) * 10) + (data[10] & 0xF);
    time_max->month = ((data[11] >> 4) * 10) + (data[11] & 0xF);
    time_max->year = 2000 + ((data[12] >> 4) * 10) + (data[12] & 0xF);

    return (data[0] >> 4) * 10 + (data[0] & 0xF);
    }


/*  function        void humidity_outdoor_reset( uint8_t minmax )

    brief           Reset outdoor minimum and maximum humidiy with timestamp

    param[in]       uint8_t minmax, bit field to control which humidity entry is reseted
*/
void humidity_outdoor_reset( uint8_t minmax )
    {
    uint8_t data_read[6];
    uint8_t data_value[4];
    uint8_t data_time[10];
    int address;
    int number;

    // First read current humidity into data_value
    address = 0x419;
    number = 1;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_value[0] = data_read[0] & 0xF;
    data_value[1] = data_read[0] >> 4;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);
    
    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    if( minmax & RESET_MIN )
        {
        // Set min value to current value
        address = 0x41B;
        number = 2;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set min value timestamp to current time
        address = 0x41F;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }

    if( minmax & RESET_MAX )
        {
        // Set max value to current value
        address = 0x41D;
        number = 2;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set max value timestamp to current time
        address = 0x429;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }
    }


/*  function        double wind_current( double * winddir )

    brief           Read wind speed and wind direction

    param[out]      double * winddir [°]

    return          double, wind speed [m/s]
*/
double wind_current( double * winddir )
    {
    int i;
    uint8_t data[3];
    int address = 0x527;                                                        // windspeed and direction
    int bytes = 3;

    for( i = 0; i < MAXWINDRETRIES; ++i )
        {
        if( read_data(data, address, bytes) != bytes )                          // wind
            handle_comm_error(ERR_COMM_READ);
    
        if( ( data[0] != 0x00 ) ||                                              // invalid wind data
            ( ( data[1] == 0xFF ) && ( ((data[2] & 0xF) == 0 ) || ( (data[2] & 0xF) == 1 ) ) ) )
            {
            usleep(10000); //wait 10 seconds for new wind measurement
            continue;
            }
        else
            break;
        }

    *winddir = (data[2] >> 4) * 22.5;

    // calculate raw wind speed
    return (((data[2] & 0xF) << 8) + data[1]) / 10.0;
    }


/*  function        double wind_current_flags( double * winddir, int * sensor_connected, int * minimum_code )

    brief           Read wind speed, wind direction, sensor flags, minimum code

    param[out]      double * winddir [°]
    param[out]      int * sensor_connected, flag : 0 = normal, 5 = sensor disconnencted 
    param[out]      int * minimum_code

    return          double, wind speed [m/s]
*/
double wind_current_flags( double * winddir, int * sensor_connected, int * minimum_code )
    {
    uint8_t data[3];
    int i;
    int address = 0x527;                                                        // windspeed and direction
    int bytes = 3;

    for( i = 0; i < MAXWINDRETRIES; ++i )
        {
        if( read_data(data, address, bytes) != bytes )                          // wind
            handle_comm_error(ERR_COMM_READ);
    
        if( ( data[1] == 0xFF ) && ( ( (data[2] & 0xF) == 0 ) || ( (data[2] & 0xF) == 1 )) )
            {
            usleep(10000);                                                      // wait 10 seconds for new wind measurement
            continue;
            }
        else
            break;
        }

    *winddir = (data[2] >> 4) * 22.5;
    *sensor_connected = data[0] & 0x0F;
    *minimum_code = (data[0] >> 4) & 0x0F;

    return (((data[2] & 0xF) << 8) + data[1]) / 10.0;
    }


/*  function        double wind_all( int * winddir_index, double * winddir )

    brief           Read wind speed, wind direction and last 5 wind directions

    param[out]      int * winddir_index,
                        Current wind direction expressed as ticks from North
                        where North=0. Used to convert to direction string
    param[out]      double * winddir,
                        Array of doubles containing current wind direction
                        in winddir[0] and the last 5 in the following
                        positions all given in degrees

    return          double, wind speed [m/s]
*/
double wind_all( int * winddir_index, double * winddir )
    {
    uint8_t data[6];
    int i;
    int address = 0x527; //Windspeed and direction
    int bytes = 6;

    for( i = 0; i < MAXWINDRETRIES; ++i )
        {
        if( read_data(data, address, bytes) != bytes )                          // wind
            handle_comm_error(ERR_COMM_READ);
         
        if( ( data[0]!=0x00 ) ||                                                // invalid wind data
           ( ( data[1] == 0xFF ) && ( ((data[2] & 0xF) == 0 )||( (data[2] & 0xF) == 1 ) ) ) )
            {
            usleep(10000);                                                      // wait 10 seconds for new wind measurement
            continue;
            }
        else
            break;
        }

    *winddir_index = data[2] >> 4;
    winddir[0] = (data[2] >> 4) * 22.5;
    winddir[1] = (data[3] & 0xF) * 22.5;
    winddir[2] = (data[3] >> 4) * 22.5;
    winddir[3] = (data[4] & 0xF) * 22.5;
    winddir[4] = (data[4] >> 4) * 22.5;
    winddir[5] = (data[5] & 0xF) * 22.5;

    return (((data[2] & 0xF) << 8) + data[1]) / 10.0;
    }


/*  function        double wind_minmax( double * wind_min, double * wind_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read minimum and maximum wind speeds with timestamps
                    If a pointer is 0 the corresponding value will be ignored

    param[out]      double * wind_min [m/s]
    param[out]      double * wind_max [m/s]
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max

    return          double, maximum wind speed [m/s]
*/
double wind_minmax( double * wind_min, double * wind_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[15];
    int address = 0x4EE;
    int bytes = 15;

    if( read_data(data, address, bytes) != bytes )
            handle_comm_error(ERR_COMM_READ);

    if( wind_min )
        *wind_min = (data[1] * 256 + data[0]) / 360.0;
    if( wind_max )
        *wind_max = (data[4] * 256 + data[3]) / 360.0;

    if( time_min )
        {
        time_min->minute = ((data[5] >> 4) * 10) + (data[5] & 0xF);
        time_min->hour = ((data[6] >> 4) * 10) + (data[6] & 0xF);
        time_min->day = ((data[7] >> 4) * 10) + (data[7] & 0xF);
        time_min->month = ((data[8] >> 4) * 10) + (data[8] & 0xF);
        time_min->year = 2000 + ((data[9] >> 4) * 10) + (data[9] & 0xF);
        }

    if( time_max )
        {
        time_max->minute = ((data[10] >> 4) * 10) + (data[10] & 0xF);
        time_max->hour = ((data[11] >> 4) * 10) + (data[11] & 0xF);
        time_max->day = ((data[12] >> 4) * 10) + (data[12] & 0xF);
        time_max->month = ((data[13] >> 4) * 10) + (data[13] & 0xF);
        time_max->year = 2000 + ((data[14] >> 4) * 10) + (data[14] & 0xF);
        }

    return (data[4] * 256 + data[3]) / 360.0;
    }


/*  function        void wind_reset( uint8_t minmax )

    brief           Reset minimum and/or maximum wind with timestamps depending
                    on the bits set in "minmax"

    param[in]       uint8_t minmax, bit field
*/
void wind_reset( uint8_t minmax )
    {
    int i;
    uint8_t data_read[6];
    uint8_t data_value[6];
    uint8_t data_time[10];
    int address;
    int number;
    int current_wind;

    address = 0x527;                                                            // windspeed
    number = 3;

    for( i = 0; i < MAXWINDRETRIES; ++i )
        {
        if( read_data(data_read, address, number) != number )
            handle_comm_error(ERR_COMM_READ);
         
        if( ( data_read[0] != 0x00 ) ||                                         // invalid wind data
            ( ( data_read[1] == 0xFF ) && ( ( (data_read[2] & 0xF) == 0 ) || ( (data_read[2] & 0xF) == 1 ) ) ) )
            {
            usleep(10000);                                                      // wait 10 seconds for new wind measurement
            continue;
            }
        else
            break;
        }

    current_wind = (((data_read[2] & 0xF) << 8) + data_read[1]) * 36;

    data_value[0] = current_wind&0xF;
    data_value[1] = (current_wind>>4) & 0xF;
    data_value[2] = (current_wind>>8) & 0xF;
    data_value[3] = (current_wind>>12) & 0xF;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    if( minmax & RESET_MIN )
        {
        // Set min value to current value
        address = 0x4EE;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set min value timestamp to current time
        address = 0x4F8;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }

    if( minmax & RESET_MAX )
        {
        // Set max value to current value
        address = 0x4F4;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set max value timestamp to current time
        address = 0x502;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);    
        }
    }


/*  function        double windchill( void )

    brief           Read current wind chill temperature

    return          double, current wind chill temperature
*/
double windchill( void )
    {
    uint8_t data[2];
    int address = 0x3A0;
    int bytes = 2;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (((data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0);
    }


/*  function        void windchill_minmax( double * wc_min, double * wc_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read minimum and maximum wind chill with timestamps

    param[out]      double * wc_min [m/s]
    param[out]      double * wc_max [m/s]
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max
*/
void windchill_minmax( double * wc_min, double * wc_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[15];
    int address = 0x3A5;
    int bytes = 15;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *wc_min = ((data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0 ) - 30.0;
    *wc_max = ((data[4] & 0xF) * 10 + (data[3] >> 4) + (data[3] & 0xF) / 10.0 + (data[2] >> 4) / 100.0 ) - 30.0;

    time_min->minute = ((data[5] & 0xF) * 10) + (data[4] >> 4);
    time_min->hour = ((data[6] & 0xF) * 10) + (data[5] >> 4);
    time_min->day = ((data[7] & 0xF) * 10) + (data[6] >> 4);
    time_min->month = ((data[8] & 0xF) * 10) + (data[7] >> 4);
    time_min->year = 2000 + ((data[9] & 0xF) * 10) + (data[8] >> 4);

    time_max->minute = ((data[10] & 0xF) * 10) + (data[9] >> 4);
    time_max->hour = ((data[11] & 0xF) * 10) + (data[10] >> 4);
    time_max->day = ((data[12] & 0xF) * 10) + (data[11] >> 4);
    time_max->month = ((data[13] & 0xF) * 10) + (data[12] >> 4);
    time_max->year = 2000 + ((data[14] & 0xF) * 10) + (data[13] >> 4);
    }


/*  function        void windchill_reset( uint8_t minmax )

    brief           Reset minimum and/or maximum windchill with timestamps depending
                    on the bits set in "minmax"

    param[in]       uint8_t minmax, bit field to control which windchill entry is reseted
*/
void windchill_reset( uint8_t minmax )
    {
    uint8_t data_read[6];
    uint8_t data_value[4];
    uint8_t data_time[10];
    int address;
    int number;

    // First read current windchill into data_value
    address = 0x3A0;
    number = 2;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_value[0] = data_read[0] & 0xF;
    data_value[1] = data_read[0] >> 4;
    data_value[2] = data_read[1] & 0xF;
    data_value[3] = data_read[1] >> 4;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);
    
    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    if( minmax & RESET_MIN )
        {
        // Set min value to current value
        address = 0x3A5;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set min value timestamp to current time
        address = 0x3AE;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }

    if( minmax & RESET_MAX )
        {
        // Set max value to current value
        address = 0x3AA;
        number = 4;
    
        if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set max value timestamp to current time
        address = 0x3B8;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }
    }


/*  function        double rain_1h( void )

    brief           Rain fallen in the last hour, current value

    return          double
*/
double rain_1h( void )
    {
    uint8_t data[3];
    int address = 0x4B4;
    int bytes = 3;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 + (data[1] >> 4) * 10 +
           (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0;
    }


/*  function        double rain_1h_all( double * rain_max, struct timestamp * time_max )

    brief           Current read rain of last 1 hour and 1h rain maximum with timestamp

    param[out]      double * rain_max
    param[out]      struct timestamp * time_max

    return          double
*/
double rain_1h_all( double * rain_max, struct timestamp * time_max )
    {
    uint8_t data[11];
    int address = 0x4B4;
    int bytes = 11;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);
    
    *rain_max = (data[5] >> 4) * 1000 + (data[5] & 0xF) * 100 + (data[4] >> 4) * 10 +
                (data[4] & 0xF) + (data[3] >> 4) / 10.0 + (data[3] & 0xF) / 100.0;
             
    time_max->minute = ((data[6] >> 4) * 10) + (data[6] & 0xF);
    time_max->hour = ((data[7] >> 4) * 10) + (data[7] & 0xF);
    time_max->day = ((data[8] >> 4) * 10) + (data[8] & 0xF);
    time_max->month = ((data[9] >> 4) * 10) + (data[9] & 0xF);
    time_max->year = 2000 + ((data[10] >> 4) * 10) + (data[10] & 0xF);

    return (data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 + (data[1] >> 4) * 10 +
           (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0;
    }


/*  function        void rain_1h_max_reset( void )

    brief           Reset max rain 1h with timestamps
*/
void rain_1h_max_reset( void )
    {
    uint8_t data_read[6];
    uint8_t data_value[6];
    uint8_t data_time[10];
    int address;
    int number;

    // First read current rain 1h into data_value
    address = 0x4B4;
    number = 3;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_value[0] = data_read[0] & 0xF;
    data_value[1] = data_read[0] >> 4;
    data_value[2] = data_read[1] & 0xF;
    data_value[3] = data_read[1] >> 4;
    data_value[4] = data_read[2] & 0xF;
    data_value[5] = data_read[2] >> 4;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    // Set max value to current value
    address = 0x4BA;
    number = 6;

    if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);

    // Set max value timestamp to current time
    address = 0x4C0;
    number = 10;

    if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);
    }


/*  function        void rain_1h_reset( void )

    brief           Clear current rain 1h
*/
void rain_1h_reset( void )
    {
    uint8_t data[30];
    int address;
    int number;

    // First overwrite the 1h rain history with zeros
    address = 0x479;
    number = 30;
    memset(&data, 0, sizeof(data));

    if( write_data(data, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);

    // Set value to zero
    address = 0x4B4;
    number = 6;

    if( write_data(data, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);
    }


/*  function        double rain_24h( void )

    brief           Rain fallen in the 24 hours, current value

    return          double
*/
double rain_24h( void )
    {
    uint8_t data[3];
    int address = 0x497;
    int bytes = 3;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 + (data[1] >> 4) * 10 +
           (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0;
    }


/*  function        double rain_24h_all( double * rain_max, struct timestamp * time_max )

    brief           Current read rain of last 24 hours and 1h rain maximum with timestamp

    param[out]      double * rain_max
    param[out]      struct timestamp * time_max

    return          double
*/
double rain_24h_all( double * rain_max, struct timestamp * time_max )
    {
    uint8_t data[11];
    int address = 0x497;
    int bytes = 11;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *rain_max = (data[5] >> 4) * 1000 + (data[5] & 0xF) * 100 + (data[4] >> 4) * 10 +
                (data[4] & 0xF) + (data[3] >> 4)/10.0 + (data[3] & 0xF) / 100.0;

    time_max->minute = ((data[6] >> 4) * 10) + (data[6] & 0xF);
    time_max->hour = ((data[7] >> 4) * 10) + (data[7] & 0xF);
    time_max->day = ((data[8] >> 4) * 10) + (data[8] & 0xF);
    time_max->month = ((data[9] >> 4) * 10) + (data[9] & 0xF);
    time_max->year = 2000 + ((data[10] >> 4) * 10) + (data[10] & 0xF);

    return (data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 + (data[1] >> 4) * 10 +
           (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0;
    }


/*  function        void rain_24h_max_reset( void )

    brief           Reset max rain 1h with timestamps
*/
void rain_24h_max_reset( void )
    {
    uint8_t data_read[6];
    uint8_t data_value[6];
    uint8_t data_time[10];
    int address;
    int number;

    // First read current rain 24h into data_value
    address = 0x497;
    number = 3;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_value[0] = data_read[0] & 0xF;
    data_value[1] = data_read[0] >> 4;
    data_value[2] = data_read[1] & 0xF;
    data_value[3] = data_read[1] >> 4;
    data_value[4] = data_read[2] & 0xF;
    data_value[5] = data_read[2] >> 4;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);

    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    // Set max value to current value
    address = 0x49D;
    number = 6;

    if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);

    // Set max value timestamp to current time
    address = 0x4A3;
    number = 10;

    if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);
    }


/*  function        void rain_24h_reset( void )

    brief           Clear current rain 1h
*/
void rain_24h_reset( void )
    {
    uint8_t data[30];
    int address;
    int number;

    // First overwrite the 24h rain history with zeros
    address = 0x446;
    number = 48;
    memset(&data, 0, sizeof(data));

    if( write_data(data, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);

    // Set value to zero
    address = 0x497;
    number = 6;

    if( write_data(data, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);
    }


/*  function        double rain_total( void )

    brief           Read the current accumulated total rain

    return          double
*/
double rain_total( void )
    {
    uint8_t data[3];
    int address = 0x4D2;
    int bytes = 3;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 + (data[1] >> 4) * 10 +
           (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0;
    }


/*  function        double rain_total_all( struct timestamp *time_since )

    brief           Read the current accumulated total rain with timestamp

    param[out]      struct timestamp * time_since

    return          double
*/
double rain_total_all( struct timestamp * time_since )
    {
    uint8_t data[8];
    int address = 0x4D2;
    int bytes = 8;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    time_since->minute = ((data[3] >> 4) * 10) + (data[3] & 0xF);
    time_since->hour = ((data[4] >> 4) * 10) + (data[4] & 0xF);
    time_since->day = ((data[5] >> 4) * 10) + (data[5] & 0xF);
    time_since->month = ((data[6] >> 4) * 10) + (data[6] & 0xF);
    time_since->year = 2000 + ((data[7] >> 4) * 10) + (data[7] & 0xF);

    return (data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 + (data[1] >> 4) * 10 +
           (data[1] & 0xF) + (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0;
    }


/*  function        void rain_total_reset( void )

    brief           Reset total rain value
*/
void rain_total_reset( void )
    {
    uint8_t data_read[6];
    uint8_t data_value[7];
    uint8_t data_time[10];
    int address;
    int number;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);
    
    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    // Set value to zero
    address = 0x4D1;
    number = 7;
    memset(&data_value, 0, sizeof(data_value));

    if( write_data(data_value, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);

    // Set max value timestamp to current time
    address = 0x4D8;
    number = 10;

    if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
        handle_comm_error(ERR_COMM_WRITE);
    }


/*  function        double rel_pressure( void )

    brief           Read current relative air pressure

    return          double
*/
double rel_pressure( void )
    {
    uint8_t data[3];
    int address = 0x5E2;
    int bytes = 3;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (data[2] & 0xF) * 1000 + (data[1] >> 4) * 100 + (data[1] & 0xF) * 10 +
           (data[0] >> 4) + (data[0] & 0xF) / 10.0;
    }


/*  function        void rel_pressure_minmax( double * pres_min, double * pres_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read relative pressure minimum and maximum with timestamps

    param[out]      double * pres_min
    param[out]      double * pres_max
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max
*/
void rel_pressure_minmax( double * pres_min, double * pres_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[13];
    int address = 0x600;
    int bytes = 13;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *pres_min = (data[2] & 0xF) * 1000 + (data[1] >> 4) * 100 + (data[1] & 0xF) * 10 +
                (data[0] >> 4) + (data[0] & 0xF) / 10.0;
    *pres_max = (data[12] & 0xF) * 1000 + (data[11] >> 4) * 100 + (data[11] & 0xF) * 10 +
                (data[10] >> 4) + (data[10] & 0xF) / 10.0;
    
    address = 0x61E;                                                            // relative pressure time and date for min/max
    bytes = 10;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    time_min->minute = ((data[0] >> 4) * 10) + (data[0] & 0xF);
    time_min->hour = ((data[1] >> 4) * 10) + (data[1] & 0xF);
    time_min->day = ((data[2] >> 4) * 10) + (data[2] & 0xF);
    time_min->month = ((data[3] >> 4) * 10) + (data[3] & 0xF);
    time_min->year = 2000 + ((data[4] >> 4) * 10) + (data[4] & 0xF);

    time_max->minute = ((data[5] >> 4) * 10) + (data[5] & 0xF);
    time_max->hour = ((data[6] >> 4) * 10) + (data[6] & 0xF);
    time_max->day = ((data[7] >> 4) * 10) + (data[7] & 0xF);
    time_max->month = ((data[8] >> 4) * 10) + (data[8] & 0xF);
    time_max->year = 2000 + ((data[9] >> 4) * 10) + (data[9] & 0xF);
    }


/*  function        double abs_pressure( void )

    brief           Read current absolute air pressure

    return          double
*/
double abs_pressure( void )
    {
    uint8_t data[3];
    int address = 0x5D8;
    int bytes = 3;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (data[2] & 0xF) * 1000 + (data[1] >> 4) * 100 + (data[1] & 0xF) * 10 +
           (data[0] >> 4) + (data[0] & 0xF) / 10.0;
    }


/*  function        void abs_pressure_minmax( double * pres_min, double * pres_max, struct timestamp * time_min, struct timestamp * time_max )

    brief           Read absolute pressure minimum and maximum with timestamps

    param[out]      double * pres_min
    param[out]      double * pres_max
    param[out]      struct timestamp * time_min
    param[out]      struct timestamp * time_max
*/
void abs_pressure_minmax( double * pres_min, double * pres_max, struct timestamp * time_min, struct timestamp * time_max )
    {
    uint8_t data[13];
    int address = 0x5F6;
    int bytes = 13;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *pres_min = (data[2] & 0xF) * 1000 + (data[1] >> 4) * 100 + (data[1] & 0xF) * 10 +
                (data[0] >> 4) + (data[0] & 0xF) / 10.0;
    *pres_max = (data[12] & 0xF) * 1000 + (data[11] >> 4) * 100 + (data[11] & 0xF) * 10 +
                (data[10] >> 4) + (data[10] & 0xF) / 10.0;
    
    address = 0x61E; //Relative pressure time and date for min/max
    bytes = 10;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    time_min->minute = ((data[0] >> 4) * 10) + (data[0] & 0xF);
    time_min->hour = ((data[1] >> 4) * 10) + (data[1] & 0xF);
    time_min->day = ((data[2] >> 4) * 10) + (data[2] & 0xF);
    time_min->month = ((data[3] >> 4) * 10) + (data[3] & 0xF);
    time_min->year = 2000 + ((data[4] >> 4) * 10) + (data[4] & 0xF);

    time_max->minute = ((data[5] >> 4) * 10) + (data[5] & 0xF);
    time_max->hour = ((data[6] >> 4) * 10) + (data[6] & 0xF);
    time_max->day = ((data[7] >> 4) * 10) + (data[7] & 0xF);
    time_max->month = ((data[8] >> 4) * 10) + (data[8] & 0xF);
    time_max->year = 2000 + ((data[9] >> 4) * 10) + (data[9] & 0xF);
    }


/*  function        void pressure_reset( uint8_t minmax )

    brief           Reset minimum and/or maximum pressure with timestamps depending
                    on the bits set in "minmax"

    param[in]       uint8_t minmax, bit field to control which pressure entry is reseted
*/
void pressure_reset( char minmax )
    {
    uint8_t data_read[8];
    uint8_t data_value_abs[5];
    uint8_t data_value_rel[5];
    uint8_t data_time[10];
    int address;
    int number;

    // First read current abs/rel pressure into data_value_abs/rel
    address = 0x5D8;
    number = 8;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);
    
    data_value_abs[0] = data_read[0] & 0xF;
    data_value_abs[1] = data_read[0] >> 4;
    data_value_abs[2] = data_read[1] & 0xF;
    data_value_abs[3] = data_read[1] >> 4;
    data_value_abs[4] = data_read[2] & 0xF;

    data_value_rel[0] = data_read[5] & 0xF;
    data_value_rel[1] = data_read[5] >> 4;
    data_value_rel[2] = data_read[6] & 0xF;
    data_value_rel[3] = data_read[6] >> 4;
    data_value_rel[4] = data_read[7] & 0xF;

    // Get current time from station
    address = 0x23B;
    number = 6;

    if( read_data(data_read, address, number) != number )
        handle_comm_error(ERR_COMM_READ);
    
    data_time[0] = data_read[0] & 0xF;
    data_time[1] = data_read[0] >> 4;
    data_time[2] = data_read[1] & 0xF;
    data_time[3] = data_read[1] >> 4;
    data_time[4] = data_read[2] >> 4;
    data_time[5] = data_read[3] & 0xF;
    data_time[6] = data_read[3] >> 4;
    data_time[7] = data_read[4] & 0xF;
    data_time[8] = data_read[4] >> 4;
    data_time[9] = data_read[5] & 0xF;

    if( minmax & RESET_MIN )
        {
        // Set min abs value to current abs value
        address = 0x5F6;
        number = 5;
    
        if( write_data(data_value_abs, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        
        // Set min rel value to current rel value
        address = 0x600;
        number = 5;
    
        if( write_data(data_value_rel, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set min value timestamp to current time
        address = 0x61E;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        }

    if( minmax & RESET_MAX )
        {
        // Set max abs value to current abs value
        address = 0x60A;
        number = 5;
    
        if( write_data(data_value_abs, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);
        
        // Set max rel value to current rel value
        address = 0x614;
        number = 5;
    
        if( write_data(data_value_rel, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);

        // Set max value timestamp to current time
        address = 0x628;
        number = 10;

        if( write_data(data_time, address, number, WRITE_NIBBBLE) != number )
            handle_comm_error(ERR_COMM_WRITE);    
        }
    }


/*  function        double pressure_correction( void )

    brief           Read the correction from absolute to relaive air pressure

    return          double, correction factor
*/
double pressure_correction( void )
    {
    uint8_t data[3];
    int address = 0x5EC;
    int bytes = 3;

    if( read_data(data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    return (data[2] & 0xF) * 1000 + (data[1] >> 4) * 100 + (data[1] & 0xF) * 10 +
           (data[0] >> 4) + (data[0] & 0xF) / 10.0 - 1000;
    }


/*  function        void tendency_forecast( int * tendency, int * forecast )

    brief           Read pressure tendency and weather forecast

    param[out]      int * tendency, 0 = steady, 1 = rising, 2 = falling
    param[out]      int * forecast, 0 = rainy, 1 = cloudy, 2 = sunny
*/
void tendency_forecast( int * tendency, int * forecast )
    {
    uint8_t data;
    int address = 0x26B;
    int bytes = 1;

    if( read_data(&data, address, bytes) != bytes )
        handle_comm_error(ERR_COMM_READ);

    *tendency = data >> 4;
    *forecast = data & 0xF;
    }


/*  function        void light( int set )

    brief           Turns display light on and off

    param[in]       int set, boolean value : 0 = off, else = on
*/
void light( int set )
    {
    uint8_t data;
    int address = 0x016;

    data = 0;
    
    if( write_data(&data, address, 1, (set) ? BIT_SET : BIT_CLEAR) != 1 )
        handle_comm_error(ERR_COMM_WRITE);
    }


/*  function        void ReadData( void )

    brief           reads data from the connected weather station and saves them
                    to the global weather data structure
                    the part commented out by NIX puts sample data to the global
                    weather data structure
*/
void ReadData( void )
    {
debug("+%s \n", __func__);
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

debug(" %s temperature_outdoor()\n", __func__);
    the_weatherdata.temperature = temperature_outdoor();         // outdoor temperature
    nanosleep(&ts, 0);
debug(" %s temperature_indoor()\n", __func__);
    the_weatherdata.temperature_in = temperature_indoor();       // indoor temperature
    nanosleep(&ts, 0);
debug(" %s humidity_outdoor()\n", __func__);
    the_weatherdata.humidity = humidity_outdoor();
    nanosleep(&ts, 0);
debug(" %s humidity_indoor()\n", __func__);
    the_weatherdata.humidity_in = humidity_indoor();
    nanosleep(&ts, 0);
debug(" %s dewpoint()\n", __func__);
    the_weatherdata.dewpoint = dewpoint();
    nanosleep(&ts, 0);
debug(" %s wind_current_flags()\n", __func__);
    the_weatherdata.speed[0] = wind_current_flags(&the_weatherdata.direction, &the_weatherdata.sensor_connected, &minimum_code);
    nanosleep(&ts, 0);
    the_weatherdata.speed[1] = the_weatherdata.speed[0] * KMH;
    the_weatherdata.speed[2] = the_weatherdata.speed[0] * KNOTS;
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
debug(" %s rain_1h()\n", __func__);
    the_weatherdata.rain_per_hour = rain_1h();                                  // mm or l/qm
    nanosleep(&ts, 0);
debug(" %s rain_24h()\n", __func__);
    the_weatherdata.rain_per_day = rain_24h();                                  // mm or l/qm
    nanosleep(&ts, 0);
debug(" %s abs_pressure()\n", __func__);
    the_weatherdata.pressure = abs_pressure();
    nanosleep(&ts, 0);
debug(" %s windchill()\n", __func__);
    the_weatherdata.windchill = windchill();
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

    the_weatherdata.temperature = 15.5;                                         // outdoor temperature
    the_weatherdata.temperature_in = 17.4;                                      // indoor temperature
    the_weatherdata.humidity = 56;
    the_weatherdata.humidity_in = 32;
    the_weatherdata.dewpoint = 16.3;
    the_weatherdata.speed[0] = 10;
    the_weatherdata.direction = 275.0;
    the_weatherdata.sensor_connected = 0;
    the_weatherdata.speed[1] = the_weatherdata.speed[0] * KMH;
    the_weatherdata.speed[2] = the_weatherdata.speed[0] * KNOTS;
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
    the_weatherdata.rain_per_hour = 0.0;                                        // mm or l/qm
    the_weatherdata.rain_per_day = 0;                                           // mm or l/qm
    the_weatherdata.pressure = 1005.0;
    the_weatherdata.windchill = 3.8;
#endif  // NIX
debug(" %s \n", __func__);

    if( verbose() )
        {
        printf("data read\n");
        fflush(stdout);
        }
debug("-%s \n", __func__);
    }
