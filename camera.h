/**
 * camera.h
 * 
 * Simple camera implementation for a free moving camera with FPS style
 * controls: mouse gives direction and keys are bound to forward motion,
 * backward motion and strafing.
 */

#ifndef _CAMERA_H_
#define _CAMERA_H_

#define T camera

#include "global.h"


/* Camera directions. */
typedef enum
{
  CAM_MOVE_FORWARD,
  CAM_MOVE_BACKWARD,
  CAM_MOVE_STRAFE_L,
  CAM_MOVE_STRAFE_R
} cam_dir;


/* Position enums */
enum { R_X, R_Y };
enum { P_X, P_Y, P_Z };


/* Camera structure. */
typedef struct T
{
  float pos[3];                   /* Camera position. */
  float rot[2];                   /* Camera rotation. */

  float pos_to[3];                /* Used for interpolation. */
  float pos_from[3];
  float rot_to[2];
  float rot_from[2];
  bool  interp;
  int time_from;
  int time_to;

  float pos_speed;                /* Speeds of rotation and movement. */
  float rot_speed;

  bool moving[4];                 /* Movement/Rotation flags. */
  bool roting;
} T;


/* Interface. */
extern T *cam_new();
extern void cam_set(T *cam);
extern T *cam_get();
extern void cam_move(cam_dir dir);
extern void cam_stop(cam_dir dir);
extern void cam_update(float t_passed, int now);
extern void cam_mouse_motion(int mx, int my);
extern void cam_mouse_click(int btn, int state, int mx, int my);


#undef T

#endif
