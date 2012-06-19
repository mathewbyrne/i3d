/**
 * bone.c
 */

#include "3d.h"
#include "global.h"
#include "mem.h"
#include "util.h"

#include <stdio.h>
#include <string.h>


/**
 * Adds a new child bone to the parent. If this number is greater than
 * BONE_CHILD_MAX then the bone is not added as a child and false is returned.
 * Function returns true if the bone is added, false otherwise.
 * Note: the use of double pointers in this function to GREATLY simplyfy the
 * code using linked lists.
 */
int bone_add_child(bone *parent, bone *child)
{
   bone **cur_bone = &(parent->child);

   if(parent->child_count >= BONE_CHILD_MAX)
      return 0;

   while(*cur_bone != NULL)
      cur_bone = &((*cur_bone)->sibling);

   *cur_bone = child;
   parent->child_count++;

   return 1;
}


/**
 * Searches a skeleton to find a bone with a particular name. NULL is returned
 * if no bone is found. This function recursively searches a skeleton.
 */
bone *skel_find_bone(bone *skel, const char *name)
{
   bone *found;

   /* Not interested in empty references. */
   if(!skel)
      return NULL;

   /* If this bone has the right name then return it. */
   if(streq(name, skel->name))
      return skel;

   /* Otherwise, search through siblings, if the bone is found in them then
    * return it. */
   found = skel_find_bone(skel->sibling, name);
   if(found != NULL)
      return found;

   found = skel_find_bone(skel->child, name);
   if(found != NULL)
      return found;

   return NULL;
}


/**
 * Adds a child to a skeleton by finding the bone with the given name and
 * adding the provided bone as a child. Returns TRUE if the bone is found and
 * the child has been added.
 */
bool skel_add_child(const char *name, bone *root, bone *child)
{
   bone *parent = NULL;
   parent = skel_find_bone(root, name);

   /* Will only return true if a parent is found and the child is successfully
    * added to the skeleton.  */
   if(parent != NULL)
      if(bone_add_child(parent, child))
         return true;

   return false;
}


/**
 * Frees all dynamically allocated memory associated with a particular
 * skeleton. Will free all the children/sibling of a bone.
 */
void free_skel(bone *skel)
{
  if(!skel) return;

  free_skel(skel->child);
  free_skel(skel->sibling);

  free_bone(skel);
}


/**
 * Shallow version of skel free. So that things don't get double free'd.
 */
void skel_shallow_free(bone *skel)
{
  if(!skel) return;

  skel_shallow_free(skel->child);
  skel_shallow_free(skel->sibling);
  
  FREE(skel);
}


/**
 * Frees all dynamically allocated memory associated with a particular bone.
 * Generally not for public use as it doesn't care about children or
 * siblings, just frees the bone passed to it. EASY WAY TO GET LEAKS!
 */
void free_bone(bone *bone)
{
  FREE(bone->name);
  FREE(bone->geometry);
  FREE(bone);
}


/**
 * Private function used in skel_make_array to simply recurse over a given
 * skeleton. The use of the integer point below is so that it's properly
 * incremented throughout the process.
 */
void skel_array(bone **array, bone *curr, int array_size, int *count)
{
  if(!curr) return;

  /* Make sure that we don't exceed the array limit and start to get
   * nasties at run-time. */
  if(*count == array_size)
  {
    fprintf(stderr, "ERROR(skel_array): Array limit reached.\n");
    return;
  }

  /* Actual get the pointer from the current bone and assign it the the
   * current array element. Also increment the position counter. */
  array[*count] = curr;
  (*count)++;

  /* Depth first! So children first then siblings. Seems to be the
   * easiest way to assign things. Makes a bit more sense Breadth first but
   * it's a minor point for this assignment.  */
  skel_array(array, curr->child, array_size, count);
  skel_array(array, curr->sibling, array_size, count);
}


/**
 * Creates an array of bone pointers which point to elements in a skeleton
 * tree. The tree is assigned to the array depth first and the first element
 * will always be the root of the skeleton.
 */
bone **skel_make_array(bone *skel, int array_size)
{
  bone **b_array, *curr_bone = skel;
  int count = 0;

  if(skel == NULL || array_size <= 0)
  {
    fprintf(stderr, "ERROR(skel_make_array): Invalid Arguments.\n");
    return NULL;
  }

  b_array = malloc(sizeof(bone *) * array_size);
  if(b_array == NULL)
  {
    fprintf(stderr, "ERROR(skel_make_array): Unable to allocate memory.\n");
    return NULL;
  }

  /* Start the recursion over the skeleton, assigning pointers to the
   * various skeleton elements to the array. */
  skel_array(b_array, curr_bone, array_size, &count);

  return b_array;
}


/**
 * Creates an array of floats which represent all the rotations in a bone
 * array. The bone array is something gotten using skel_make_array.
 */
float *skel_get_frame(bone **bone_array, int n_bones)
{
  int i;
  float *frame;

  CHECK(bone_array);

  frame = malloc(sizeof(float) * n_bones * 3);

  for(i = 0; i < n_bones; i++)
  {
    frame[3 * i + 0] = bone_array[i]->rot[0];
    frame[3 * i + 1] = bone_array[i]->rot[1];
    frame[3 * i + 2] = bone_array[i]->rot[2];
  }

  return frame;
}


/**
 * Sets a skeletons rotations to a set of rotations specified in the 
 * rots array.
 */
void skel_set_rots(bone **bone_array, float *rots, int n_bones)
{
  int i;

  if(!bone_array  || !rots)
    return;
  
  for(i = 0; i < n_bones; i++)
    v_copy(bone_array[i]->rot, rots + (3 * i));
}


/**
 * Returns a shallow copy of a bone and all it's children. Most elements are
 * not copied but their references copied instead. This is mostly to save on
 * resources.
 */
bone *clone_skel(bone *skel)
{
  int i;
  bone *clone;

  if(skel == NULL) return NULL;
  
  clone = malloc(sizeof(bone));
  if(clone == NULL) return NULL;

  /* Copy main fields. */
  clone->name        = skel->name;
  clone->length      = skel->length;
  clone->child_count = skel->child_count;
  clone->geometry    = skel->geometry;
  clone->tri_count   = skel->tri_count;

  /* Copy rotations. */
  for(i = 0; i < TRANS_SIZE; i++)
    clone->rot[i] = skel->rot[i];

  /* Recurse the rest of the skeleton tree. */
  clone->child   = clone_skel(skel->child);
  clone->sibling = clone_skel(skel->sibling);

  return clone;
}

