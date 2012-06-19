/**
 * drawing.c
 *
 * Most of the more complex drawing tasks are taken care of in this file.
 */

#include "drawing.h"
#include "camera.h"
#include "editor.h"
#include "mem.h"
#include <stdio.h>


int dlists;
int dlist_count = 6;

/* The below enum is used to provide easier display list names, quite
 * useful for calling later. Should be used with the call_list
 * macro under it. */
enum {
  DL_READY_SHADOWS,

  DL_TEX_GROUND,
  DL_WIRE_GROUND,

  DL_READY_GRASS,
  DL_TEX_GRASS,
  DL_WIRE_GRASS,
};

#define call_list(dl) glCallList(dlists + (dl))


/* Shadow variables. */
float shadow_mat[4][4];
float ground_verts[4][3] = {
  {-1.0, 0.0,  1.0}, { 1.0, 0.0,  1.0},
  { 1.0, 0.0, -1.0}, {-1.0, 0.0, -1.0},
};
float ground_tcords[4][2] = {
  {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}
};
float ground_plane[4];
int   ground_tex;
bool  shadowing = false;

/* Grass variables. */
int    grass_tex;
int    grass_count = 0;
float *grass_loc   = NULL;
float  grass_size  = 1.0;

/* Model register. */
model *mdl_reg[MODEL_REGISTER_SIZE];
int mdl_reg_index = 0;

bone *curr_bone;


/**
 * Creates some general display lists for potentially increasing performance
 * in some aspects. Also sets up other drawing related functions for later
 * use. This function should be called before anything else here. It's an
 * unchecked runtime error not to do so.
 */
void draw_init()
{
  int i;

  dlists = glGenLists(dlist_count);

  /* Display list for readying the stencil buffer for the shadow effect. */
  glNewList(dlists + DL_READY_SHADOWS, GL_COMPILE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 3, 0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glEndList();

  ground_tex = loadTexture("data/texture/ground.png");

  /* Display list for drawing a 1x1 quad facing the positive y direction. */
  glNewList(dlists + DL_TEX_GROUND, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, ground_tex);
    glBegin(GL_QUADS);
      for(i = 0; i < 4; i++)
      {
        glTexCoord2f(ground_tcords[i][0] * GROUND_SCALE,
                     ground_tcords[i][1] * GROUND_SCALE);
        glVertex3f  (ground_verts[i][X],
                     ground_verts[i][Y],
                     ground_verts[i][Z]);
      }
    glEnd();
  glEndList();

  /* Wireframe version of the above list. */
  glNewList(dlists + DL_WIRE_GROUND, GL_COMPILE);
    glColor4f(0.54, 0.43, 0.42, 1.0);
    glBegin(GL_LINE_LOOP);
      for(i = 0; i < 4; i++)
        glVertex3f(ground_verts[i][0], ground_verts[i][1], ground_verts[i][2]);
    glEnd();
  glEndList();

  /* Load grass texture and clamp it's edges. We don't want any
   * artifacts around the edges of a grass sprite. */
  grass_tex = loadTexture("data/texture/grass.png");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  /* Display list for drawing a grass quad. */
  glNewList(dlists + DL_TEX_GRASS, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, grass_tex);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0); glVertex3f(-1.0,  0.0,  0.0);
      glTexCoord2f(1.0, 0.0); glVertex3f( 1.0,  0.0,  0.0);
      glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  0.5,  0.0);
      glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  0.5,  0.0);
      glTexCoord2f(0.0, 0.0); glVertex3f( 0.0,  0.0, -1.0);
      glTexCoord2f(1.0, 0.0); glVertex3f( 0.0,  0.0,  1.0);
      glTexCoord2f(1.0, 1.0); glVertex3f( 0.0,  0.5,  1.0);
      glTexCoord2f(0.0, 1.0); glVertex3f( 0.0,  0.5, -1.0);
    glEnd();
  glEndList();

  /* Display list for wireframe grass. */
  glNewList(dlists + DL_WIRE_GRASS, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
      glColor4f(0.63, 0.73, 0.51, 1.0);
      glVertex3f(-1.0,  0.0,  0.0);
      glVertex3f( 1.0,  0.0,  0.0);
      glVertex3f( 1.0,  0.5,  0.0);
      glVertex3f(-1.0,  0.5,  0.0);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3f( 0.0,  0.0, -1.0);
      glVertex3f( 0.0,  0.0,  1.0);
      glVertex3f( 0.0,  0.5,  1.0);
      glVertex3f( 0.0,  0.5, -1.0);
    glEnd();
  glEndList();

  /* Display list for readying grass drawing. */
  glNewList(dlists + DL_READY_GRASS, GL_COMPILE);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);      /* We want both sides of the face drawn. */
    glDepthMask(GL_FALSE);        /* Disable writing to depth mask. */
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor4f(0.36, 0.58, 0.11, 0.8);
  glEndList();

  gen_grass(GRASS_COUNT, global.world_size, GRASS_ROT, GRASS_SIZE);

  skybox_init();
  
  findPlane(ground_plane, ground_verts[0], ground_verts[1],
      ground_verts[2]);
  shadowMatrix(shadow_mat, ground_plane, global.sun_pos);
}


/**
 * Cleans up resources.
 */
void draw_cleanup()
{
  int i;

  FREE(grass_loc);

  /**
   * Free all models in the model register. Only shallowly frees them however
   * since they're supposed to be all clones of other models which should
   * be freeed elsewhere in code.
   */
  for(i = 0; i < mdl_reg_index; i++)
    model_shallow_free(mdl_reg[i]);
}


/**
 * Draws all the models in the model register along with thier shadows
 */
void draw_scene()
{
  int i;
  camera *cam = cam_get();

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  if(global.r_shadows)
    glClear(GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);

  if(global.world_mode == WORLD_MODE_EDITOR)
  {
    glLoadIdentity();

    glTranslatef(0.0, -cam->pos[P_Y], -cam->pos[P_Z]);

    glRotatef(cam->rot[R_X], 1.0, 0.0, 0.0);
    glRotatef(cam->rot[R_Y], 0.0, 1.0, 0.0);
  }
  else
  {
    /* Translate to Camera position and draw the skybox. */
    glLoadIdentity();
    glRotatef(cam->rot[R_X], 1.0, 0.0, 0.0);
    glRotatef(cam->rot[R_Y], 0.0, 1.0, 0.0);

    if(global.r_skybox) skybox_render();

    glTranslatef(cam->pos[P_X], cam->pos[P_Y], cam->pos[P_Z]);
  }

  /* Position the light source. */
  glLightfv(GL_LIGHT0, GL_POSITION, global.sun_pos);

  /* Render shadows if they're enabled. */
  if(global.r_shadows)
  {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    call_list(DL_READY_SHADOWS);
    glDisable(GL_LIGHTING);

    draw_ground(global.world_size);

    shadowing = true;
    glDisable(GL_TEXTURE_2D);
    for(i = 0; i < mdl_reg_index; i++)
      draw_shadow(mdl_reg[i]);
    shadowing = false;
    glPopAttrib();
  }
  else
    draw_ground(global.world_size);

  /* Render models. */
  for(i = 0; i < mdl_reg_index; i++)
  {
    draw_model(mdl_reg[i], DRAW_SKEL_GEOMETRY);

    /* Render model bones. */
    if(global.r_bones)
    {
      glDisable(GL_DEPTH_TEST);
      draw_model(mdl_reg[i], DRAW_SKEL_BONES);
      glEnable(GL_DEPTH_TEST);
    }
  }

  if(global.r_grass)
    draw_grass();

  if(global.r_fps)
    glPrint(global.fps_str, 10, 10);

  glPrint(mode_str[global.world_mode], 10, global.wh - 20);
  if(global.world_mode == WORLD_MODE_EDITOR)
    glPrint(edit_get_string(), 10, global.wh - 40);

  glutSwapBuffers();
}


/**
 * The model register is a simple way of storing models that need to be
 * rendered to the scene.
 */
void draw_model_register(model *mdl)
{
  if(!mdl)
  {
    fprintf(stderr, "ERROR(model_register): NULL Model supplied.\n");
    return;
  }

  if(mdl_reg_index < MODEL_REGISTER_SIZE)
    mdl_reg[mdl_reg_index++] = mdl;
  else
    fprintf(stderr, "ERROR(model_register): Model Register full.\n");
}


bone *c_bone = NULL;
/**
 * Used in the editor to highlight the current bone.
 */
void set_curr_bone(bone *bone)
{
  c_bone = bone;
}


/**
 * Draws a line, used when drawing a representation of a models bone
 * system.
 */
void draw_bone(float length, bool curr)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_LINES);
    if(!curr) glColor3f(1.0, 0.0, 0.0);
    else glColor3f(1.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    if(!curr) glColor3f(0.0, 1.0, 0.0);
    glVertex3f(length, 0.0, 0.0);
  glEnd();
  glPopAttrib();
}


/**
 * Create a matrix that will project the desired shadow given a plane and
 * a source of light. Works for both point and directional lighting.
 * source: http://www.opengl.org/resources/code/samples/mjktips/
 *   TexShadowReflectLight.html
 */
void shadowMatrix(float shadowMat[4][4], float groundplane[4],
   float lightpos[4])
{
  float dot;

  /* Find dot product between light position vector and ground plane normal. */
  dot = groundplane[0] * lightpos[0] +
        groundplane[1] * lightpos[1] +
        groundplane[2] * lightpos[2] +
        groundplane[3] * lightpos[3];

  shadowMat[0][0] = dot - lightpos[0] * groundplane[0];
  shadowMat[1][0] = 0.f - lightpos[0] * groundplane[1];
  shadowMat[2][0] = 0.f - lightpos[0] * groundplane[2];
  shadowMat[3][0] = 0.f - lightpos[0] * groundplane[3];

  shadowMat[X][1] = 0.f - lightpos[1] * groundplane[0];
  shadowMat[1][1] = dot - lightpos[1] * groundplane[1];
  shadowMat[2][1] = 0.f - lightpos[1] * groundplane[2];
  shadowMat[3][1] = 0.f - lightpos[1] * groundplane[3];

  shadowMat[X][2] = 0.f - lightpos[2] * groundplane[0];
  shadowMat[1][2] = 0.f - lightpos[2] * groundplane[1];
  shadowMat[2][2] = dot - lightpos[2] * groundplane[2];
  shadowMat[3][2] = 0.f - lightpos[2] * groundplane[3];

  shadowMat[X][3] = 0.f - lightpos[3] * groundplane[0];
  shadowMat[1][3] = 0.f - lightpos[3] * groundplane[1];
  shadowMat[2][3] = 0.f - lightpos[3] * groundplane[2];
  shadowMat[3][3] = dot - lightpos[3] * groundplane[3];

  return;
}


/**
 * Find the plane equation given 3 points.
 * source: http://www.opengl.org/resources/code/samples/mjktips/
 *   TexShadowReflectLight.html
 */
void findPlane(float plane[4], float v0[3], float v1[3], float v2[3])
{
  GLfloat vec0[3], vec1[3];

  /* Need 2 vectors to find cross product. */
  vec0[X] = v1[X] - v0[X];
  vec0[Y] = v1[Y] - v0[Y];
  vec0[Z] = v1[Z] - v0[Z];

  vec1[X] = v2[X] - v0[X];
  vec1[Y] = v2[Y] - v0[Y];
  vec1[Z] = v2[Z] - v0[Z];

  /* find cross product to get A, B, and C of plane equation */
  plane[0] = vec0[Y] * vec1[Z] - vec0[Z] * vec1[Y];
  plane[1] = -(vec0[X] * vec1[Z] - vec0[Z] * vec1[X]);
  plane[2] = vec0[X] * vec1[Y] - vec0[Y] * vec1[X];
  plane[3] = -(plane[0] * v0[X] + plane[1] * v0[Y] + plane[2] * v0[Z]);
}


/**
 * Draws shadows for a given model. Really just sets up the rendering, applies
 * a matrix to the current stack before drawing the model again. The stencil
 * buffer is used to ensure that no area is drawn to more than once, and
 * shadows do not have a 'cumulative effect.
 */
void draw_shadow(model *mdl)
{
  glStencilFunc(GL_LESS, 2, 0xffffffff);

  glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glColor4f(0.07, 0.0, 0.0, SHADOW_DEPTH);

  glPushMatrix();
  
  glMultMatrixf((float *)shadow_mat);   /* Apply the shadow matrix and */
  draw_model(mdl, DRAW_SKEL_GEOMETRY);  /* redraw the model.           */

  glPopMatrix();
}


/**
 * Draws a big plane for the ground.
 */
void draw_ground(float dist)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  glDisable(GL_LIGHTING);

  glScalef(dist, 1.0, dist);

  if(global.r_texture) glEnable(GL_TEXTURE_2D);
  else glColor4f(0.54, 0.43, 0.42, 1.0);

  if(global.world_mode == WORLD_MODE_EDITOR)
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  if(global.r_wire)
    call_list(DL_WIRE_GROUND);
  else
    call_list(DL_TEX_GROUND);

  if(global.world_mode == WORLD_MODE_EDITOR)
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  glPopMatrix();
  glPopAttrib();
}


/**
 * Generates count random grass locations and rotations. The locations
 * are within a square of length 2 * dist and the rotations are between
 * 0 and max_rot - 1 degrees.
 */
void gen_grass(int count, float dist, int max_rot, float size)
{
  int i;

  if(count <= 0 || dist <= 0) return;

  if(grass_loc == NULL) free(grass_loc);

  grass_loc = malloc(sizeof(float) * count * 3);
  if(grass_loc == NULL) return;

  grass_count = count;
  grass_size  = size;

  for(i = 0; i < count * 3; i += 3)
  {
    grass_loc[i + 0] = R * dist * (R > 0.5 ? -1.0 : 1.0);
    grass_loc[i + 1] = R * dist * (R > 0.5 ? -1.0 : 1.0);
    grass_loc[i + 2] = R * max_rot;
  }
}


/**
 * Draws all the patches of grass around the world. The grass locations are
 * stored in the float array grass_loc. Grass locations can be set with the
 * init_grass function above.
 */
void draw_grass()
{
  int i;
  camera *cam = cam_get();

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  if(global.r_texture && !global.r_wire) glEnable(GL_TEXTURE_2D);
  call_list(DL_READY_GRASS);

  for(i = 0; i < grass_count * 3; i += 3)
  {
    glPushMatrix();

    glTranslatef(grass_loc[i], 0.0, grass_loc[i + 1]);
    if(global.bb_grass)
      glRotatef(-cam->rot[R_Y], 0.0, 1.0, 0.0);
    else
      glRotatef(grass_loc[i + 2], 0.0, 1.0, 0.0);
    glScalef(grass_size, grass_size * 1.5, grass_size);

    if(global.r_wire)
      call_list(DL_WIRE_GRASS);
    else
      call_list(DL_TEX_GRASS);

    glPopMatrix();
  }
  glPopAttrib();

}


/**
 * Recursive function to draw an entire skeleton to the display. Also does the
 * required translations to draw the skeleton to the screen.
 */
void draw_skeleton(bone *skel, int type)
{
  if(skel == NULL) return;

  glPushMatrix();

  /* All translations are taken care of here. */
  glRotatef(skel->rot[RX], 1.0, 0.0, 0.0);
  glRotatef(skel->rot[RY], 0.0, 1.0, 0.0);
  glRotatef(skel->rot[RZ], 0.0, 0.0, 1.0);

  if(type == DRAW_SKEL_BONES)
  {
    if(c_bone == skel)
      draw_bone(skel->length, true);
    else
      draw_bone(skel->length, false);
  }
  else if(type == DRAW_SKEL_GEOMETRY && skel->geometry != NULL)
  {
    glInterleavedArrays(GL_T2F_N3F_V3F, 0, skel->geometry);
    if(global.r_wire)
      glDrawArrays(GL_LINES, 0, skel->tri_count);
    else
      glDrawArrays(GL_TRIANGLES, 0, skel->tri_count);
  }

  /* Translate to the new position. */
  glTranslatef(skel->length, 0.0, 0.0);

  /* Draw the skeletons first child. */
  draw_skeleton(skel->child, type);

  glPopMatrix();

  /* Draw the skeletons sibling after the pop. */
  draw_skeleton(skel->sibling, type);
}


/**
 * Draws a model struct. Most of the drawing is done in draw_skeleton.
 */
void draw_model(model *mdl, int type)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  if(!shadowing)
  {
    if(global.r_texture && !global.r_wire)
      glEnable(GL_TEXTURE_2D);
    else
      glDisable(GL_TEXTURE_2D);

    if(global.r_wire)
    {
      glDisable(GL_LIGHTING);
      glColor4f(0.6, 0.6, 0.7, 1.0);
    }
    else
      glEnable(GL_LIGHTING);
  }
  else
  {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
  }

  glBindTexture(GL_TEXTURE_2D, mdl->texture);

  glTranslatef(mdl->pos[0], mdl->pos[1], mdl->pos[2]);
  glRotatef(mdl->pos[3], 1.0, 0.0, 0.0);
  glRotatef(mdl->pos[4], 0.0, 1.0, 0.0);
  glRotatef(mdl->pos[5], 0.0, 0.0, 1.0);

  draw_skeleton(mdl->root, type);

  glPopMatrix();
  glPopAttrib();
}


/**
 * Utility function, allows the printing of text onto the screen using screen
 * coordinates. Needs to change the current projection and modelview matrixes
 * in order to make the coorinates sync up with the current screen.
 */
void glPrint(char *string, int x, int y)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glColor4f(0.0, 0.0, 0.0,1.0);

  /* Reset the Projection matrix and set up the screen. */
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.0, global.ww, 0.0, global.wh);

  /* Reset the ModelView matrix. */
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  /* Draw the text. */
  glRasterPos2d(x, y);
  glColor3f(1.0, 1.0, 1.0);
  // glutBitmapString(GLUT_BITMAP_9_BY_15, (unsigned char *) string);

  /* Reset the matrices. */
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

