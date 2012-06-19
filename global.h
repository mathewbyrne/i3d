/**
 * global.h
 *
 * Contains a set of definitions and includes that are required in multiple
 * files. Mostly just contains the global struct however, which controls
 * the state of the program.
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/**
 * Boolean type definition.
 */
typedef enum{false, true} bool;

/**
 * Macro for checking string equality. A little easier to read than
 * if(str(s1, s2) == 0) ... ie if(streq(s1, s2)) ...
 */
#define streq(s1, s2) (strcmp((s1), (s2)) == 0)

/* R is a psuedo-random number between [0, 1), a useful shorthand for
 * generating random numbers. */
#define R (rand()/(((float)RAND_MAX) + 1.0))

#define WHITESPACE " \n\t"

/* Enumerations and string constants for world mode. */

enum {
  WORLD_MODE_NORMAL,
  WORLD_MODE_EDITOR,
  WORLD_MODE_FLIGHT
};

static char *mode_str[] = {"SOLO", "EDITOR", "FLIGHT"};


/* Used to toggle global variables. */
#define R_TGL(r) (global.r = !(global.r))

/**
 * global struct used to store any variables that may be needed globally.
 * Mostly used to track the state of the program.
 */
typedef struct _global_s
{
  int ww, wh;

  /* GLOBAL STATE */

  int world_mode;               /* Current world mode. */

  /* WORLD DIMENSIONS */

  float sun_pos[4];             /* Sun direction. */
  float ground_plane[4];        /* Planar representation of the ground. */
  float world_size;             /* 1/2 width of the world. */

  /* FPS COUNTER */

  float fps;                    /* FPS for current second. */
  char fps_str[16];             /* String version of FPS. */

  /* RENDERING OPTIONS */

  bool r_texture;               /* Textured or flat color. */
  bool r_shading;               /* Smooth or Flat shading. */
  bool r_wire;                  /* Wireframe mode. */
  bool r_skybox;                /* Render skybox? */
  bool r_shadows;               /* Are we rendering a flat shadow? */
  bool r_grass;                 /* Render grass? */
  bool r_ground;                /* Render the ground? */
  bool r_bones;                 /* Render the bones of the model? */
  bool r_fps;

  bool bb_grass;                /* Render grass billboard or normal style. */

} global_s;

global_s global;                /* Global struct. */

#endif

