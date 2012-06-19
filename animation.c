/**
 * animation.c
 *
 * Contains all the functions relevant to animations, creating and making sure
 * that their contents are consistent as well as actual functions to animate
 * skeletons.
 */

#include "3d.h"
#include <math.h>

/**
 * Creates a new anim struct, initialises it and allocates memory needed
 * initially. Sets all members to appropriate values.
 */
anim *new_anim(int frames, int bones)
{
  anim *new;
  int i;

  /* Attempt to allocate some memory. If the memory was not successfully
   * allocated then return NULL.  */
  new = malloc(sizeof(anim));
  if(new == NULL) return NULL;

  /* Allocate memory for other parts of the struct. Again, return null on
   * an error and make sure that no memory is leaked by returning null at
   * this point in the code. */
  new->key_frames = malloc(sizeof(float *) * frames);
  new->times = malloc(sizeof(int) * frames);
  if(new->key_frames == NULL || new->times == NULL)
  {
    if(new->key_frames != NULL) free(new->key_frames);
    if(new->times != NULL) free(new->times);

    free(new);
    return NULL;
  }

  /* Initialise all frames to null.  */
  for(i = 0; i < frames; i++)
  {
    new->key_frames[i] = NULL;
    new->times[i] = 0;
  }

  /* Initialise the other variables in the struct. */
  new->n_frames = frames;
  new->n_bones = bones;

  return new;
}


/**
 * Frees all memory associated with a particular animation struct.
 */
void free_anim(anim *trash, bool del_frames)
{
  int i;

  /* Make sure we have something to throw out. */
  if(trash == NULL) return;

  /* Loop through all key frames, making sure to destroy any arrays we
   * come accross which are not NULL.  */
  if(del_frames)
  {
    if(trash->key_frames != NULL)
    {
      for(i = 0; i < trash->n_frames; i++)
        if(trash->key_frames[i] != NULL)
          free(trash->key_frames[i]);

      free(trash->key_frames);
    }
  }

  /* Free the list of times and finally the animation itself. */
  if(trash->times != NULL)
    free(trash->times);
  free(trash);
}


/**
 * Adds a new keyframe to an animation and copies all translation values
 * out of an array of translations.
 * Returns bool true on success, false otherwise.
 */
bool anim_new_frame(anim *curr, float *new_frame, int time_int)
{
  int i, index = -1;

  if(time_int <= 0) return false;

  /* Find first empty frame in the frame list. */
  for(i = 0; i < curr->n_frames; i++)
  {
    if(curr->key_frames[i] == NULL)
    {
      index = i;
      break;
    }
  }

  /* Frame must be full if no empty value was fond. */
  if(index < 0) return false;

  /* Set the new values. */
  curr->key_frames[index] = new_frame;
  curr->times[index] = time_int;

  return true;
}


/**
 * Adjusts the bones in a skeleton so that they are at an appropritae
 * interpolated point between two animation key_frames using the type of
 * interpolation specified.
 */
void animate(model *mdl, int now)
{
  anim *anim = mdl->curr_anim;
  
  /* If we're pointing to the same frame then we generally do not want to
   * animate anything, so return. */
  if(mdl->n_index == mdl->p_index) return;
  
  /* First we need to update the animation loop to the current time. This
   * loop should correctly forward an animation till it gets to the present
   * time. */
  if(now >= mdl->n_time)
  {
    while(now >= mdl->n_time)
    {
      mdl->p_time  = mdl->n_time;
      mdl->n_time += anim->times[mdl->n_index];
      mdl->p_index = mdl->n_index;
      mdl->n_index = (mdl->n_index + 1) % anim->n_frames;
      mdl->p_frame = anim->key_frames[mdl->p_index];
      mdl->n_frame = anim->key_frames[mdl->n_index];
    }
  }

  int_keyframes(mdl, now);
}


/**
 * Starts a new animation for the given model.
 */
bool start_animation(model *mdl, int index, int start_time)
{
  if(index >= mdl->n_anims || !mdl->anims[index])
    return false;


  /* There are a few assumptions made here regarding having a valid
   * animation struct. An invalid structure may not function correctly.*/
  if(mdl->curr_anim == NULL || mdl->anims[index] == mdl->curr_anim)
  {
    mdl->curr_anim = mdl->anims[index];

    mdl->n_index =  1 % mdl->curr_anim->n_frames;
    mdl->p_index = 0;

    mdl->n_time = mdl->curr_anim->times[mdl->n_index] + start_time;
    mdl->p_time = start_time;

    mdl->n_frame = mdl->curr_anim->key_frames[mdl->n_index];
    mdl->p_frame = mdl->curr_anim->key_frames[mdl->p_index];
    skel_set_rots(mdl->bone_array, mdl->n_frame, mdl->n_bones);
  }
  /* There is currently another animation running, so switch between them. */
  else
  {
    mdl->curr_anim = mdl->anims[index];
    mdl->n_index = 0;
    mdl->n_time = start_time + mdl->curr_anim->times[mdl->n_index];
  }

  return true;
}


/**
 * Linear Interpolation.
 *
 * equation source: en.wikipedia.org/wiki/Linear_Interpolation
 *
 * Returns the solution for y on a straight line between 2 points (x0, y0) and
 * (x1, y1).
 * the 
 */
float int_linear(float x0, float y0, float x1, float y1, float x)
{
   return (((x - x0) / (x1 - x0)) * (y1 - y0)) + y0;
}


/**
 * A simpler version of the above linear interpolation function. For use when
 * the value of a has been pre-calculated, potentially for efficientcy reasons.
 */
float int_linear_quick(float a, float y0, float y1)
{
   return (a * (y1 - y0)) + y0;
}


/**
 * Linearly interpolates between keyframes of an animation for a given
 * model.
 */
void int_keyframes(model *mdl, int now)
{
  float a = (now - mdl->p_time) / (float)(mdl->n_time - mdl->p_time);
  float *from = mdl->p_frame;
  float *to = mdl->n_frame;
  int i, j;
  bone *curr_bone;

  /* Loop through all bones in a model. */
  for(i = 0; i < mdl->n_bones; i++)
  {
    curr_bone = mdl->bone_array[i];

    /* Actual interpolation happens here. */
    for(j = 0; j < TRANS_SIZE; j++)
    {
      if(curr_bone->rot[j] != *to)
       curr_bone->rot[j] = int_linear_quick(a, *from, *to);

      from++; to++;
    }
  }

  return;
}

