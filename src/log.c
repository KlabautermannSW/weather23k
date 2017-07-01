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

    file        log.c

    date        15.10.2016

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       wait function
                write log file

    details     

    project     weather23k
    target      Linux
    begin       16.12.2015

    note        

    todo        

*/


#include "log.h"
#include "ws23k.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "ftp.h"


/*  function        int WaitForNextMinute( void )

    brief           waits for the next minute to begin

    return          int, 0 if timed out
                        1 if key was pressed (not used now)
*/
int WaitForNextMinute( void )
    {
    time_t t;
    for( ; ; )
        {
        sleep(2);
#ifdef NIX
        if( kbhit() )
            {
            getchar();
            return 1;
            }
#endif    // NIX
        t = time(0) % 60;
        if( (t == 0) || (t == 1) )
            return 0;
        }
    }


/*  function        ERRNO Log( void )

    brief           logs the weather data to the current day's log file

    return          ERRNO
*/
ERRNO Log( void )
    {
    ERRNO error = NOERR;
    time_t basictime;
    char filename[256];
    char curr_date[11];
    char line[1024];
    FILE * logfile;
    weatherdata_t * p_weatherdata = get_weatherdata_ptr();

    sprintf(line, "%s", p_weatherdata->act_time);
    sprintf(line, "%s %6.2f", line, p_weatherdata->temperature);                // temperature [٠C]
    sprintf(line, "%s %6.1f", line, p_weatherdata->pressure);                   // absolute pressure [hPa]
    sprintf(line, "%s %6.1f", line, GetRelPressure());                          // relative pressure [hPa]
    sprintf(line, "%s %3d", line, p_weatherdata->humidity);                     // relative humudity [%]
    sprintf(line, "%s %5.1f", line, p_weatherdata->direction);                  // wind direction [٠]
    sprintf(line, "%s %3s", line, p_weatherdata->dir);                          // wind direction
    sprintf(line, "%s %4.1f", line, p_weatherdata->speed[0]);                   // wind speed [m/sec]
    sprintf(line, "%s %5.1f", line, p_weatherdata->speed[1]);                   // wind speed [km/h]
    sprintf(line, "%s %5.1f", line, p_weatherdata->speed[2]);                   // wind speed [kn]
    sprintf(line, "%s %2d", line, (int)p_weatherdata->speed[3]);                // wind speed [bft]
    sprintf(line, "%s %6.2f", line, p_weatherdata->dewpoint);                   // dewpoint [٠]
    sprintf(line, "%s %6.2f", line, p_weatherdata->windchill);                  // windchill [٠]
    sprintf(line, "%s %5.1f", line, p_weatherdata->rain_per_hour);              // rain_per_hour [l]
    sprintf(line, "%s %5.1f", line, p_weatherdata->rain_per_day);               // rain_per_day [l]
    sprintf(line, "%s\n", line);

    time(&basictime);
    strftime(curr_date, sizeof(curr_date), "%Y_%m_%d", localtime(&basictime));
    sprintf(filename, "%s%sdata.log", log_path(), curr_date);
    logfile = fopen(filename, "a+");
    if( logfile )
        {
        fprintf(logfile, "%s", line);
        fflush(logfile);

        fclose(logfile);
        }

    sprintf(filename, "%s%sdata.log", "/wetter/", curr_date);
    if( (error = AppendFile(filename, line)) != 0 )
        {
        printf("Error logging to server %d\n", error);
        return error;
        }
    
    return error;
    }
