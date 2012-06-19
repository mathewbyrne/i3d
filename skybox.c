/**
 * skybox.c
 *
 * Skybox creation rendering. A skybox for the purposes of this assignment is
 * a simple inverted box which contains a texture on each of its faces. This
 * box is meant to be drawn before anything else in the scene with the
 * Depth Buffer disabled so that later drawing calculations do not affect
 * it.
 */
#include "drawing.h"

#define SKY_SIZE 10.0

/**
 * Skybox files. They're named according to their direction, ie xp is x-axis
 * in the positive direction.
 */
char *sky_files[] = { "data/sky/zn.png",           /* 0 - z-axis negative. */
                      "data/sky/xp.png",           /* 1 - x-axis positive. */
                      "data/sky/zp.png",           /* 2 - z-axis positive. */
                      "data/sky/xn.png",           /* 3 - x-axis negative. */
                      "data/sky/yp.png",           /* 4 - y-axis positive. */
                      "data/sky/yn.png"};          /* 5 - y-axis negative. */

/**
 * Simple data structure for holding sky data.
 */
typedef struct _skybox
{
  int texture[6];
  int dlist;
} skybox;

skybox sky;
bool sb_made = false;


/**
 * Returns an array of integers relating to OpenGL texture objects which
 * can be used weith render_skybox below to draw the skybox.
 */
void skybox_init()
{
  int i;

  /* Loop through files, loading thier contents. */
  for(i = 0; i < 6; i++)
  {
    sky.texture[i] = loadTexture(sky_files[i]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  skybox_gen_lists(SKY_SIZE);
  sb_made = true;
}


/**
 * Generates a display list for a skybox. Since the display box is being
 * drawn every frame, we create a display list that can be used to speed
 * up the rendering. This is done independently of the client code.
 */
void skybox_gen_lists(float size)
{
  float f = size, ts = 0.0, tl = 1.0;
  int i, draw_type;

  sky.dlist = glGenLists(2);

  for(i = 0; i < 2; i++)
  {
    /* Generate display list. */
    glNewList(sky.dlist + i, GL_COMPILE);

    /* Disable the depth buffer so that the sky is drawn first before anything
     * else and other drawing occurs over the sky. In other words: depth
     * values for the sky are not recorded. */
    glDisable(GL_DEPTH_TEST);

    if(i == 0)
    {
      draw_type = GL_QUADS;
    }
    else
    {
      draw_type = GL_LINE_LOOP;
      glDisable(GL_TEXTURE_2D);
      glColor4f(0.57, 0.85, 1.0, 1.0);
    }

    /* The following could have been done with Vertex Arrays and generated
     * in a loop, but since it was going into a display list, it was a little
     * easier just to do it by hand. */

    /* Neg Z Dir. */
    glBindTexture(GL_TEXTURE_2D, sky.texture[0]);
    glBegin(draw_type);
      glTexCoord2f(ts, ts); glVertex3f(-f, -f, -f);
      glTexCoord2f(tl, ts); glVertex3f( f, -f, -f);
      glTexCoord2f(tl, tl); glVertex3f( f,  f, -f);
      glTexCoord2f(ts, tl); glVertex3f(-f,  f, -f);
    glEnd();

    /* Pos X Dir. */
    glBindTexture(GL_TEXTURE_2D, sky.texture[1]);
    glBegin(draw_type);
      glTexCoord2f(ts, ts); glVertex3f(f, -f, -f);
      glTexCoord2f(tl, ts); glVertex3f(f, -f,  f);
      glTexCoord2f(tl, tl); glVertex3f(f,  f,  f);
      glTexCoord2f(ts, tl); glVertex3f(f,  f, -f);
    glEnd();

    /* Pos Z Dir. */
    glBindTexture(GL_TEXTURE_2D, sky.texture[2]);
    glBegin(draw_type);
      glTexCoord2f(ts, ts); glVertex3f( f, -f, f);
      glTexCoord2f(tl, ts); glVertex3f(-f, -f, f);
      glTexCoord2f(tl, tl); glVertex3f(-f,  f, f);
      glTexCoord2f(ts, tl); glVertex3f( f,  f, f);
    glEnd();

    /* Neg X Dir. */
    glBindTexture(GL_TEXTURE_2D, sky.texture[3]);
    glBegin(draw_type);
      glTexCoord2f(ts, ts); glVertex3f(-f, -f,  f);
      glTexCoord2f(tl, ts); glVertex3f(-f, -f, -f);
      glTexCoord2f(tl, tl); glVertex3f(-f,  f, -f);
      glTexCoord2f(ts, tl); glVertex3f(-f,  f,  f);
    glEnd();

    /* Pos Y Dir. */
    glBindTexture(GL_TEXTURE_2D, sky.texture[4]);
    glBegin(draw_type);
      glTexCoord2f(ts, ts); glVertex3f(-f, f, -f);
      glTexCoord2f(tl, ts); glVertex3f( f, f, -f);
      glTexCoord2f(tl, tl); glVertex3f( f, f,  f);
      glTexCoord2f(ts, tl); glVertex3f(-f, f,  f);
    glEnd();

    /* Neg Y Dir. */
    glBindTexture(GL_TEXTURE_2D, sky.texture[5]);
    glBegin(draw_type);
      glTexCoord2f(ts, ts); glVertex3f(-f, -f,  f);
      glTexCoord2f(tl, ts); glVertex3f( f, -f,  f);
      glTexCoord2f(tl, tl); glVertex3f( f, -f, -f);
      glTexCoord2f(ts, tl); glVertex3f(-f, -f, -f);
    glEnd();

    glEndList();
  }
}


/**
 * Just executes the display list generated above.
 */
bool skybox_render()
{
  if(!sb_made)
    return false;

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  if(global.r_wire)
    glCallList(sky.dlist + 1);
  else 
  {
    if(global.r_texture)
      glEnable(GL_TEXTURE_2D);
    else
      glColor4f(0.57, 0.85, 1.0, 1.0);
    glCallList(sky.dlist);
  }

  glPopAttrib();

  return true;
}
