/* $Id: texture.h,v 1.1 2006/04/21 02:27:27 aholkner Exp $ 
 * 
 * Simple texture loader for Interactive 3D Graphics and Animation
 *
 * Alex Holkner
 * http://yallara.cs.rmit.edu.au/~aholkner
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
#  include <windows.h>
#endif

#ifndef GL_HEADER
#  define GL_HEADER <OpenGL/gl.h>
#endif

#include GL_HEADER

/* Load a texture from a PNG file.  The return value is a texture ID which you
 * can pass to glBindTexture.  E.g.:
 * 
 * GLuint myTexture;
 * glEnable(GL_TEXTURE_2D);
 * myTexture = loadTexture("mytexture.png");
 * glBindTexture(GL_TEXTURE_2D, myTexture);
 */

GLuint loadTexture(const char *filename);

/* You can set your own error callback if you like, or just keep the default
 * one, which prints to stderr.
 */
typedef void (*textureErrorCallback)(const char *filename, const char *message);
void setTextureErrorCallback(textureErrorCallback func);

#endif
