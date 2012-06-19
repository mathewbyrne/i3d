/**
 * flight.c
 *
 * Creates some simple code to create multiple instances of birds and
 * animate them.
 */

#include "flight.h"
#include "3d.h"
#include "util.h"
#include "drawing.h"
#include "mem.h"
#include "load_mdl.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


typedef struct flight_pat
{
  model *mdl;
  float pos[3];
  float angle;
  float dist;
  float speed;
} flight_pat;

model *base_bird;
flight_pat *patterns[MODEL_REGISTER_SIZE];
int pat_index = 0;


/**
 * Create a new flight pattern array.
 */
void flight_init()
{
  patterns[pat_index] = NULL;

  base_bird = load_model("data/model/bird.mdl");
  CHECK_NR(base_bird);
}


/**
 * Note that models in the pattern are NOT freed. Often this will be done with
 * the draw_cleanup() function in drawing.c
 */
void flight_cleanup()
{
  int i = 0;

  while(patterns[i])
    FREE(patterns[i++]);

  FREE(base_bird);
}


/**
 * Updates the position of a single flight pattern.
 */
void flight_update_single(flight_pat *pat, float passed)
{
  pat->mdl->pos[0] = pat->dist * cos(RAD(pat->angle)) + pat->pos[0];
  pat->mdl->pos[2] = pat->dist * sin(RAD(pat->angle)) + pat->pos[2];
  pat->mdl->pos[1] = pat->pos[1];
  pat->mdl->pos[4] = - pat->angle + 90;

  pat->angle += passed * pat->speed;
  pat->angle  = mod(pat->angle, 360);
}


/**
 * Clones the passed in model and adds it to the flight.
 */
void flight_new_bird(model *ref, int now)
{
  model *clone = clone_model(ref);
  flight_pat *pat;
  CHECK_NR(clone);

  if(pat_index >= MODEL_REGISTER_SIZE)
  {
    fprintf(stderr, "ERROR(flight_add_bird): Flight array full.\n");
    return;
  }

  draw_model_register(clone);

  NEW(pat);
  pat->mdl    = clone;
  pat->angle  = R * 180.0;
  pat->dist   = R * 100 + 60;
  pat->pos[0] = R * global.world_size / 2.0 * (R > 0.5 ? -1.0 : 1.0);
  pat->pos[1] = R * 40.0 + 20.0;
  pat->pos[2] = R * global.world_size / 2.0 * (R > 0.5 ? -1.0 : 1.0);
  pat->speed  = R * 40 + 80;

  patterns[pat_index++] = pat;

  flight_update_single(pat, 0);

  start_animation(clone, 0, now);
}


/**
 * Updates the positions of all the birds in the scene.
 */
void flight_update(int now)
{
  int i;
  static int last = 0;

  for(i = 0; i < pat_index; i++)
  {
    animate(patterns[i]->mdl, now);
    flight_update_single(patterns[i], (now - last) / 1000.0);
  }
  
  last = now;
}


/**
 * Adds a clone of the passed in model to the flight pattern array. Initialises
 * the flight pattern.
 */
void flight_add_bird(int now)
{
  flight_new_bird(base_bird, now);
}

