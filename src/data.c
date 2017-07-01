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

    file        data.c

    date        18.06.2017

    author      Uwe Jantzen (jantzen@klabautermann-software.de)

    brief       Read and store data from .ini-file
                Store global data

    details     

    project     weather23k
    target      Linux
    begin       05.09.2015

    note        

    todo        

*/


#include "data.h"
#include "ws23k.h"
#include "password.h"
#include <string.h>
#include <malloc.h>


//    type codes for _GetEntry(.)
#define    EMPTY                        0
#define    SECTION                      1
#define    KEY_VAL                      2


struct _tokens
    {
    char * p_str;
    int length;
    struct _tokens * next;
    };

static const char * the_variables[] =
    {
    "temp",                                                                     // temperature
    "press",                                                                    // relative air pressure
    "hum",                                                                      // relative air humidity
    "winddir",                                                                  // wind direction
    "speed_m",                                                                  // wind speed [m/sec]
    "speed_kmh",                                                                // wind speed [kmh]
    "speed_kn",                                                                 // wind speed [kn]
    "speed_bf",                                                                 // wind speed [bft]
    "dew",                                                                      // dewpoint temperature
    "chill",                                                                    // windchill temperature
    "rph",                                                                      // rain per hour
    "rpd",                                                                      // rain per day
    "dirstr",                                                                   // wind direction as text
    "time"                                                                      // current time stamp
    };
static const int max_var_length[] =                                             // maximum string lenght if variable is printed 
    { 0, 6, 6, 3, 3, 4, 5, 5, 2, 6, 6, 6, 6, 3, 11 };

static const char * the_default_init_file_name = "conf/weatherman.conf";
static const char * the_default_com_port = "/dev/ttyS0";

static char the_verbose_flag = 0;
static char the_debug_flag = 0;
static char the_com_port[128];
static char the_log_path[128];
static char the_ftp_server[256];
static char the_user_name[128];
static char the_key[MAX_PASSWORD_LENGTH];
static char the_ftp_file[128] = { 0, };
static char * the_init_file_name = 0;

static struct _tokens * the_p_token_list = 0;
static struct _tokens * the_p_token = 0;

static char * the_ftp_string = 0;


/*  function        void set_verbose( char set )

    brief           set the verbosity

    param[in]       char set, 0 : quiet, other : verbose
*/
void set_verbose( char set )
    {
    the_verbose_flag = set;
    }


/*  function        char verbose( void )

    brief           get the verbosity

    return          char, 0 : quiet, other : verbose
*/
char verbose( void )
    {
    return the_verbose_flag;
    }


/*  function        void set_debug( char set )

    brief           set the verbosity

    param[in]       char set, 0 : quiet, other : show debug output
*/
void set_debug( char set )
    {
    the_debug_flag = set;
    }


/*  function        char is_debug( void )

    brief           get the verbosity

    return          char, 0 : quiet, other : show debug output
*/
char is_debug( void )
    {
    return the_debug_flag;
    }


/*  function        void set_ini_file( char * ini_file_name )

    brief           set the initialization file name into a global variable

    param[in]       char * ini_file_name
*/
void set_ini_file( char * ini_file_name )
    {
    the_init_file_name = ini_file_name;
    }


/*  function        char * com_port( void )

    brief           return the serial port's name string

    return          char *, pointer to serial port's name string
*/
char * com_port( void )
    {
    return the_com_port;
    }


/*  function        char * log_path( void )

    brief           returns a pointer to the path for the log files

    return          char *, pointer to the log file path string
*/
char * log_path( void )
    {
    return the_log_path;
    }


/*  function        char * ftp_server( void )

    brief           returns a pointer to the ftp server name  string

    return          char *, pointer to the ftp server name  string
*/
char * ftp_server( void )
    {
    return the_ftp_server;
    }


/*  function        char * user_name( void )

    brief           returns a pointer to the user name string

    return          char *, pointer to the user name string
*/
char * user_name( void )
    {
    return the_user_name;
    }


/*  function        char * user_key( void )

    brief           returns a pointer to the users' key string

    return          char *, pointer to the users' key string
*/
char * user_key( void )
    {
    return the_key;
    }


/*  function        char * ftp_string( void )

    brief           returns a pointer to the string that is to transfer over the
                    ftp conection

    return          char *, pointer to the string to transfer
*/
char * ftp_string( void )
    {
    return the_ftp_string;
    }


/*  function        char * ftp_file( void )

    brief           returns a pointer to the name of the file to handle
                    on the ftp server

    return          char *, pointer to the file on the ftp server
*/
char * ftp_file( void )
    {
    return the_ftp_file;
    }


/*  function        static void _add_token( char * p_str, int len )

    brief           collects a token to the token array

    param[in]       char * p_str, pointer to token
                                  that is an int pointer if token is a variable !
    param[in]       int len, string length or 0 if token is a variable
*/
static void _add_token( char * p_str, int len )
    {
    if( the_p_token )
        {
        the_p_token->next = malloc(sizeof(struct _tokens));
        the_p_token = the_p_token->next;
        }
    else
        {
        the_p_token = malloc(sizeof(struct _tokens));
        the_p_token_list = the_p_token;                                         // set the anchor
        }
    the_p_token->p_str = p_str;
    the_p_token->length = len;
    the_p_token->next = 0;
    }


/*  function        char * get_first_token( int * len )

    brief           returns a pointer to the string residing as first token

    param[in]       int * len, string length or 0 if token is a variable

    return          char *, pointer to token
                            this is an int pointer if token is a variable ! 
*/
char * get_first_token( int * len )
    {
    *len = the_p_token_list->length;

    the_p_token = the_p_token_list->next;

    return the_p_token_list->p_str;
    }


/*  function        char * get_next_token( int * len )

    brief           returns a pointer to the string residing as current token
                    in the array

    param[in]       int * len, string length or 0 if token is a variable

    return          char *, pointer to token
                            this is an int pointer if token is a variable ! 
*/
char * get_next_token( int * len )
    {
    char * p_str = 0;

    if( the_p_token )
        {
        p_str = the_p_token->p_str;
        *len = the_p_token->length;
        the_p_token = the_p_token->next;
        }
    return p_str;
    }


/*  function        ERRNO Remove( char * str, char chr )

    brief           Remove all occurences of chr in p_str.
                    Only if no errors occured the contents of p_str is changed.

    param[in]       char * p_str, pointer to a string closed with a 0x00
    param[in]       char chr, character to remove from string

    return          ERRNO, error code
*/
ERRNO Remove( char * str, char chr )
    {
    char * buffer;
    char * p_buffer;
    char * p_str;
    size_t len;
    size_t i;

    if( !str )
        return ERR_ILLEGAL_STRING_PTR;

    len = strlen(str) + 1;
    if( len == 1 )
        return NOERR;

    buffer = (char *)malloc(len);
    if( !buffer )
        return ERR_OUT_OF_MEMORY;

    memset(buffer, 0, len);
    p_str = str;
    p_buffer = buffer;
    for( i=0; i<len; ++i )
        {
        if( *p_str != chr )
            {
            *p_buffer = *p_str;
            ++p_buffer;
            }
        ++p_str;
        }

    strcpy(str, buffer);

    free(buffer);

    return NOERR;
    }


/*  function        ERRNO _GetEntry( FILE * p_file, char * p_section, char * p_key, char * p_val )

    brief           Reads a line from an .ini file.
                    Returns the name of the section which contains the actual key,
                    the actual key name and the key's value.
                    Additionally the result value is set accordingly to a possible
                    error condition.
                    Only if NOERR the strings contains a legal vaule!

    param[in]       FILE * p_file, pointer to ini file 
    param[out]      char * p_section, pointer to section name
    param[out]      char * p_key, pointer to key name
    param[out]      char * p_val, pointer to key value

    return          ERRNO, error code
*/
ERRNO _GetEntry( FILE * p_file, char * p_section, char * p_key, char * p_val )
    {
    char buffer[1024];
    char * p_str;
    int type;
    int len;
            
    type = EMPTY;
    if( !p_file )
        {
        if( the_debug_flag )
            printf("data.c _GetEntry : NO FILE\n");
        return ERR_NO_INIFILE;
        }

    do
        {
        if( !fgets(buffer, 1020, p_file) )                                      // read one line from file
            {
            if( feof(p_file) )
                {
                if( the_debug_flag )
                    printf("data.c _GetEntry : END OF FILE\n");
                return ERR_EOF;
                }
            else
                {
                if( the_debug_flag )
                    printf("data.c _GetEntry : UNKNOWN ERROR\n");
                return ERR_UNKNOWN;
                }
            }
        if( *buffer == '#' )                                                    // comment line
            {
            if( the_debug_flag )
                printf("data.c _GetEntry : COMMENT\n");
            return NOERR;
            }

        if( (*buffer == '\n') || (*buffer == '\r') )                            // empty line
            {
            if( the_debug_flag )
                printf("data.c _GetEntry : NEWLINE\n");
            return NOERR;
            }

        if( *buffer == '[' )                                                    // begin of section identifier
            {
            type = SECTION;
            strcpy(p_section, buffer);
            Remove(p_section, 0x0d);
            Remove(p_section, 0x0a);
            Remove(p_section, ' ');
            Remove(p_section, '[');
            Remove(p_section, ']');
            if( the_debug_flag )
                printf("data.c _GetEntry : SECTION %s\n", p_section);
            if( strcmp(p_section, "Template") == 0 )
                return NOERR;
            }
        else
            {
            // from here on this must be a key
            type = KEY_VAL;
            strcpy(p_key, buffer);
            Remove(p_key, ' ');
            len = (int)(strchr(p_key, '=') - p_key);
            if( len == 0 )                                                      // if equal the "=" is missing
                {
                type = EMPTY;
                if( the_debug_flag )
                    printf("data.c _GetEntry : ILLEGAL KEY LINE %s\n", p_key);
                return ERR_ILLEGAL_KEYLINE;
                }
            *(p_key+len) = 0x00;                                                // get key (everything in front of "=")
            strcpy(p_val, buffer);
            p_str = p_val;
            Remove(p_val, ' ');
            while( *(p_val+len+1) != 0x00 )
                {
                *p_val = *(p_val+len+1);
                ++p_val;
                }
            *p_val = 0x00;
            p_val = p_str;
            Remove(p_val, 0x0d);
            Remove(p_val, 0x0a);
            Remove(p_val, '"');
        if( the_debug_flag )
            printf("data.c _GetEntry :   SECTION = %s, KEY %s = %s\n", p_section, p_key, p_val);
            }
        }
    while( type != KEY_VAL );

    return NOERR;
    }


/*  function        ERRNO Init( void )

    brief           initializes the global viarables from the data from the
                    .ini file

    return          ERRNO, initialization error or success
*/
ERRNO Init( void )
    {
    FILE * p_inifile;
    ERRNO error = NOERR;
    char section[80];
    char key[80];
    char val[3*MAX_PASSWORD_LENGTH];
    long i;
    long template_len;
    char * p_template_buffer = 0;
    char * p_buffer;
    char * p_token;
    char * p_str;
    int ftp_str_length = 0;
    int l;
    int j;    

    p_template_buffer = 0;                                                      // initialize to prevent memory access errors
    template_len = 0;

    /* initialize the strings */
    strncpy(the_com_port, the_default_com_port, 127);
    the_com_port[127] = 0;                                                      // always terminate the string
    *the_log_path = 0;                                                          // empty string
    *the_ftp_server = 0;                                                        // empty string
    *the_user_name = 0;                                                         // empty string
    *the_key = 0;                                                               // empty string
    *the_ftp_file = 0;                                                          // empty string

    if( !the_init_file_name )
        the_init_file_name = (char *)the_default_init_file_name;

    if( the_verbose_flag )
        printf("Reading configuration from %s\n", the_init_file_name);
 
    p_inifile = fopen(the_init_file_name, "r");
    if( !p_inifile )                                                            // no ini file found
        return ERR_NO_INIFILE;

    while( _GetEntry(p_inifile, section, key, val ) == NOERR )
        {
/*
        always use strncpy() for the strings at here and put a terminating 0 at the end of the buiffer !
        NOT DONE YET !!!!!
*/
        if( (strcmp(section, "FTP") == 0) && (strcmp(key, "server") == 0) )
            strcpy(the_ftp_server, val);
        else if( (strcmp(section, "FTP") == 0) && (strcmp(key, "user") == 0) )
            strcpy(the_user_name, val);
        else if( (strcmp(section, "FTP") == 0) && (strcmp(key, "key") == 0) )
            decode(val, the_key);
        else if( (strcmp(section, "FTP") == 0) && (strcmp(key, "file") == 0) )
            strcpy(the_ftp_file, val);
        else if( (strcmp(section, "File") == 0) && (strcmp(key, "logpath") == 0) )
            strcpy(the_log_path, val);
        else if( (strcmp(section, "Port") == 0) && (strcmp(key, "port") == 0) )
            strcpy(the_com_port, val);
        else if( strcmp(section, "Template") == 0 )
            {                                                                   // now get the template
            i = ftell(p_inifile);
            fseek(p_inifile, 0, SEEK_END);
            template_len = ftell(p_inifile) - i;
            if( template_len <= 0 )
                {
                error = NOERR;                                                  // no template
                goto end_Init;
                }
            fseek(p_inifile, i, SEEK_SET);
            p_template_buffer = malloc(template_len + 1);                       // get space for template
            if( !p_template_buffer )
                {
                error = ERR_NOT_ENOUGH_MEMORY;
                goto end_Init;
                }

            p_buffer = p_template_buffer;
            while( !feof(p_inifile) )                                           // read file
                {
                *p_buffer = fgetc(p_inifile);
                ++p_buffer;
                }
            *p_buffer = 0;                                                      // make it a string
            break;                                                              // the template is the last section
            }
        key[0] = 0;
        }

    // if we come here we hav a template so divide it to tokens containing texts and variables
    p_buffer = p_template_buffer;
    while( (p_buffer - p_template_buffer) < template_len )
        {
        p_token = strstr(p_buffer, "<*");
        if( p_token != p_buffer )                                               // there is a text
            {
            if( !p_token )                                                      // this is the last text
                l = template_len - (p_buffer - p_template_buffer);
            else
                l = p_token - p_buffer;
            p_str = malloc(l+1);                                                // beware for the trailing 0
            if( !p_str )
                {
                error = ERR_NOT_ENOUGH_MEMORY;
                goto end_Init;
                }
            memcpy(p_str, p_buffer, l);
            *(p_str + l) = 0;
            if( !p_token )                                                      // this is the last text
                p_buffer = p_template_buffer + template_len;
            else
                p_buffer = p_token;
            ftp_str_length += l;                                                // collect the string length
            }
        else                                                                    // this is a token
            {
            l = 0;
            p_str = malloc(sizeof(int));                                        // reserve space for an int
            if( !p_str )
                {
                error = ERR_NOT_ENOUGH_MEMORY;
                goto end_Init;
                }
            *(int *)p_str = VAR_UNKNOWN;
            p_token += 6;                                                       // point to variable's name
            for( j=0; j<VAR_NUM_OF_VARS; ++j )                                  // identify variable
                {
                if( strncmp(p_token, the_variables[j], strlen(the_variables[j])) == 0)
                    {                                                           // identifier found
                    *(int *)p_str = j+1;
                    break;
                    }
                }
            p_buffer = strstr(p_token, "*>") + 2;                               // set buffer pointer behind the variable
            ftp_str_length += max_var_length[j];                                // collect the string length
            }
        _add_token(p_str, l);
        }
    the_ftp_string = malloc(ftp_str_length + 1);                                // beware for the trailing 0
    if( !the_ftp_string )
        error = ERR_NOT_ENOUGH_MEMORY;

end_Init:                                                                       // error exit
    free(p_template_buffer);
    fclose(p_inifile);
    return error;
    }


/*  function        void DeInit( void )

    brief           closes the connenction to the weather station and
                    releases all allocated memory
*/
void DeInit( void )
    {
    struct _tokens * p_token;

    the_p_token = the_p_token_list;
    while( the_p_token )
        {
        p_token = the_p_token;
        the_p_token = the_p_token->next;
        free(p_token->p_str);
        free(p_token);
        }
    free(the_ftp_string);
    }


/*  function        ERRNO PrintVariable( int var, char * dst )

    brief           adds the formatted variable value to the end of the string

    param[in]       int var, index to variable
    param[out]      char * dst, string to print in

    return          ERRNO
*/
ERRNO PrintVariable( int var, char * dst )
    {
    weatherdata_t * p_weatherdata;
    char var_str[20];

    if( (var > VAR_NUM_OF_VARS) || (var == VAR_UNKNOWN) )
        return ERR_VAR_UNKNOWN;

    if( !dst )
        return ERR_ILLEGAL_STRING_PTR;

    p_weatherdata = get_weatherdata_ptr();
    
    switch( var )
        {
        case VAR_TEMP :
            sprintf(var_str, "%1.2f", p_weatherdata->temperature);
            break;
        case VAR_PRESS :
            sprintf(var_str, "%1.1f", GetRelPressure());
            break;
        case VAR_HUM :
            sprintf(var_str, "%d", p_weatherdata->humidity);
            break;
        case VAR_WINDDIR :
            if( p_weatherdata->sensor_connected == 0 )
                sprintf(var_str, "%1.1f", p_weatherdata->direction);
            else
                sprintf(var_str, "-.-");
            break;
        case VAR_SPEED_M :
            if( p_weatherdata->sensor_connected == 0 )
                sprintf(var_str, "%1.1f", p_weatherdata->speed[0]);
            else
                sprintf(var_str, "-.-");
            break;
        case VAR_SPEED_KMH :
            if( p_weatherdata->sensor_connected == 0 )
                sprintf(var_str, "%1.1f", p_weatherdata->speed[1]);
            else
                sprintf(var_str, "-.-");
            break;
        case VAR_SPEED_KN :
            if( p_weatherdata->sensor_connected == 0 )
                sprintf(var_str, "%1.1f", p_weatherdata->speed[2]);
            else
                sprintf(var_str, "-.-");
            break;
        case VAR_SPEED_BF :
            if( p_weatherdata->sensor_connected == 0 )
                sprintf(var_str, "%d", (int)p_weatherdata->speed[3]);
            else
                sprintf(var_str, "-");
            break;
        case VAR_DEW :
            sprintf(var_str, "%1.2f", p_weatherdata->dewpoint);
            break;
        case VAR_CHILL :
            if( p_weatherdata->sensor_connected == 0 )
                sprintf(var_str, "%1.2f", p_weatherdata->windchill);
            else
                sprintf(var_str, "-.-");
            break;
        case VAR_RPH :
            sprintf(var_str, "%1.1f", p_weatherdata->rain_per_hour);
            break;
        case VAR_RPD :
            sprintf(var_str, "%1.1f", p_weatherdata->rain_per_day);
            break;
        case VAR_DIRSTR     :
            if( p_weatherdata->sensor_connected == 0 )
                sprintf(var_str, "%s", p_weatherdata->dir);
            else
                sprintf(var_str, "---");
            break;
        case VAR_TIME :
            sprintf(var_str, "%s", p_weatherdata->act_time);
            break;
        default :
            break;
        }

    strcat(dst, var_str);
    return NOERR;
    }


/*  function        void SetFtpString( void )

    brief           collects all data to be sent to the ftp seerver
*/
void SetFtpString( void )
    {
    char * str;
    int len;
    
    *the_ftp_string = 0;                                                        // start from scratch every time
    str = get_first_token(&len);
    
    do
        {
        if( len == 0 )                                                          // variable
            {
            PrintVariable((int)*str, the_ftp_string);
            }
        else
            {
            strcat(the_ftp_string, str);
            }
        
        str = get_next_token(&len);
        }
    while( str );
    }
