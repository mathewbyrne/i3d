/**
 * drawing.h
 *
 * Declerations for drawing.c. Does most of the game loop OpenGL drawing.
 * The entire interface is defined here although most of these functions
 * will not be used outside drawing.c.
 */

#ifndef _DRAWING_H_
#define _DRAWING_H_

#include <GLUT/glut.h>

#include "global.h"
#include "3d.h"
#include "texture.h"

#define SHADOW_DEPTH 0.2

#define GRASS_COUNT 128
#define GRASS_ROT    45
#define GRASS_SIZE  6.0

#define GROUND_SCALE 6.0

#define MODEL_REGISTER_SIZE 128


/* States for drawing a skeleton. Can draw either mesh or bones. */
enum {
  DRAW_SKEL_BONES,
  DRAW_SKEL_GEOMETRY
};


/* Drawing Interface. */

extern void draw_init();
extern void draw_cleanup();
extern void draw_scene();
extern void glPrint(char *string, int x, int y);

/* Model register functions. */
extern void draw_model_register(model *mdl);

/* Shadow functions. */
extern void shadowMatrix(float shadowMat[4][4], float groundplane[4],
    float lightpos[4]);
extern void ready_shadows();
extern void finish_shadows();
extern void draw_shadow(model *mdl);
extern void findPlane(float plane[4], float v0[3], float v1[3], float v2[3]);

/* Environment functions. */
extern void draw_ground(float dist);
extern void gen_grass(int count, float dist, int max_rot, float size);
extern void draw_grass();

/* Model drawing functions. */
extern void set_curr_bone(bone *bone);
extern void draw_bone(float length, bool curr);
extern void draw_skeleton(bone *skel, int type);
extern void draw_model(model *mdl, int type);

/* Skybox functions. (from skybox.c) */
extern void skybox_init();
extern void skybox_gen_lists(float size);
extern bool skybox_render();

#endif

