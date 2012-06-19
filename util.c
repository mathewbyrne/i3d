/**
 * util.c
 */

#include "util.h"

#include <math.h>
#include <stdio.h>


/**
 * Normalize a vector. Simply divide by its magnitude.
 */
void v_norm(float v[3])
{
  float mag = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  if(mag == 0.0)
  {
    fprintf(stderr, "ERROR(normalize): 0 length vector.\n");
    return;
  }

  v[0] /= mag;
  v[1] /= mag;
  v[2] /= mag;
}


/**
 * Vector Dot Product.
 */
float v_dot(float v0[3], float v1[3])
{
  return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

/**
 * Scale a vector by a given amount.
 */
void v_scale(float v[3], float factor)
{
  v[0] *= factor;
  v[1] *= factor;
  v[2] *= factor;
}


/**
 *  Add one vector to another.
 */
void v_add(float r[3], float v[3])
{
  r[0] += v[0];
  r[1] += v[1];
  r[2] += v[2];
}

/**
 * Vector Subtraction.
 */
void v_sub(float r[3], float v0[3], float v1[3])
{
  r[0] = v0[0] - v1[0];
  r[1] = v0[1] - v1[1];
  r[2] = v0[2] - v1[2];
}


/**
 * Vector Cross Product.
 */
void v_cross(float r[3], float v0[3], float v1[3])
{
  r[0] = (v0[1] * v1[2]) - (v0[2] * v1[1]);
  r[1] = (v0[2] * v1[0]) - (v0[0] * v1[2]);
  r[2] = (v0[0] * v1[1]) - (v0[1] * v1[0]);
}


/**
 * Copy a vector to another.
 */
void v_copy(float dest[3], float v[3])
{
  dest[0] = v[0];
  dest[1] = v[1];
  dest[2] = v[2];
}


/**
 * Sets a vector to 0.
 */
void v_clear(float v[3])
{
  v[0] = v[1] = v[2] = 0.0;
}


/**
 * Print the values of a vector out in a readable format.
 */
void v_print(float v[3])
{
  printf("{ %.2f, %.2f, %.2f} ", v[0], v[1], v[2]);
}


/**
 * Mod for floats :D
 */
float mod(float value, int mod)
{
  float mod_value = value;

  while(mod_value > mod)
    mod_value -= mod;

  return mod_value;
}


/**
 * Restricts a value to only being between two other values.
 */
float clamp(float value, float min, float max)
{
  if(value >= max)
    return max;
  else if(value <= min)
    return min;
  else
    return value;
}


