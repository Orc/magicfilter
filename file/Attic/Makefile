# Makefile for file(1) cmd. 
# Copyright (c) Ian F. Darwin 86/09/01 - see LEGAL.NOTICE.
# @(#)$Id$
#
# This software is not subject to any license of the American Telephone
# and Telegraph Company or of the Regents of the University of California.
#
# Permission is granted to anyone to use this software for any purpose on
# any computer system, and to alter it and redistribute it freely, subject
# to the following restrictions:
#
# 1. The author is not responsible for the consequences of use of this
#    software, no matter how awful, even if they arise from flaws in it.
#
# 2. The origin of this software must not be misrepresented, either by
#    explicit claim or by omission.  Since few users ever read sources,
#    credits must appear in the documentation.
#
# 3. Altered versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.  Since few users
#    ever read sources, credits must appear in the documentation.
#
# 4. This notice may not be removed or altered.

MAGIC	= /etc/magic

CFLAGS  += -DMAGIC='"$(MAGIC)"' $(DEFS) $(FLAGS) $(INCLUDES)
LFLAGS  += $(FLAGS)
LIBRARY = libfile.a
RANLIB = :

FILEOBJ=file.o
FILE   =file

TARGETS = $(LIBRARY) $(LOADER) $(FILE)

OBJECTS=apprentice.o \
	fsmagic.o \
	softmagic.o \
	ascmagic.o \
	process.o \
	compress.o \
	is_tar.o \
	print.o \
	form.o \
	libe.o \
	progname.o

all: $(TARGETS)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJECTS)
	$(RANLIB) $(LIBRARY)

%.o:: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

$(FILE):   file.o $(LIBRARY)
	$(CC) $(LFLAGS) -o $(FILE) file.o $(LIBRARY)

ascmagic.o:	names.h

compress.o apprentice.o ascmagic.o file.o fsmagic.o print.o softmagic.o: file.h

clean:
		rm -f $(TARGETS) *.o $(LIBRARY)
