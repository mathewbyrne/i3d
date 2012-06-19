/* $Id: texture.c,v 1.2 2006/04/21 02:28:58 aholkner Exp $ 
 * 
 * Simple texture loader for Interactive 3D Graphics and Animation
 *
 * Alex Holkner
 * http://yallara.cs.rmit.edu.au/~aholkner
 */

#include "texture.h"

#include <png.h>
#include <stdio.h>
#include <stdlib.h>

void defaultErrorCallback(const char *filename, const char *message)
{
    fprintf(stderr, "Error loading \"%s\": %s\n", filename, message);
}

static textureErrorCallback errorCallback = defaultErrorCallback;
void setTextureErrorCallback(textureErrorCallback func)
{
    if (!func)
        func = defaultErrorCallback;
    errorCallback = func;
}

#define DIE(message) \
    { errorCallback(filename, message); \
      if (fp) \
          fclose(fp); \
      if (png_ptr) \
          png_destroy_read_struct(&png_ptr, &info_ptr, &end_info); \
      return -1; }

#define SIG_BYTES_TO_READ 8

GLuint loadTexture(const char *filename)
{
    FILE *fp = NULL;
    png_byte sig[SIG_BYTES_TO_READ];
    png_size_t sig_num = SIG_BYTES_TO_READ;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_infop end_info = NULL;
    png_uint_32 width;
    png_uint_32 height;
    png_bytep *row_pointers = NULL;
    png_byte color_type;
    png_byte bit_depth;

    char *image_data;
    size_t components;
    int y;
    GLuint texId;

    printf("Loading texture from '%s' ... ", filename);
    
    fp = fopen(filename, "r");
    if (!fp)
        DIE("can't open file");
    
    sig_num = fread(sig, 1, sig_num, fp);
    if (png_sig_cmp(sig, 0, sig_num))
        DIE("not a PNG file");

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        DIE("can't create PNG read struct");

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        DIE("can't create PNG info struct");

    end_info = png_create_info_struct(png_ptr);
    if (!end_info)
        DIE("can't create PNG end info struct");
    
    if (setjmp(png_jmpbuf(png_ptr)))
        DIE("error reading PNG file");
    
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, sig_num);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE ||
        (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) ||
        png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand(png_ptr);
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    components = (color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
                  color_type == PNG_COLOR_TYPE_RGB_ALPHA) ? 4 : 3;
    
    row_pointers = png_get_rows(png_ptr, info_ptr);
    image_data = malloc(width * height * components);
    /* Copy image data from row pointers (there is no guarantee that data
       is already sequential */
    for (y = 0; y < height; y++)
        memcpy(&image_data[width * components * y], 
               row_pointers[height - 1 - y], 
               width * components);

#if DEBUG
    printf("Loaded %s, width=%d, height=%d, components=%d\n",
            filename, (int) width, (int) height, components);
#endif

    /* Create the OpenGL texture */
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 components,
                 width,
                 height,
                 0,
                 components == 3 ? GL_RGB : GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Clean up */
    free(image_data);
    fclose(fp); 
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info); 

    printf("done\n");

    return texId;
}


