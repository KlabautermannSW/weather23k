#
#   Copyright (C)
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   Klabautermann Software
#   Uwe Jantzen
#   Weingartener Straße 33
#   76297 Stutensee
#   Germany
#
#   file        Makefile
#
#   date        13.10.2016
#
#   author      Uwe Jantzen (jantzen@klabautermann-software.de)
#
#   brief       Makefile for weather23k
#
#   details     
#
#   project     weather23k
#   target      Linux
#   begin       09.10.2016
#
#   note        
#
#   todo        
#

vpath %.h include
vpath %.c src
vpath %.o obj

CC  = gcc
CC  = gcc
OBJ = weather23k.o sercom.o ws23kcom.o ws23k.o ftp.o getargs.o data.o log.o password.o driver.o errors.o
DSRC = src
DINC = include
DBIN = bin
DOBJ = obj
CONF = conf

VERSION = 1.00

CFLAGS = -I $(DINC) -Wall -O3 -DVERSION=\"$(VERSION)\"
CC_LDFLAGS = -lm

####### Build rules

all: weather23k

weather23k : $(OBJ)
	$(CC) $(CFLAGS) -o $(DBIN)/$@ \
		$(DOBJ)/weather23k.o \
		$(DOBJ)/sercom.o \
		$(DOBJ)/ws23kcom.o \
		$(DOBJ)/ws23k.o \
		$(DOBJ)/ftp.o \
		$(DOBJ)/getargs.o \
		$(DOBJ)/data.o \
		$(DOBJ)/log.o \
		$(DOBJ)/password.o \
		$(DOBJ)/driver.o \
		$(DOBJ)/errors.o \
		-lcurl \
		$(CC_LDFLAGS)

weather23k.o : weather23k.c
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/weather23k.c -o $(DOBJ)/weather23k.o

sercom.o : sercom.c errors.h
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/sercom.c -o $(DOBJ)/sercom.o

ws23kcom.o : ws23kcom.c sercom.h errors.h
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/ws23kcom.c -o $(DOBJ)/ws23kcom.o

ws23k.o : ws23k.c
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/ws23k.c -o $(DOBJ)/ws23k.o

ftp.o : ftp.c
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/ftp.c -o $(DOBJ)/ftp.o

getargs.o : getargs.c
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/getargs.c -o $(DOBJ)/getargs.o

data.o : data.c
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/data.c -o $(DOBJ)/data.o

log.o : log.c
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/log.c -o $(DOBJ)/log.o

password.o : password.c
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/password.c -o $(DOBJ)/password.o

errors.o : errors.c errors.h data.h
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/errors.c -o $(DOBJ)/errors.o

driver.o : driver.c errors.h
	mkdir -p $(DOBJ)
	$(CC) $(CFLAGS) -c $(DSRC)/driver.c -o $(DOBJ)/driver.o

clean:
	rm -v -f $(DSRC)/*~ $(DINC)/*~ $(CONF)/*~ $(DOBJ)/* *~ 
