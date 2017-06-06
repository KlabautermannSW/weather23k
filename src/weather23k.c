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

    file        weather23k.c

    date        13.10.2016

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       this file holds the main program

                get the neccessary data from ini file

                every minute
                    read data from ws2300
                    log data to log file
                    push data to ftp server

    details     

    project     weather23k
    target      Linux
    begin       08.09.2015

    note        

    todo        

*/


#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "data.h"
#include "getargs.h"
#include "ws23k.h"
#include "ftp.h"
#include "log.h"
#include "sercom.h"


/*  function        int main( int argc, char *argv[] )

    brief           main function :
                        reads arguments,
                        prepares weather statio and ftp connection
                        loops
                            reads data from weather station
                            logs data to ftp server
                            prints messages and data to cobnsole if in debug mode

    param[in]       int argc, number of command line parameters
    param[in]       char *argv[], command line parameter list

    return          int, error code
*/
int main( int argc, char *argv[] )
    {
    WEATHERDATA * p_weatherdata;
    char act_time[11];
    time_t basictime;
    ERRNO error = NOERR;
    int i;

    if( argc > 0 )
        {
        for( i=1; i<argc; ++i )
            {
            error = handle_arg(argv[i]);
            if( error )
                printf("Error in command line argument : %s\n", argv[i]);
            }
        }
        
    ws_close();
    sleep(15);

    error = Init();
    if( error )
        {
        printf("General initialization error : %d, programm exiting!\n", error);
        return error;
        }
    error = FtpInit();
    if( error )
        {
        printf("FTP initialization error : %d, programm exiting!\n", error);
        return error;
        }

    i = 0;
    for( ; ; )
        {
        ReadData();
        p_weatherdata = get_weatherdata_ptr();
        if( verbose() )
            {
            printf("Zeit : %s\n", p_weatherdata->act_time);
            printf("Temperatur :          %5.1f °C\n", p_weatherdata->temperature);         // temperature [٠C]
            printf("Luftdruck (abs.) :   %6.1f hPa\n", p_weatherdata->pressure);            // absolute pressure [hPa]
            printf("Luftdruck (rel.) :   %6.1f hPa\n", GetRelPressure());                   // relative pressure [hPa]
            printf("Luftfeuchtigkeit :    %3d %%\n", p_weatherdata->humidity);              // relative humudity [%]
            if( p_weatherdata->sensor_connected == 0 )
                {
                printf("Windrichtung :        %5.1f °\n", p_weatherdata->direction);        // wind direction [٠]
                printf("Windrichtung :        %3s\n", p_weatherdata->dir);                  // wind direction
                printf("Windgeschwindigkeit :  %4.1f m/sec\n", p_weatherdata->speed[0]);    // wind speed [m/sec]
                printf("                      %5.1f km/h\n", p_weatherdata->speed[1]);      // wind speed [km/h]
                printf("                      %5.1f kn\n", p_weatherdata->speed[2]);        // wind speed [kn]
                printf("                       %2d bft\n", (int)p_weatherdata->speed[3]);   // wind speed [bft]
                }
            else
                {
                printf("Windsensor nicht angeschlossen!\n");
                }
            printf("Taupunkt :            %5.1f °C\n", p_weatherdata->dewpoint);            // dewpoint [٠]
            printf("Gefühlte Temp. :      %5.1f °C\n", p_weatherdata->windchill);           // windchill [٠]
            printf("Regen / Stunde :      %5.1f mm\n", p_weatherdata->rain_per_hour);       // rain_per_hour [l]
            printf("Regen / 24 Stunden :  %5.1f mm\n", p_weatherdata->rain_per_day);        // rain_per_day [l]            }
        if( debug() )
            {
            printf("Preparing data string\n");
            fflush(stdout);
            }
        SetFtpString();
#ifndef NIX
        if( p_weatherdata->temperature < 75.0 )
            {
            if( debug() )
                {
                printf("Sending data string\n");
                fflush(stdout);
                }
            error = PushFile();
            if( error )
                {
                printf("FTP error : %d, programm continuing!\n", error);
                }
            }

#endif    // NIX
        if( debug() )
            {
            printf("Logging\n");
            fflush(stdout);
            }
        if( Log() )
            {
            printf("Logging error : %d, programm continuing!\n", error);
            }

        time(&basictime);
        strftime(act_time, sizeof(act_time)-1, "%H:%M:%S", localtime(&basictime));
        act_time[10] = 0;

        if( verbose() )
            {
            printf("%s\n", act_time);
            }

        ws_close();
        sleep(20);
        ws_open();

        WaitForNextMinute();
        }

    printf("\n");

    FtpCleanup();
    DeInit();

    return NOERR;
    }
