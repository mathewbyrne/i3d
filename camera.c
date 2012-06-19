/**
 * camera.c
 *
 * Implementation of the camera interface specified in camera.h
 */

#include "camera.h"
#include "util.h"
#include "mem.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GLUT/glut.h>


#define T camera
#define CHECK_CAM() if(!c_cam) \
  { \
    fprintf(stderr, "ERROR(cam_set): No Camera assigned.\n"); \
    return; \
  }
#define MAX_MOUSE_DIST 400
#define MAX_X_TILT 60


static T *c_cam = NULL;
static int s_mx, s_my;
static int c_mx, c_my;


/**
 * Create and initialize a new camera struct.
 */
T *cam_new()
{
  T *cam;

  NEW(cam);
  CHECK(cam);

  v_clear(cam->pos);
  v_clear(cam->pos_from);
  v_clear(cam->pos_to);

  cam->rot[R_X] = cam->rot[R_Y] = 0.0;
  cam->rot_from[R_X] = cam->rot_from[R_Y] = 0.0;
  cam->rot_to[R_X] = cam->rot_to[R_Y] = 0.0;
  cam->interp = false;

  cam->pos_speed = 100.0;
  cam->rot_speed = 800.0;

  cam->roting                    = false;
  cam->moving[CAM_MOVE_FORWARD]  = false;
  cam->moving[CAM_MOVE_BACKWARD] = false;
  cam->moving[CAM_MOVE_STRAFE_L] = false;
  cam->moving[CAM_MOVE_STRAFE_R] = false;

  return cam;
}


/**
 * Set the current internal camera pointer. This file works like a state
 * machine. You set up a camera and let this file do the work on it.
 */
void cam_set(T *cam)
{
  if(!cam)
    fprintf(stderr, "ERROR(cam_set): NULL Camera assigned.\n");
  c_cam = cam;
}


/**
 * Returns a pointer to the currently set camera. May return a null pointer.
 */
T *cam_get()
{
  return c_cam;
}


/**
 * Set the camera moving in a particular direction.
 */
void cam_move(cam_dir dir)
{
  CHECK_CAM();
  if(c_cam->interp) c_cam->interp = false;

  c_cam->moving[dir] = true;
}


/**
 * Stop the camera moving in a direction.
 */
void cam_stop(cam_dir dir)
{
  CHECK_CAM();

  c_cam->moving[dir] = false;
}


/**
 * Most of the work with the camera is done here. Any movements or rotations
 * are taken care of, should be called once per frame.
 */
void cam_update(float t_passed, int now)
{
  float dir[3];
  float cosy;
  float dx, dy;
  float a;
  int i;

  CHECK_CAM();

  /* Rotations. */
  if(c_cam->roting)
  {
    dx = t_passed * c_cam->rot_speed * ((float)(c_mx - s_mx) / MAX_MOUSE_DIST);
    dy = t_passed * c_cam->rot_speed * ((float)(c_my - s_my) / MAX_MOUSE_DIST);

    c_cam->rot[R_X] = clamp(c_cam->rot[R_X] + dy, -MAX_X_TILT, MAX_X_TILT);
    c_cam->rot[R_Y] = mod(c_cam->rot[R_Y] + dx, 360);
  }

  /* Directional Movements. */
  /* Here we are converting between spherical polar coordinates (ie a pair
   * or rotations, and direction vectors. These vectors are used to
   * caculate the direction the camera is to travel. */
  if(c_cam->moving[CAM_MOVE_FORWARD])
  {
    cosy = cos(RAD(c_cam->rot[R_X]));

    dir[P_X] = -  cosy * sin(RAD(c_cam->rot[R_Y]));
    dir[P_Z] =    cosy * cos(RAD(c_cam->rot[R_Y]));
    dir[P_Y] = sin(RAD(c_cam->rot[R_X]));

    v_norm(dir);
    v_scale(dir, c_cam->pos_speed * t_passed);
    v_add(c_cam->pos, dir);
  }
  if(c_cam->moving[CAM_MOVE_BACKWARD])
  {
    cosy = cos(RAD(c_cam->rot[R_X]));

    dir[P_X] = - cosy * sin(RAD(c_cam->rot[R_Y]));
    dir[P_Z] =   cosy * cos(RAD(c_cam->rot[R_Y]));
    dir[P_Y] = sin(RAD(c_cam->rot[R_X]));

    v_norm(dir);
    v_scale(dir, -c_cam->pos_speed * t_passed);
    v_add(c_cam->pos, dir);
  }
  if(c_cam->moving[CAM_MOVE_STRAFE_R])
  {
    dir[P_X] = cos(RAD(c_cam->rot[R_Y]));
    dir[P_Z] = sin(RAD(c_cam->rot[R_Y]));
    dir[P_Y] = 0.0;

    v_norm(dir);
    v_scale(dir, - c_cam->pos_speed * t_passed);
    v_add(c_cam->pos, dir);
  }
  if(c_cam->moving[CAM_MOVE_STRAFE_L])
  {
    dir[P_X] = cos(RAD(c_cam->rot[R_Y]));
    dir[P_Z] = sin(RAD(c_cam->rot[R_Y]));
    dir[P_Y] = 0.0;

    v_norm(dir);
    v_scale(dir, c_cam->pos_speed * t_passed);
    v_add(c_cam->pos, dir);
  }

  if(c_cam->interp)
  {
    a = MY_PI / 2 * (c_cam->time_to - c_cam->time_from);

    for(i = 0; i < 3; i++)
    {
      if(c_cam->rot[i] != c_cam->rot_to[i])
        c_cam->rot[i] = (c_cam->rot_to[i] - c_cam->rot_from[i]) *
          sin(a * (now - c_cam->time_from)) + c_cam->rot_from[i];
    }
  }
}


/**
 * Updates camera rotation depending on the position of the
 * mouse.
 */
void cam_mouse_motion(int mx, int my)
{
  CHECK_CAM();

  if(c_cam->roting)
  {
    c_mx = mx;
    c_my = my;
  }
}


/**
 * Sets camera state depending on what button was clicked.
 */
void cam_mouse_click(int btn, int state, int mx, int my)
{
  CHECK_CAM();

  switch(btn)
  {
    case GLUT_LEFT_BUTTON:
      if(state == GLUT_DOWN)
      {
        if(c_cam->interp) c_cam->interp = false;
        c_cam->roting = true;
        s_mx = c_mx = mx;
        s_my = c_my = my;
      }
      else
        c_cam->roting = false;
      break;
  }
}


#undef T

