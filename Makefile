# $Id: Makefile,v 1.8 2006/03/23 12:18:19 mbyrne Exp $
#
# This Makefile builds the Robot Arm demo from the OpenGL Redbook.  Feel
# free to modify this file for your own project.
#
# Alex Holkner
# http://yallara.cs.rmit.edu.au/~aholkner


# The config.* file contains information specific to your operating system.
# You should copy or symlink the appropriate config.* files to config.os
# before running make.  E.g. if you are in linux:
#    ln -s config.linux config.os
# include config.os
PLATFORM_LIBS = -framework GLUT -framework OpenGL -framework Cocoa

# Uncomment the following line to enable debugging (remember to make clean)
# DEBUG = -g

# Uncomment the following line to generate profiling information (remember
# to make clean)
#PROFILE = -pg

# Uncomment the following line to enable best optimisations
#OPTIMISE = -O3

# This is the name of your program, in this case "robot".  On Windows the
# program will be called "robot.exe" automatically.
PROGRAM = robot
VERSION = 1.0

# A list of all your source files.  If you are writing in C++ remember to
# make the modifications detailed on my webpage.
SOURCES = robot.c animation.c bone.c load_mdl.c capture.c load_obj.c \
          mesh.c texture.c skybox.c drawing.c util.c camera.c editor.c \
          flight.c

# A list of your header files.  These aren't compiled, but if you change one
# it signals Make to recompile everything.
HEADERS = robot.h global.h load_mdl.h capture.h 3d.h load_obj.h texture.h \
					drawing.h util.h mem.h camera.h editor.h flight.h

# A list of object files.  These are the same as your source files, but with
# a .o extension instead of .c.   Remember to keep this up-to-date.
OBJECTS = robot.o animation.o bone.o load_mdl.o capture.o load_obj.o \
          mesh.o texture.o skybox.o drawing.o util.o camera.o editor.o \
          flight.o


#--------------------------------------------------------------------------
# Unless you are doing something unusual you can leave the rest of this
# Makefile alone.  The rules should be fairly simple and self-explanatory,
# however.

DEFINES = 

EXE = $(PROGRAM)$(PLATFORM_EXE)
CFLAGS  = -std=c99 -Wall -pedantic \
          -I/usr/X11/include \
          $(DEBUG) \
          $(PROFILE) \
          $(OPTIMISE) \
          $(PLATFORM_CFLAGS) \
          $(DEFINES)
LDFLAGS = $(PLATFORM_LIBS) -L/usr/X11/lib -lpng -lz

EXTRADIST = Makefile
CONFIGFILES = config.linux config.mac config.cygwin

DISTDIR = $(PROGRAM)-$(VERSION)
DISTFILES = $(SOURCES) $(HEADERS) $(EXTRADIST) $(CONFIGFILES)

CC = clang

all: $(EXE)

$(EXE) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXE) $(LDFLAGS) 

$(OBJECTS): %.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(EXE)
	rm -rf $(DISTDIR)

dist: clean
	rm -rf $(DISTDIR) $(DISTDIR).tar $(DISTDIR).tar.gz
	mkdir $(DISTDIR) 
	cp $(DISTFILES) $(DISTDIR)
	tar -cf $(DISTDIR).tar $(DISTDIR)
	gzip $(DISTDIR).tar
	rm -rf $(DISTDIR)
