/* $Id: capture.c,v 1.1 2006/03/19 14:31:14 mbyrne Exp $
 * Alex Holkner, aholkner@cs.rmit.edu.au
 */

#ifndef GL_HEADER
#  define GL_HEADER   <OpenGL/gl.h>
#  define GLU_HEADER  <OpenGL/glu.h>
#  define GLUT_HEADER <GLUT/glut.h>
#endif

#ifdef WIN32
#  include <windows.h>
#endif
#include GL_HEADER
#include GLUT_HEADER

#include <png.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef TRUE
#  define TRUE 1
#  define FALSE 0
#endif

int capture(const char *filename)
{
    int width, height;
    FILE *fp;
    unsigned char *buffer;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;
    int i;

    /* Determine size of window.  Replace this with lookups to your
     * own global variables if you have them.
     */
    width = glutGet(GLUT_WINDOW_WIDTH);
    height = glutGet(GLUT_WINDOW_HEIGHT);

    /* Open output file */
    fp = fopen(filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "capture: Couln't open output file \"%s\"", filename);
        return FALSE;
    }
    
    /* Read buffer data from GL, allowing 3 bytes per pixel.
     * Be sure to restore the alignment to 4-bytes if you need
     * to.
     */
    buffer = (unsigned char *) malloc(width * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    /*  Initialize PNG structs */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
        NULL, NULL, NULL); 
    if (!png_ptr)
    {
        fprintf(stderr, "capture: Can't initialize png_ptr");
        return FALSE;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
         png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
         fprintf(stderr, "capture: Can't initialze info_ptr");
         return FALSE;
    }

    /*  Initialize PNG error jump */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        fprintf(stderr, "capture: Unknown error");
        return FALSE;
    }

    /*  Give PNG the file handle */
    png_init_io(png_ptr, fp);

    /*  Set PNG options/info.  Assumes 24 bit color buffer, change this
     *  if you need to.
     */
    png_set_IHDR(png_ptr, info_ptr, 
        width,                          /* Width */
        height,                         /* Height */
        8,                              /* Bit depth */ 
        PNG_COLOR_TYPE_RGB,             /* Color type */
        PNG_INTERLACE_NONE,             /* Interlacing */
        PNG_COMPRESSION_TYPE_DEFAULT,   /* Compression */
        PNG_FILTER_TYPE_DEFAULT);       /* Filter method */
    
    /*  Set up row pointers.  OpenGL stores the buffer in reverse
     *  row order to PNG (bottom-to-top instead of top-to-bottom),
     *  so we flip them here.
     */
    row_pointers = png_malloc(png_ptr, height * sizeof(png_bytep));
    for (i = 0; i< height; i++)
        row_pointers[i] = &buffer[(height - i) * width * 3];
    png_set_rows(png_ptr, info_ptr, row_pointers);
    
    /*  Write the PNG */
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /*  Free up */
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    free(buffer);
    
    return TRUE;
}

int captureFrame(const char *prefix)
{
    static char filename[256];
    static int frame = 1;
    snprintf(filename, sizeof filename, "%s%04d.png", prefix, frame);
    frame++;
    return capture(filename);
}

