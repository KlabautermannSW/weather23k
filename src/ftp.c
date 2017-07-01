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
#include "debug.h"
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
    debug("RNFR %s\n", ftp_file());

    if( strlen(ftp_server()) == 0 )
        return ERR_NO_FTP_SERVER;

    sprintf(remote_url, "ftp://%s%s", ftp_server(), ftp_file());
    debug("ftp://%s%s\n", ftp_server(), ftp_file());

    sprintf(name_pass, "%s:%s", user_name(), user_key());
    debug("Set user name and key : %s %s\n", user_name(), user_key());

    fsize = (curl_off_t)strlen(ftp_string());                                   // get the number of bytes for transfer
    the_ftp_string_ptr = ftp_string();  

    curl = curl_easy_init();                                                    // get a curl handle
    if( !curl )
        {
        error = ERR_CURL_EASY_INIERRNOOR;
        goto end_PushFile;
        }   
    debug("Curl initialized\n");

    headerlist = curl_slist_append(headerlist, buf_1);                          // build a list of commands to pass to libcurl
    if( !headerlist )
        {
        error = ERR_CURL_HEADERLISERRNOOR;
        goto cleanup_PushFile;
        }
    debug("Headerlist set\n");

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
    debug("Options set\n");

    /* Set the size of the file to upload (optional). If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    if( curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize) )
        goto setopt_PushFile;
    debug("Filesize set set\n");

    error = NOERR;
    if( curl_easy_perform(curl) )                                               // Now run off and do what you've been told!
        error = ERR_CURL_PERFORM_ERROR;
    debug("Curl performed\n");

setopt_PushFile:
    curl_slist_free_all(headerlist);                                            // clean up the FTP commands list
    debug("Curl freed\n");
cleanup_PushFile:
    curl_easy_cleanup(curl);                                                    // always cleanup
    debug("Curl cleaned up\n");
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

    if( line == 0 )
        return ERR_NO_LOG_DATA;

    sprintf(buf_1, "RNFR %s", logfile);
    debug("RNFR %s\n", logfile);
    if( strlen(ftp_server()) == 0 )
        return ERR_NO_FTP_SERVER;
    sprintf(remote_url, "ftp://%s%s", ftp_server(), logfile);
    debug("ftp://%s%s\n", ftp_server(), logfile);
    sprintf(name_pass, "%s:%s", user_name(), user_key());
    debug("Set user name and key : %s %s\n", user_name(), user_key());

    fsize = (curl_off_t)strlen(line);                                           // get the number of bytes for transfer
    the_ftp_string_ptr = line;  
    debug("ftp string : %s, len %d\n", the_ftp_string_ptr, (int)fsize);

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();                                                    // get a curl handle
    if( !curl )
        {
        error = ERR_CURL_EASY_INIERRNOOR;
        goto end_AppendFile;
        }
    debug("Curl initialized\n");

    headerlist = curl_slist_append(headerlist, buf_1);                          // build a list of commands to pass to libcurl
    if( !headerlist )
        {
        error = ERR_CURL_HEADERLISERRNOOR;
        goto cleanup_AppendFile;
        }
    debug("Headerlist set\n");
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
    debug("Options set\n");

    /* Set the size of the file to upload (optional). If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    if( curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize) )
        goto setopt_AppendFile;
    debug("Filesize set set\n");

    error = NOERR;
    if( (res = curl_easy_perform(curl)) )                                       // Now run off and do what you've been told!
        {
        debug("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        error = ERR_CURL_PERFORM_ERROR;
        }
    debug("Curl performed\n");

setopt_AppendFile:
    curl_slist_free_all(headerlist);                                            // clean up the FTP commands list
    debug("Curl freed\n");
cleanup_AppendFile:
    curl_easy_cleanup(curl);                                                    // always cleanup
    debug("Curl esy cleaned up\n");
    curl_global_cleanup();
    debug("Curl global cleaned up\n");
end_AppendFile:
    return error;
    }


/*  function        ERRNO FtpInit( void )

    brief           does the global init of the curl lib

    return          ERRNO
*/
ERRNO FtpInit( void )
    {
    debug("Initialize curl\n");
    return ( curl_global_init(CURL_GLOBAL_ALL) ) ? ERR_CURL_INIERRNOOR : NOERR;
    }


/*  function        void FtpCleanup( void )

    brief           does the global cleanup of the curl lib
*/
void FtpCleanup( void )
    {
    curl_global_cleanup();
    }
