# $Id: config.os,v 1.3 2006/03/19 11:17:38 mbyrne Exp $
#
# This configuration file is included by Makefile to define Linux-specific
# functionality.  You may need to change the path to X11 if it is installed
# in a non-standard location.
#
# To use this configuration file, copy or symlink config.os to this file.
# E.g.,
#      ln -s config.linux config.os
#
# Alex Holkner
# http://yallara.cs.rmit.edu.au/~aholkner

GL_HEADER   = GL/gl.h
GLU_HEADER  = GL/glu.h
GLUT_HEADER = GL/glut.h

PLATFORM_LIBS   = -lGL -lGLU -lglut -lSDL_image
PLATFORM_CFLAGS = 
PLATFORM_EXE    = 
