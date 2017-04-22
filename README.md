# weather23k
Weather data flow from WS2300 weather station to a web page.

To make and use this application you must have installed libcurl and
libcurl-devel on your system. 


Files :

.git                        the hidden git directory

bin/weather23k              the application

conf/weatherman.conf        sample configuration file
conf/weatherman.conf_org    sample configuration file
conf/weatherman.conf_SFT    sample configuration file
conf/weatherman.conf_temp   sample configuration file

include/data.h
inlcude/errors.h
inlcude/ftp.h
inlcude/getargs.h
inlcude/log.h
inlcude/password.h
inlcude/raw23k.h~
inlcude/sercom.h
inlcude/ws23kcom.h
inlcude/ws23k.h

doc/prerequisites.txt		what you need to create and use this project

obj/*                       object files created by comlining

php/                        sample php scripts to show data on the web page
php/humid.php               graphic displaying the outdoor humitity
php/relpress.php            graphic displaying the outdoor air pressure (relative) 
php/temperatures.php        graphic displaying the outdoor temperature
php/windchill.php           graphic displaying the windchill temperature
php/winddir.php		        graphic displaying the wind direction
php/windspeed.php           graphic displaying the wind speed

src/data.c
src/errors.c
src/ftp.c
src/ftpupload.c
src/getargs.c
src/log.c
src/password.c
src/sercom.c
src/var_args.c
src/weather23k.c
src/ws23k.c
src/ws23kcom.c

.gitignore                  the git ignore rules
LICENSE                     the license description
Makefile                    make : compiles and links the whole project
						    make clean : removes object files and bluefish backup
                                         files but not the application
README.MD                   this documentation
