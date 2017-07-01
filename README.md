# weather23k
Weather data flow from WS2300 weather station to a web page.

## Security hints
If you run the program "weather23k" on a computer that is accessible from
outside your local network you should beware of the following :
- do NOT store the source code on the same computer
- implement the functions
    - encode( void )
    - decode( char * p_in, char * p_password )
with your own encoder/decoder algorithm so there will be no plain text
password in the configuration file

## More to know
To make and use this application you must have installed libcurl and
libcurl-devel on your system.

For a files list see doc/filestree.txt
