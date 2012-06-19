/**
 * load_mdl.c
 *
 * Handles the loading of model and animation data. This code is not very
 * bullet-proof and should really ONLY read in files in the correct format.
 * Segmentation faults on startup will nearly always be related to this file
 * some way.
 */

#include "load_mdl.h"


/* Function prototypes. */
bool load_animation(model *mdl, int frames, FILE *fp);
bool parse_frame(anim *anim, char *frame, int bones);


/**
 * Takes a string and seperates it into smaller strings. These strings are
 * seperated by the supplied delimiter. The resulting smaller strings are
 * stored in an array of char pointers, pointed to by args. The number of
 * pointers avaliable should also be passed in.
 * Any pointers not set to arguments within the string will be set to NULL.
 */
int makeargs(char *buffer, char delim, char **args, int arg_size)
{
  char *cchar;
  int arg_count = 0, i;

  if(strlen(buffer) == 0)
    return 0;

  /**
   * Initialize the first arg and the pointer to the start of the buffer
   * string.
   */
  args[0] = cchar = buffer;
  arg_count++;

  /**
   * Loop until the end of the string is found.
   */
  while(cchar != NULL)
  {
    cchar = strchr(cchar, delim);

    /* If cchar is NULL then the end of the string has been reached. */
    if(cchar != NULL)
    {
      *cchar = '\0';
      arg_count++;
      cchar++;

      if(arg_count < arg_size)
        args[arg_count - 1] = cchar;
    }
  }

  /**
   * Lastly clear any pointers that weren't set in the above loop.
   */
  if(arg_count < arg_size)
    for(i = arg_count; i < arg_size; i++)
      args[i] = NULL;

  return arg_count;
}


/**
 * Loads a model from a file.
 * Returns a pointer to the bone struct containing the data loaded.
 */
model *load_model(const char *file_name)
{
  FILE *fp;
  char buffer[BUFF_LEN], *arg_list[MAX_ARGS];
  int i, line = 0, arg_count;
  mesh *geo;
  bone *b_new;
  model *new_mdl = NULL;


  /* Attempt to open the requested file. Prints error message on error. */
  if((fp = fopen(file_name, "r")) == NULL)
  {
    fprintf(stderr, "ERROR(load_model): Unable to open file '%s'.\n",
      file_name);
    return NULL;
  }

  while(fgets(buffer, BUFF_LEN - 1, fp) != NULL)
  {
    line++;

    /* No lines should be more then the max buffer size in length. If any
     * lines do breach this contraint, then the file is corrupt and will
     * not be loaded. */
    if(buffer[strlen(buffer) - 1] != '\n')
    {
      fprintf(stderr,
        "ERROR(load_model): Buffer overflow in file '%s' line %d\n", file_name,
        line);
      return NULL;
    }
    buffer[strlen(buffer) - 1] = '\0';

    arg_count = makeargs(buffer, SEPERATOR, arg_list, MAX_ARGS);
    if(arg_count == 0)
      continue;


    /* If there's an m as the first character (which there should be at the
     * head of each model file) then we prepare the model.  */
    if(buffer[0] == 'm')
    {
      if(arg_count != 4)
        continue;

      /* Create a new model struct. */
      new_mdl = new_model(arg_list[1], atoi(arg_list[2]));
      if(new_mdl == NULL) return NULL;

      /* Initialise all the new models variables. */
      new_mdl->texture = loadTexture(arg_list[3]);

      printf("New model '%s' created. Loading Model...\n", arg_list[1]);
    }


    /* If we have a bone, indicated by a b as the first character. Note that
     * bones can only be added if a model line has been read in. */
    else if(buffer[0] == 'b')
    {
      if(new_mdl == NULL)
      {
        fprintf(stderr,
          "ERROR(load_mdl): Attempted load bone without model decl.\n");
        continue;
      }
      if(arg_count < 7) continue;

      printf("Loading bone '%s' into model '%s'.\n", arg_list[1],
          new_mdl->name);

      new_mdl->n_bones++;
      /* Allocate memory for the bone object and various chores that need
       * to be done to the bone to keep it healthy. */
      b_new = malloc(sizeof(bone));
      if(b_new == NULL)
      {
        fprintf(stderr, "ERROR(load_model): Cannot allocate memory.\n");
        exit(1);
      }
      b_new->child = NULL;
      b_new->sibling = NULL;
      b_new->child_count = 0;

      /**
       * Allocate memory for the bone name and copy the string accross.
       */
      b_new->name = malloc(strlen(arg_list[1]));
      strcpy(b_new->name, arg_list[1]);

      /* Parse translations from the strings and put them into the bone's
       * translation struct. */
      for(i = 0; i < TRANS_SIZE; i++)
        b_new->rot[i] = atof(arg_list[i + 2]);

      b_new->length = atof(arg_list[5]);

      /* Attempt to load a new mesh into the current bone.  */
      if(strlen(arg_list[7]) > 0)
      {
        geo = load_obj(arg_list[7]);
        if(geo == NULL)
        {
          fprintf(stderr, "Error loading obj %s\n", arg_list[7]);
          exit(1);
        }

        /* Create a vertex array out of the loaded obj and free the mesh
         * struct that was temporarily created. */
        if(strlen(arg_list[8]) > 0 && arg_list[8][0] == 'F')
          b_new->geometry = mesh_to_array(geo, NORM_FLAT);
        else
          b_new->geometry = mesh_to_array(geo, NORM_SMOOTH);
        b_new->tri_count = geo->n_elements;
        free_mesh(geo);
      }
      else
        b_new->geometry = NULL;

      /* If the root bone has not been set then this is obviously the root
       * bone, or a file format error. */
      if(new_mdl->root == NULL)
      {
        if(strlen(arg_list[6]) == 0)
          new_mdl->root = b_new;
        else
        {
          fprintf(stderr,
            "ERROR(load_model): Error, root bone must not have parent.\n");
          free_model(new_mdl);
          return NULL;
        }
      }
      else
      {
        if(!skel_add_child(arg_list[6], new_mdl->root, b_new))
        {
          printf("Cannot add child %s.\n", b_new->name);
          free_model(new_mdl);
          return NULL;
        }
      }
    }


    /* Start of an animation. */
    else if(buffer[0] == 'a')
    {
      if(new_mdl == NULL)
      {
        fprintf(stderr,
          "ERROR(load_mdl): Attempted load animation without model decl.\n");
        continue;
      }
      if(arg_count != 2) continue;

      load_animation(new_mdl, atoi(arg_list[1]), fp);
    }
  }

  if(new_mdl != NULL)
    new_mdl->bone_array = skel_make_array(new_mdl->root, new_mdl->n_bones);

  printf("New model '%s' successfully loaded.\n", new_mdl->name);
  printf("%d bones loaded.\n", new_mdl->n_bones);
  printf("%d animations loaded.\n", new_mdl->n_anims);

  return new_mdl;
}


/**
 * Loads a set of frames from a file into a model. Returns true on success.
 */
bool load_animation(model *mdl, int frames, FILE *fp)
{
  int i;
  anim *anim;
  char buffer[BUFF_LEN];

  if(!mdl || frames <= 0 || !fp)
    return false;


  /* Find the first empty animation inside the model and create a new
   * animations, assigning the spare slot to it. */
  for(i = 0 ; i < mdl->n_anims; i++)
  {
    if(mdl->anims[i] == NULL)
    {
      printf("Creating new animation, %d frames, %d bones\n", frames,
          mdl->n_bones);
      anim = new_anim(frames, mdl->n_bones);
      mdl->anims[i] = anim;
      break;
    }
    /* If no slots are avaliable then return false. */
    else if(i == mdl->n_anims - 1)
      return false;
  }

  /* Continue reading in files until all the frames have been read in. */
  i = 0;
  while(fgets(buffer, BUFF_LEN - 1, fp) != NULL) 
    if(parse_frame(anim, buffer, mdl->n_bones))
      if(++i == frames)
        break;

  if(i != frames)
    return false;

  return true;
}


/**
 * Takes a string and parses it into an animation frame which is inserted
 * into the provided animation. This function does not count the values and
 * malloc enough space, you must tell it how much space is required.
 */
bool parse_frame(anim *anim, char *frame, int bones)
{
  float *values;
  int count = 0, timeint;
  char *token;
  bool ret_value;

  /* Check for a valid frame string. */
  if(!frame || frame[0] != 'f') return false;

  token = strtok(frame, " \n");
  timeint = atoi(strtok(NULL, " \n"));
  if(timeint <= 0) return false;

  /* Allocate enough memory to hold onto our frame. 3 floats per bone. */
  values = malloc(sizeof(float) * bones * 3);
  if(values == NULL) return false;

  while((token = strtok(NULL, " \n")) != NULL)
  {
    values[count] = atof(token);
    count++;
  }

  /* Check that the correct amount of values were extracted. */
  if(count % 3 != 0 || count / 3 != bones)
  {
    free(values);
    return false;
  }

  /* Add a new frame to the animation. */
  ret_value = anim_new_frame(anim, values, timeint);
  if(!ret_value) printf("Frame Error!\n");
  return ret_value;
}


