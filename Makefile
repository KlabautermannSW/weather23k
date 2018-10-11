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
#   date        18.06.2017
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

CC  := gcc
DSRC := src
DINC := include
DBIN := bin
DOBJ := obj
CONF := conf

OBJ := weather23k.o sercom.o ws23kcom.o ws23k.o ftp.o getargs.o data.o log.o password.o errors.o locals.o debug.o

VERSION = 1.00

CFLAGS = -I $(DINC) -Wall -O3 -DVERSION=\"$(VERSION)\"
CC_LDFLAGS = -lm

.c.o: $(DOBJ)
	$(CC) $(CFLAGS) -c $< -o $(DOBJ)/$@

####### Build rules

all: install weather23k

weather23k : $(OBJ) $(DBIN)
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
		$(DOBJ)/errors.o \
		$(DOBJ)/locals.o \
		$(DOBJ)/debug.o \
		-lcurl \
		$(CC_LDFLAGS)

weather23k.o : weather23k.c data.h getargs.h ws23k.h ftp.h log.h sercom.h debug.h

sercom.o : sercom.c sercom.h errors.h debug.h

ws23kcom.o : ws23kcom.c sercom.h errors.h debug.h

ws23k.o : ws23k.c data.h ws23kcom.h ws23k.h locals.h debug.h

ftp.o : ftp.c ftp.h data.h debug.h

getargs.o : getargs.c data.h password.h getargs.h debug.h

data.o : data.c data.h ws23k.h password.h debug.h

log.o : log.c log.h ws23k.h debug.h

password.o : password.c password.h debug.h

errors.o : errors.c errors.h data.h debug.h

locals.o : locals.c locals.h debug.h

debug.o : debug.c debug.h

####### create object and executable directory if missing
install:
	@if [ ! -d  $(DBIN) ]; then mkdir $(DBIN); fi
	@if [ ! -d  $(DOBJ) ]; then mkdir $(DOBJ); fi

####### cleanup all objects and executables
.PHONY clean:
clean:
	-rm -v -f $(DOBJ)/* $(DBIN)/*
	-rmdir $(DOBJ) $(DBIN)
