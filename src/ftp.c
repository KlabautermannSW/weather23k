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

    file        ftp.c

    date        15.10.2016

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       establish a ftp connection to the internet server
                push the weather data to a file on the server
                kill the connection

    details     

    project     weather23k
    target      Linux
    begin       16.12.2015

    note        

    todo        

*/


#include "ftp.h"
#include "data.h"
#include <curl/curl.h>
#include <string.h>


static char * the_ftp_string_ptr = 0;


/*  function        static size_t _copy( char * dst, size_t n )

    brief           copies a part of the string that is to be sent over the ftp connection
                    to the send buffer

    param[in]       char * dst, send buffer
    param[in]       size_t n, number of bytes to copy from src to dst

    return          int, number of bytes copied
*/
static size_t _copy( char * dst, size_t n )
    {
    size_t res = strlen(the_ftp_string_ptr);

    if( res > n )
        res = n;

    memcpy(dst, the_ftp_string_ptr, res);
    the_ftp_string_ptr += res;
    return res;
    }


/*  function        static size_t _read_callback( void * ptr, size_t size, size_t nmemb, void * stream )

    brief           copies a part of the string that is to be sent over the ftp connection
                    to the send buffer

    param[in]       void * ptr
    param[in]       size_t size
    param[in]       size_t nmemb,
    param[in]       void * stream,

    return          size_t, number of bytes transferred
*/
static size_t _read_callback( void * ptr, size_t size, size_t nmemb, void * stream )
    {
    size_t retcode = _copy(ptr, size*nmemb);
    return retcode;
    }


/*  function        ERRNO PushFile( void )

    brief           opens a ftp connection and transfers the ftp string to the
                    given file on the server

    return          ERRNO
*/
ERRNO PushFile( void )
    {
    ERRNO error = NOERR;
    CURL * curl = 0;
    curl_off_t fsize;
    struct curl_slist * headerlist = 0;
    char buf_1[148];
    char remote_url[420];
    char name_pass[272]; 

    sprintf(buf_1, "RNFR %s", ftp_file());
    if( debug() )
        {
        printf("RNFR %s\n", ftp_file());
        fflush(stdout);
        }
    sprintf(remote_url, "ftp://%s%s", ftp_server(), ftp_file());
    if( debug() )
        {
        printf("ftp://%s%s\n", ftp_server(), ftp_file());
        fflush(stdout);
        }
    sprintf(name_pass, "%s:%s", user_name(), user_key());
    if( debug() )
        {
        printf("Set user name and key\n");
        fflush(stdout);
        }

    fsize = (curl_off_t)strlen(ftp_string());                                   // get the number of bytes for transfer
    the_ftp_string_ptr = ftp_string();  

    curl = curl_easy_init();                                                    // get a curl handle
    if( !curl )
        {
        error = ERR_CURL_EASY_INIERRNOOR;
        goto end_PushFile;
        }
    if( debug() )
        {
        printf("Curl initialized\n");
        fflush(stdout);
        }

    headerlist = curl_slist_append(headerlist, buf_1);                          // build a list of commands to pass to libcurl
    if( !headerlist )
        {
        error = ERR_CURL_HEADERLISERRNOOR;
        goto cleanup_PushFile;
        }
    if( debug() )
        {
        printf("Headerlist set\n");
        fflush(stdout);
        }
    error = ERR_CURL_SETOPERRNOOR;
    if( curl_easy_setopt(curl, CURLOPT_READFUNCTION, _read_callback) )          // we want to use our own read function
        goto setopt_PushFile;
    if( curl_easy_setopt(curl, CURLOPT_UPLOAD, 1) )                             // enable uploading
        goto setopt_PushFile;
    if( curl_easy_setopt(curl,CURLOPT_URL, remote_url) )                        // specify target
        goto setopt_PushFile;
    if( curl_easy_setopt(curl, CURLOPT_USERPWD, name_pass) )                    // set user and password
        goto setopt_PushFile;
    if( curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist) )                 // pass in that last of FTP commands to run after the transfer
        goto setopt_PushFile;
    if( curl_easy_setopt(curl, CURLOPT_READDATA, the_ftp_string_ptr) )          // now specify which file to upload
        goto setopt_PushFile;
    if( debug() )
        {
        printf("Options set\n");
        fflush(stdout);
        }

    /* Set the size of the file to upload (optional). If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    if( curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize) )
        goto setopt_PushFile;
    if( debug() )
        {
        printf("Filesize set set\n");
        fflush(stdout);
        }

    error = NOERR;
    if( curl_easy_perform(curl) )                                               // Now run off and do what you've been told!
        error = ERR_CURL_PERFORM_ERROR;
    if( debug() )
        {
        printf("Curl performed\n");
        fflush(stdout);
        }

setopt_PushFile:
    curl_slist_free_all(headerlist);                                            // clean up the FTP commands list
    if( debug() )
        {
        printf("Curl freed\n");
        fflush(stdout);
        }
cleanup_PushFile:
    curl_easy_cleanup(curl);                                                    // always cleanup
    if( debug() )
        {
        printf("Curl cleaned up\n");
        fflush(stdout);
        }
end_PushFile:
    return error;
    }


/*  function        ERRNO AppendFile( char * logfile, char * line )

    brief           opens a ftp connection and transfers the line to the
                    logfile on the server

    param[in]       char * logfile,
    param[in]       char * line

    return          ERRNO
*/
ERRNO AppendFile( char * logfile, char * line )
    {
    ERRNO error = NOERR;
    CURL * curl = 0;
    CURLcode res;
    curl_off_t fsize;
    struct curl_slist * headerlist = 0;
    char buf_1[272];
    char remote_url[540];
    char name_pass[272]; 

    if( line == 0)
        return -1;

    sprintf(buf_1, "RNFR %s", logfile);
    if( debug() )
        {
        printf("RNFR %s\n", logfile);
        fflush(stdout);
        }
    sprintf(remote_url, "ftp://%s%s", ftp_server(), logfile);
    if( debug() )
        {
        printf("ftp://%s%s\n", ftp_server(), logfile);
        fflush(stdout);
        }
    sprintf(name_pass, "%s:%s", user_name(), user_key());
    if( debug() )
        {
        printf("Set user name and key\n");
        fflush(stdout);
        }

    fsize = (curl_off_t)strlen(line);                                           // get the number of bytes for transfer
    the_ftp_string_ptr = line;  
    if( debug() )
        {
        printf("ftp string : %s, len %d\n", the_ftp_string_ptr, (int)fsize);
        fflush(stdout);
        }

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();                                                    // get a curl handle
    if( !curl )
        {
        error = ERR_CURL_EASY_INIERRNOOR;
        goto end_AppendFile;
        }
    if( debug() )
        {
        printf("Curl initialized\n");
        fflush(stdout);
        }

    headerlist = curl_slist_append(headerlist, buf_1);                          // build a list of commands to pass to libcurl
    if( !headerlist )
        {
        error = ERR_CURL_HEADERLISERRNOOR;
        goto cleanup_AppendFile;
        }
    if( debug() )
        {
        printf("Headerlist set\n");
        fflush(stdout);
        }
    error = ERR_CURL_SETOPERRNOOR;
    if( curl_easy_setopt(curl, CURLOPT_READFUNCTION, _read_callback) )          // we want to use our own read function
        goto setopt_AppendFile;
    if( curl_easy_setopt(curl, CURLOPT_APPEND, 1) )                             // enable append instead of overwrite
        goto setopt_AppendFile;
    if( curl_easy_setopt(curl, CURLOPT_UPLOAD, 1) )                             // enable uploading
        goto setopt_AppendFile;
    if( curl_easy_setopt(curl,CURLOPT_URL, remote_url) )                        // specify target
        goto setopt_AppendFile;
    if( curl_easy_setopt(curl, CURLOPT_USERPWD, name_pass) )                    // set user and password
        goto setopt_AppendFile;
    if( curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist) )                 // pass in that last of FTP commands to run after the transfer
        goto setopt_AppendFile;
    if( curl_easy_setopt(curl, CURLOPT_READDATA, the_ftp_string_ptr) )          // now specify which file to upload
        goto setopt_AppendFile;
    if( debug() )
        {
        printf("Options set\n");
        fflush(stdout);
        }

    /* Set the size of the file to upload (optional). If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    if( curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize) )
        goto setopt_AppendFile;
    if( debug() )
        {
        printf("Filesize set set\n");
        fflush(stdout);
        }

    error = NOERR;
    if( (res = curl_easy_perform(curl)) )                                       // Now run off and do what you've been told!
        {
        if( debug() )
            {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            fflush(stdout);
            }
        error = ERR_CURL_PERFORM_ERROR;
        }
    if( debug() )
        {
        printf("Curl performed\n");
        fflush(stdout);
        }

setopt_AppendFile:
    curl_slist_free_all(headerlist);                                            // clean up the FTP commands list
    if( debug() )
        {
        printf("Curl freed\n");
        fflush(stdout);
        }
cleanup_AppendFile:
    curl_easy_cleanup(curl);                                                    // always cleanup
    if( debug() )
        {
        printf("Curl esy cleaned up\n");
        fflush(stdout);
        }
    curl_global_cleanup();
    if( debug() )
        {
        printf("Curl global cleaned up\n");
        fflush(stdout);
        }
end_AppendFile:
    return error;
    }


/*  function        ERRNO FtpInit( void )

    brief           does the global init of the curl lib

    return          ERRNO
*/
ERRNO FtpInit( void )
    {
    return ( curl_global_init(CURL_GLOBAL_ALL) ) ? ERR_CURL_INIERRNOOR : NOERR;
    }


/*  function        void FtpCleanup( void )

    brief           does the global cleanup of the curl lib
*/
void FtpCleanup( void )
    {
    curl_global_cleanup();
    }
