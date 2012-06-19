/**
 * 3d.h
 *
 * Contains all the structs and function definitions that a client would
 * require if they were wanting to use the '3d package' that this framework
 * employs. Includes all required structure definitions and function
 * prototypes.
 */

#ifndef _3D_H_
#define _3D_H_

#include <stdlib.h>

#include "global.h"

/**
 * These definitions aid in the use of the rot struct. In essence they allow
 * a client to treat a rot less as an array and more as an object.
 */
#define TRANS_SIZE 3

enum { X, Y, Z };
enum { RX, RY, RZ };

/**
 * A rot struct stores a set of angles which are used to rotate an object in
 * 3 dimensions.
 */
typedef float rot[TRANS_SIZE];


/**
 * Maximum number of children that a child may have.
 */
#define BONE_CHILD_MAX 16

/**
 * bone structs are the buildng blocks of the animation system used throughout
 * this framework. The bones form a tree like structure which is used to
 * represent their heiarchy. Each bone stores a set of rotations in 3d and
 * its length. These rotations are relative to their parent.
 */
typedef struct _bone bone; /* Typedef here as it needs to refer to itself. */

struct _bone
{
   char* name;              /* Human readable name for the bone. */
   
   /**
    * Reference to the bone's first child and their sibling. Note that in this
    * struct, the child is only the first child of the parent, other children
    * are the siblings of this child.
    */
   bone *child;             /* First child. */
   bone *sibling;           /* Next sibling. */

   short child_count;       /* Number of children beneath this bone. */

   rot rot;                 /* Current rotation relative to parent. */
   float length;            /* Current length in the pos x axis. */

   float *geometry;         /* Vertex array of geometry for the bone. */
   int tri_count;           /* Number of triangles in the array. */
};


/**
 * The animation struct is really just a 2D array of float triplets. These
 * store rotations for the bones of a model. Actual animation state
 * tracking is done inside the model struct below.
 */
typedef struct _anim
{
  float **key_frames;           /* 2D array of frames. */
  int *times;                   /* 1D array holding time data. */

  int n_frames;                 /* Height of the 2D array. */
  int n_bones;                  /* Width of the 2D array. */
} anim;


/** 
 * The model struct encapsulates all data to do with a specific instance
 * of an object that has a skeleton and geometry. It also stores its own
 * animation data and tracks animation state.
 */
typedef struct _model
{
  char *name;               /* Human readable name for the model. */

  float pos[6];             /* Position and rotation of the model. */

  bone *root;               /* Root of the skeleton. */
  bone **bone_array;        /* Array of pointers at the skeletons bones. */
  int n_bones;              /* Number of bones in the skeleton. */

  int texture;              /* Reference to OpenGL Texture object. */

  float *n_frame;           /* A pointer to the next animation frame. */
  float *p_frame;           /* A pointer to the previous animation frame. */
  int p_index, n_index; 
  int p_time, n_time;       /* Next/Previous actual times. */

  anim **anims;             /* An array of pointers to animation structs. */
  anim *curr_anim;          /* Pointer the current animation. */
  int n_anims;              /* Number of held animations. */

} model;


/**
 * The vf_node is the way that we track what faces use which vetrtecies. The
 * point of doing so is that when calculating the normals for a mesh object,
 * we need to know what faces to average.
 */
typedef struct _vf_node vf_node;

struct _vf_node
{
  int face;                     /* Index of face in mesh array. */
  vf_node *next;                /* Next list node. */
};

typedef struct _vf_head
{
  int nodes;                    /* Number of faces for this vertex. */
  vf_node *head;                /* First face. */
} vf_head;

/* Enum for controlling normal loading. */
enum { NORM_SMOOTH, NORM_FLAT };

/**
 * Mesh structure. Stores information about a geometric model. Will generally
 * be converted into an array for drawing before the main program starts.
 */
typedef struct _mesh
{
  float *v;                 /* Vertices. */
  float *vn;                /* Vertex Normals. */
  float *vt;                /* Vertex Texture Coordinates. */

  int *faces;               /* Faces. */

  int n_v;                  /* Count of various elements. */
  int n_vt;
  int n_faces;
  int n_elements;           /* Number of elements all together in the
                               mesh. Generally will be 3 * faces. */

  int c_v;                  /* Current last filled index of types. */
  int c_vn;
  int c_vt;
  int c_face;
  int c_face_index;

  vf_head *vf;              /* Tracks which faces use which vertexes. */

} mesh;

/* 3D Interface. */

/* animation.c functions. */
extern anim *new_anim(int frames, int bones);
extern void free_anim(anim *trash, bool del_frames);
extern void bone_shallow_free(bone *bone);
extern bool anim_new_frame(anim *curr, float *key_frame, int time_int);
extern void animate(model *mdl, int now);
extern bool start_animation(model *mdl, int index, int start_time);
extern float int_linear(float x0, float y0, float x1, float y1, float x);
extern float int_linear_quick(float a, float y0, float y1);
extern void int_keyframes(model *model, int now);

/* bone.c functions. */
extern int bone_add_child(bone *parent, bone *child);
extern bone *skel_find_bone(bone *skel, const char *name);
extern bool skel_add_child(const char* name, bone *root, bone *child);
extern void free_skel(bone *skel);
extern void free_bone(bone *bone);
extern void skel_shallow_free(bone *skel);
extern bone **skel_make_array(bone *skel, int array_size);
extern float *skel_get_frame(bone **bone_array, int n_bones);
extern void skel_set_rots(bone **bone_array, float *rots, int n_bones);
extern bone *clone_skel(bone *skel);

/* mesh.c functions */
extern mesh *new_mesh();
extern float *mesh_to_array(mesh *geo, int type);
extern void free_mesh(mesh *geo);
extern model *new_model();
extern void free_model(model *mdl);
extern void model_shallow_free(model *mdl);
extern void model_info(model *mdl);
extern model *clone_model(model *mdl);


#endif

