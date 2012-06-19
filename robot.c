/**
 * robot.c
 *
 * Interactive 3D Graphics & Animation
 * 
 * Mathew Byrne
 * 3081491
 *
 * This is the main entry point of this assignment. It started as a very
 * bloated piece of code but slowly I've moved sections out into other
 * files (drawing.c for instance).
 *
 * Read the report for more details of the development.
 */

#include <OpenGL/gl.h> 
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "global.h"
#include "3d.h"
#include "load_mdl.h"
#include "capture.h"
#include "drawing.h"
#include "camera.h"
#include "editor.h"
#include "util.h"
#include "flight.h"
#include "util.h"


/* Enumerations for menu constants. */

enum {
  MM_T_FPS,
  MM_T_SKYBOX,
  MM_T_SHADOWS,
  MM_T_GRASS,
  MM_T_BB_GRASS,
  MM_RANDOMIZE_GRASS,
  MM_SCREENSHOT,
  MM_EXIT
};

enum {
  RM_TEXTURE,
  RM_WIRE,
  RM_SHADING
};

enum {
  AN_FLYING,
  AN_GET_WORM,
  AN_FRIGHT
};


model *bird; /* Only used in solo mode now. */


/**
 * Init function initialises the program and performs all operations
 * required for the program to execute.
 */
void init(void) 
{
  /* Variables mostly needed for setting up simple materials and lighting. */
  float mat_spec[]     = {1.0, 1.0, 1.0, 1.0};
  float mat_shin[]     = {100.0};
  float mat_amb_diff[] = {0.6, 0.6, 0.6, 1.0};
  float white_light[]  = {1.0, 1.0, 0.9, 1.0};
  float lmodel_amb[]   = {0.4, 0.4, 0.4, 1.0};
  int now = glutGet(GLUT_ELAPSED_TIME);
  camera *cam;

  /* Init random seed to the current time. */
  srand(time(0));

  /* OpenGL initialization. */
  printf("Setting up OpenGL variables ... ");
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(1.0);
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glPolygonOffset(-2.0, -1.0);

  /* Setup lighting. */
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shin);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
  glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_amb);
  glEnable(GL_LIGHT0);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
  printf("done");

  /* Setup global variables. */
  printf("Setting up global variables ... ");
  global.sun_pos[0] =   2.0;
  global.sun_pos[1] =   1.3;
  global.sun_pos[2] =   1.0;
  global.sun_pos[3] =   0.0;
  global.r_texture  =  true;
  global.r_wire     = false;
  global.r_shading  =  true;
  global.r_shadows  =  true;
  global.r_skybox   =  true;
  global.r_grass    =  true;
  global.r_ground   =  true;
  global.bb_grass   =  true;
  global.r_fps      =  true;
  global.world_size = 512.0;
  printf("done\n");

  /* Global camera instance. */
  printf("Setting up new camera instance ... ");
  cam = cam_new();
  cam->pos[0] =   50;
  cam->pos[1] =  -20;
  cam->pos[2] =   50;
  cam->rot[0] = cam->rot_from[0] = -40;
  cam->rot[1] = cam->rot_from[1] = 150;
  cam->rot_to[0] =  15;
  cam->rot_to[1] = 130;
  cam->interp = true;
  cam->time_from = now;
  cam->time_to = now + 4100;
  cam_set(cam);
  printf("done\n");

  /* Initialise drawing functions. */
  draw_init();

  /* Set global variables depending on world mode. */
  switch(global.world_mode)
  {
    default:
    case WORLD_MODE_NORMAL:
      /* Load models and skeletons. */
      bird = load_model("data/model/bird.mdl");
      if(bird == NULL)
      {
        fprintf(stderr, "ERROR(init): Unable to load model from file.\n");
        exit(1);
      }
      bird->pos[4] = 180;

      draw_model_register(bird);
      break;

    /* Initialize the flight mode. */
    case WORLD_MODE_FLIGHT:
      printf("Setting up flight mode ...");
      flight_init();
      flight_add_bird(now);
      break;

    case WORLD_MODE_EDITOR:
      printf("Setting up editor ... ");
      edit_init();
      glutKeyboardFunc(edit_keyboard);
      break;
  }
}


/**
 * The reshape function is called by GLUT when the program starts and
 * also when the window is resized.  It defines the size of the viewport
 * (how much is visible), the field-of-view and near/far clipping planes.
 */
void reshape (int w, int h)
{
  /* Set the viewport to be the same size as the window. */
  glViewport(0, 0, (GLsizei) w, (GLsizei) h); 
   
  /* Tell OpenGL we want to modify the projection matrix instead of the
   * modelview matrix */
  glMatrixMode(GL_PROJECTION);

  /* Clear the old projection matrix */
  glLoadIdentity();
  gluPerspective(65.0, (GLfloat) w/(GLfloat) h, 1.0, 1024);

  /* Resume modifying the modelview matrix */
  glMatrixMode(GL_MODELVIEW);
  /* Clear the old modelview matrix -- we are now at the world origin */
  glLoadIdentity();
  /* Translate the modelview matrix -5 units on the z-axis; i.e. move the
   * camera back (out of the screen) 5 units */
  glTranslatef(0.0, 0.0, -5.0);

  global.ww = w;
  global.wh = h;
}


bool jumping = false; /* Ooo, that's a bit of a hack :P */
int jump_time = 0;
#define JUMP_LEN 1710


/**
 * Main processing function in the program. These commands are executed once
 * every frame.
 */
void idle()
{
  /* Time constants used to track the period of time that has elapsed between
   * now and the previous frame.  */
  static int last = 0;
  static int frames = 0;
  static int frame_time = 0;
  int now = glutGet(GLUT_ELAPSED_TIME);
  float passed = (now - last) / 1000.0F;
  int error;

  /* Update FPS Counter. */
  frames++;
  frame_time += (now - last);
  if(frame_time > 1000)
  {
    global.fps = frames / ((float)frame_time / 1000.0);
    frame_time = frames = 0;
    sprintf(global.fps_str, "FPS: %.1f", global.fps);
  }

  cam_update(passed, now);
  
  switch(global.world_mode)
  {
    case WORLD_MODE_NORMAL:
      animate(bird, now);

      /* This small bit of code is used when the fright animation is selected.
       * It makes the bird jump up in the air. */
      if(jumping)
      {
        if(jump_time > JUMP_LEN + 1000)
          jump_time = 0;
        if(jump_time < JUMP_LEN - 200)
          bird->pos[1] = 20.0 * sin(2 * MY_PI * jump_time
              / (JUMP_LEN - 200) * 0.5);

        jump_time += now - last;
      }
      break;

    case WORLD_MODE_EDITOR:
      edit_update(now);
      break;

    case WORLD_MODE_FLIGHT:
      flight_update(now);
      break;
  }

  if((error = glGetError()))
    printf("GL Error: %s\n", gluErrorString(error));

  glutPostRedisplay();
  last = now;
}


/**
 * Final destructor like method which will clean up allocated memory and
 * make our program nice and neat :) Hopefully deallocating all memory
 * allocated, although please don't check this :P
 */
void cleanup()
{
  camera *cam = cam_get();
  free(cam);

  draw_cleanup();

  if(global.world_mode == WORLD_MODE_FLIGHT)
    flight_cleanup();

  if(global.world_mode == WORLD_MODE_EDITOR)
    edit_cleanup();
}


/**
 * Handles the basic menu system offered by GLUT.
 */
void main_menu(int value)
{
  switch(value)
  {
    case MM_T_FPS:
      R_TGL(r_fps);
      break;
    case MM_T_SKYBOX:
      R_TGL(r_skybox);
      break;
    case MM_T_SHADOWS:
      R_TGL(r_shadows);
      break;
    case MM_T_GRASS:
      R_TGL(r_grass);
      break;
    case MM_T_BB_GRASS:
      R_TGL(bb_grass);
      break;
    case MM_RANDOMIZE_GRASS:
      gen_grass(GRASS_COUNT, global.world_size, GRASS_ROT, GRASS_SIZE);
      break;
    case MM_EXIT:
      cleanup();
      exit(0);
      break;
    case MM_SCREENSHOT:
      captureFrame("Screenshot");
      break;
  }
}


/**
 * Change rendering mode.
 */
void render_menu(int value)
{
  switch(value)
  {
    default:
    case RM_TEXTURE:
      R_TGL(r_texture);
      break;
    case RM_WIRE:
      R_TGL(r_wire);
      break;
    case RM_SHADING:
      if(global.r_shading)
        glShadeModel(GL_FLAT);
      else
        glShadeModel(GL_SMOOTH);
      R_TGL(r_shading);
      break;
  }
}


/**
 * Special key functions.
 */
void special_keys(int key, int mx, int my)
{
  switch(key)
  {
    case GLUT_KEY_F1:
      render_menu(RM_TEXTURE);
      break;
    case GLUT_KEY_F2:
      render_menu(RM_WIRE);
      break;
    case GLUT_KEY_F3:
      render_menu(RM_SHADING);
      break;
    case GLUT_KEY_F4:
      main_menu(MM_T_SKYBOX);
      break;
    case GLUT_KEY_F5:
      main_menu(MM_T_GRASS);
      break;
    case GLUT_KEY_F6:
      main_menu(MM_T_BB_GRASS);
      break;
    case GLUT_KEY_F7:
      main_menu(MM_RANDOMIZE_GRASS);
      break;
    case GLUT_KEY_F8:
      main_menu(MM_T_FPS);
      break;
    case GLUT_KEY_F12:
      main_menu(MM_SCREENSHOT);
      break;
  }
}


/**
 * Change the current animation. Handle any special requirements that a
 * particular animation may have.
 */
void anim_menu(int value)
{
  int now = glutGet(GLUT_ELAPSED_TIME);
  start_animation(bird, value, now + 50);
  jumping = false;

  switch(value)
  {
    case AN_FLYING:
      bird->pos[1] = 10;
      break;

    case AN_GET_WORM:
      bird->pos[1] = 0;
      break;

    case AN_FRIGHT:
      jumping = true;
      jump_time = 0;
      break;
  }
}


/* The keyboard function is called by GLUT every time a key is pressed,
 * or if a key is held down, the function is called repeatedly several
 * times a second.  The parameters are:
 *   key  -- The character that was pressed
 *   x, y -- The position of the mouse when the key was pressed
 */
void keyboard (unsigned char key, int x, int y)
{
  int now = glutGet(GLUT_ELAPSED_TIME);

  switch(key)
  {
    case ' ':
      if(global.world_mode == WORLD_MODE_FLIGHT)
        flight_add_bird(now);
      break;

    case 'w':
      cam_move(CAM_MOVE_FORWARD);
      break;
    case 'a':
      cam_move(CAM_MOVE_STRAFE_L);
      break;
    case 's':
      cam_move(CAM_MOVE_BACKWARD);
      break;
    case 'd':
      cam_move(CAM_MOVE_STRAFE_R);
      break;

    case 27:
      exit(0);
      break;
  }

  glutPostRedisplay();
}


/**
 * Because the camera needs to be a little more responsive than the
 * average key pressing allows, we use this key "unpress" function to stop
 * it moving.
 */
void keyboard_up(unsigned char key, int mx, int my)
{
  switch(key)
  {
    case 'w':
      cam_stop(CAM_MOVE_FORWARD);
      break;
    case 'a':
      cam_stop(CAM_MOVE_STRAFE_L);
      break;
    case 's':
      cam_stop(CAM_MOVE_BACKWARD);
      break;
    case 'd':
      cam_stop(CAM_MOVE_STRAFE_R);
      break;
  }
}


/**
 * The main entry point of the program. Sets our world mode, sets up GLUT
 * and our menu system and gets the ball rolling.
 */
int main(int argc, char** argv)
{
  int i, render_sub_menu, anim_sub_menu;

  /* Work out command line arguments. */
  global.world_mode = WORLD_MODE_NORMAL;
  for(i = 0; i < argc; i++)
  {
    if(argv[i][0] == '-')
    {
      if(streq(argv[i] + 1, "e"))
        global.world_mode = WORLD_MODE_EDITOR;
      else if(streq(argv[i] + 1, "f"))
        global.world_mode = WORLD_MODE_FLIGHT;
    }
  }

  /* GLUT initialization. */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  glutInitWindowSize(800, 600); 
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Robo-Chicken 3000");

  /* Setting up GLUT Callback functions. */
  glutDisplayFunc(draw_scene); 
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboard_up);
  glutSpecialFunc(special_keys);
  glutMouseFunc(cam_mouse_click);
  glutMotionFunc(cam_mouse_motion);
  glutIdleFunc(idle);

  init();

  /* Setup the old-skool right-click menu. */
  render_sub_menu = glutCreateMenu(render_menu);
  glutAddMenuEntry("Toggle Textured", RM_TEXTURE);
  glutAddMenuEntry("Toggle Wire-Frame", RM_WIRE);
  glutAddMenuEntry("Toogle Shading", RM_SHADING);

  if(global.world_mode == WORLD_MODE_NORMAL)
  {
    anim_sub_menu = glutCreateMenu(anim_menu);
    glutAddMenuEntry("Flying", AN_FLYING);
    glutAddMenuEntry("Eating Worm", AN_GET_WORM);
    glutAddMenuEntry("Fright!", AN_FRIGHT);
  }
  
  glutCreateMenu(main_menu);
  glutAddSubMenu("Rendering Mode", render_sub_menu);
  if(global.world_mode == WORLD_MODE_NORMAL)
    glutAddSubMenu("Animation", anim_sub_menu);
  glutAddMenuEntry("Toggle FPS Display", MM_T_FPS);
  glutAddMenuEntry("Toggle Skybox", MM_T_SKYBOX);
  glutAddMenuEntry("Toggle Shadows", MM_T_SHADOWS);
  glutAddMenuEntry("Toggle Grass", MM_T_GRASS);
  glutAddMenuEntry("Toggle Grass Billboard", MM_T_BB_GRASS);
  glutAddMenuEntry("Randomize Grass", MM_RANDOMIZE_GRASS);
  glutAddMenuEntry("Take Screenshot", MM_SCREENSHOT);
  glutAddMenuEntry("Exit", MM_EXIT);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  glutMainLoop();

  cleanup();

  return EXIT_SUCCESS;
}

