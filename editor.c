/**
 * editor.c
 */

#include "editor.h"
#include "camera.h"
#include "global.h"
#include "load_mdl.h"
#include "mem.h"
#include "drawing.h"
#include <stdio.h>
#include <GLUT/glut.h>

#define EDIT_ROT_AMOUNT 5.0


typedef struct frame frame;

/**
 * The editor version of an animation is slightly different to the world
 * version. The editor version is kept as a linked list of frame nodes. This
 * is mostly to keep editing simple.
 */
typedef struct edit_anim
{ 
  int n_frames;
  int frame_width;

  struct frame {
    float *rots;
    int time_int;
    struct frame *next;
  } *head;
} edit_anim;


edit_anim *e_anim;              /* Editing version of the animation. */
frame *this_frame;              /* Currently selected frame. */
int frame_index = 0;            /* Current frame number. */
int curr_bone;                  /* Currently selected bone index. */
model *mdl;                     /* Model we're editing. */
int time_int = 200;             /* Interval between frames. */
char edit_string[32];           /* String for displaying info. */


/**
 * Initializes the editor.
 */
void edit_init()
{
  camera *cam;

  NEW(e_anim);
  CHECK_NR(e_anim);

  mdl = load_model("data/model/bird.mdl");
  if(!mdl)
  {
    fprintf(stderr, "ERROR(edit_init): Unable to load model.\n");
    exit(1);
  }
  mdl->pos[4] = 90;

  e_anim->n_frames    = 0;
  e_anim->frame_width = mdl->n_bones;
  e_anim->head        = NULL;

  global.r_skybox = false;
  global.r_grass  = false;
  global.r_bones  =  true;

  cam = cam_get();
  cam->pos[P_Z] = 32.0;
  cam->pos[P_Y] = 16.0;

  glLineWidth(3.0);
  this_frame = NULL;

  draw_model_register(mdl);
  set_curr_bone(mdl->bone_array[curr_bone]);
}


/**
 * Updates the current model.
 */
void edit_update(int now)
{
  animate(mdl, now);
}


/**
 *
 */
char *edit_get_string()
{
  sprintf(edit_string, "Frame %d of %d.", frame_index, e_anim->n_frames);
  return edit_string;
}


/**
 * Deallocate memory created through the editor.
 */
void edit_cleanup()
{
  frame *curr_frame, *next_frame;

  if(!e_anim) return;

  curr_frame = e_anim->head;

  while(curr_frame != NULL)
  {
    next_frame = curr_frame->next;
    FREE(curr_frame->rots);
    FREE(curr_frame);
    curr_frame = next_frame;
  }

  FREE(e_anim);
}


/**
 * Adds a new frame onto the end of the animation.
 */
void edit_add_frame()
{
  frame *new_frame, *curr_frame;

  NEW(new_frame);
  CHECK_NR(new_frame);

  new_frame->rots = skel_get_frame(mdl->bone_array, mdl->n_bones);
  new_frame->time_int = time_int;
  new_frame->next = NULL;
  e_anim->n_frames++;
  frame_index = e_anim->n_frames;
  
  curr_frame = e_anim->head;
  if(!curr_frame)
    e_anim->head = new_frame;
  else
  {
    while(curr_frame->next != NULL)
      curr_frame = curr_frame->next;

    curr_frame->next = new_frame;
  }
  this_frame = new_frame;
}


/**
 * Attempts to a delete a frame out of the animation struct which is the
 * same as the one passed in.
 */
void edit_del_frame(frame *trash)
{
  frame *curr_frame;

  if(!trash) return;

  if(e_anim->head == trash)
  {
    e_anim->head = trash->next;
    FREE(trash->rots);
    FREE(trash);
    e_anim->n_frames--;
    return;
  }

  curr_frame = e_anim->head;
  while(curr_frame)
  {
    if(curr_frame->next == trash)
    {
      curr_frame->next = trash->next;
      FREE(trash->rots);
      FREE(trash);
      e_anim->n_frames--;
      return;
    }

    curr_frame = curr_frame->next;
  }
}


/**
 * Keyboard bindings for the editor interface.
 */
void edit_keyboard(unsigned char key, int mx, int my)
{
  bone *bone = mdl->bone_array[curr_bone];
  camera *cam = cam_get();

  switch(key)
  {
    case ' ':
      edit_save_anim("anim.out");
      break;

    case 'k':
      if(!this_frame) return;
      if(this_frame->next)
      {
        this_frame = this_frame->next;
        frame_index++;
      }
      else
      {
        this_frame = e_anim->head;
        frame_index = 1;
      }
      if(this_frame)
        skel_set_rots(mdl->bone_array, this_frame->rots, mdl->n_bones);
      break;

    case 'l':
      edit_add_frame();
      break;
    case ';':
      edit_del_frame(this_frame);
      this_frame = e_anim->head;
      frame_index = (e_anim->head ? 1 : 0);
      if(this_frame)
        skel_set_rots(mdl->bone_array, this_frame->rots, mdl->n_bones);
      break;

    case ',':
      curr_bone--;
      if(curr_bone < 0)
        curr_bone = mdl->n_bones - 1;
      break;
    case '.':
      curr_bone++;
      if(curr_bone > mdl->n_bones)
        curr_bone = 0;
      break;

    case 'x':
      if(bone) bone->rot[RZ] += EDIT_ROT_AMOUNT;
      break;
    case 'z':
      if(bone) bone->rot[RZ] -= EDIT_ROT_AMOUNT;
      break;
    case 's':
      if(bone) bone->rot[RY] += EDIT_ROT_AMOUNT;
      break;
    case 'a':
      if(bone) bone->rot[RY] -= EDIT_ROT_AMOUNT;
      break;
    case 'w':
      if(bone) bone->rot[RZ] += EDIT_ROT_AMOUNT;
      break;
    case 'q':
      if(bone) bone->rot[RZ] -= EDIT_ROT_AMOUNT;
      break;

    /* Dodgy zooming. Not very smooth but for the purposes of the editor
     * it does the job fine. */
    case '+':
      cam->pos[P_Z] -= 1.0;
      break;
    case '-':
      cam->pos[P_Z] += 1.0;
      break;
  }

  set_curr_bone(mdl->bone_array[curr_bone]);
}


/**
 * Saves the current animation to the file filename.
 */
void edit_save_anim(const char *filename)
{
  FILE *outfile;
  struct frame *c_frame;
  int i;

  outfile = fopen(filename, "w");
  if(!outfile)
  {
    fprintf(stderr,
        "ERROR(edit_save_anim): Opening file %s for output.\n", filename);
    return;
  }

  fprintf(outfile, "a %d\n", e_anim->n_frames);
  for(c_frame = e_anim->head; c_frame; c_frame = c_frame->next)
  {
    fprintf(outfile, "f %d", c_frame->time_int);
    for(i = 0; i < e_anim->frame_width; i++)
      fprintf(outfile, " %.2f %.2f %.2f", c_frame->rots[3 * i],
          c_frame->rots[3 * i + 1], c_frame->rots[3 * i + 2]);
    fprintf(outfile, "\n");
  }

  fclose(outfile);
  printf("Animation saved, %i frames.\n", e_anim->n_frames);
}

