/**
 * mesh.c
 *
 * Despite the name, contains function for both mesh and model objects.
 */

#include "3d.h"
#include "util.h"
#include "mem.h"

#include <stdio.h>
#include <string.h>

#define STRIDE 8


/**
 * Creates a new mesh struct and initialises it to resonable values.
 * Returns NULL on failure. This does allocate new memory so the return
 * value SHOULD be free()d at a later date.
 */
mesh *new_mesh()
{
  mesh *new_mesh;

  NEW(new_mesh);
  CHECK(new_mesh);

  new_mesh->v = new_mesh->vn = new_mesh->vt = NULL;
  new_mesh->faces = NULL;
  new_mesh->n_v = new_mesh->n_vt = new_mesh->n_faces = 0;
  new_mesh->n_elements = 0;
  new_mesh->c_v = new_mesh->c_vn = new_mesh->c_vt = new_mesh->c_face = 0;
  new_mesh->c_face_index = 0;

  return new_mesh;
}


/**
 * Calculates a set of smooth or faccetted normals from a given mesh
 * object. The resulting normals are placed into the float array given.
 */
void calc_normals(mesh *geo, float *arr, int type)
{
  int i, j, fn;
  float *face_norms;
  float *vert_norms;
  float v0[3], v1[3];
  vf_node *temp;

  face_norms = malloc(sizeof(float) * geo->n_faces * 3);
  if(face_norms == NULL) return;

  /* Calculate face normals first. */
  for(i = 0; i < geo->n_faces; i++)
  {
    j  = i * 6;
    fn = i * 3;

    /* First find 2 vectors parralel with the face. */
    v_sub(v0, geo->v + geo->faces[j + 0] * 3,
              geo->v + geo->faces[j + 2] * 3);
    v_sub(v1, geo->v + geo->faces[j + 2] * 3,
              geo->v + geo->faces[j + 4] * 3);

    /* Find and normalize the cross product. */
    v_cross(face_norms + fn, v0, v1);
    v_norm(face_norms + fn);
  }

  if(type == NORM_SMOOTH)
  {
    vert_norms = malloc(sizeof(float) *  geo->n_v * 3);
    if(vert_norms == NULL) return;

    for(i = 0; i < geo->n_v; i++)
    {
      temp = geo->vf[i].head;

      v_clear(vert_norms + 3 * i);

      while(temp != NULL)
      {
        vert_norms[3 * i + 0] += face_norms[3 * temp->face + 0];
        vert_norms[3 * i + 1] += face_norms[3 * temp->face + 1];
        vert_norms[3 * i + 2] += face_norms[3 * temp->face + 2];

        temp = temp->next;
      }

      v_norm(vert_norms + 3 * i);
    }

    geo->vn = vert_norms;
  }
  else if(type == NORM_FLAT)
  {
    for(i = 0; i < geo->n_faces * 3; i += 3)
    {
      v_copy(arr + (i + 0) * STRIDE + 2, face_norms + i);
      v_copy(arr + (i + 1) * STRIDE + 2, face_norms + i);
      v_copy(arr + (i + 2) * STRIDE + 2, face_norms + i);
    }
  }

  free(face_norms);
}


/**
 * Converts a mesh object to an array of floats for use with 
 * glInterleavedArray. Because of the way this is implemented, the values are
 * to be in the following order:
 * {VT1, VT2, VN1, VN2, VN3, V1, V2, V3 ... }
 * This function also assumes a valid mesh is provided, ie does not check
 * bounds of vertex, normals etc.
 */
float *mesh_to_array(mesh *geo, int type)
{
  int i, c, f;
  float *mesh_array = malloc(sizeof(float) * STRIDE * geo->n_elements);

  if(!mesh_array) return NULL;

  calc_normals(geo, mesh_array, type);

  /* This nasty looking for loop is really just a way of fiddling around
   * with the values to get them into the right positions. */
  for(i = 0, c = 0; i < geo->n_elements; i++)
  {
    f = 2 * i;

    /* Texture Coordinates. */
    mesh_array[c++] = geo->vt[geo->faces[f + 1] * 3];
    mesh_array[c++] = geo->vt[geo->faces[f + 1] * 3 + 1];

    /* Normal Vectors. */
    if(type == NORM_SMOOTH)
    {
      mesh_array[c++] = geo->vn[geo->faces[f] * 3];
      mesh_array[c++] = geo->vn[geo->faces[f] * 3 + 1];
      mesh_array[c++] = geo->vn[geo->faces[f] * 3 + 2];
    }
    else c += 3;

    /* Vertex points. */
    mesh_array[c++] = geo->v[geo->faces[f] * 3];
    mesh_array[c++] = geo->v[geo->faces[f] * 3 + 1];
    mesh_array[c++] = geo->v[geo->faces[f] * 3 + 2];
  }

  return mesh_array;
}


/**
 * Frees dynamically allocated memory that has been gathered for a mesh
 * object. Should be used with any mesh created from the new_mesh() function
 * above.
 */
void free_mesh(mesh *geo)
{
  int i;
  vf_node *temp, *temp2;

  if(geo == NULL) return;

  if(geo->v     != NULL) free(geo->v);
  if(geo->vn    != NULL) free(geo->vn);
  if(geo->vt    != NULL) free(geo->vt);
  if(geo->faces != NULL) free(geo->faces);

  /**
   * Free the linked list of face to vertex nodes.
   */
  if(geo->vf != NULL)
  {
    for(i = 0; i < geo->n_v; i++)
    {
      temp = geo->vf[i].head;
      while(temp != NULL)
      {
        temp2 = temp;
        temp = temp->next;
        free(temp2);
      }
    }
  }
  free(geo->vf);

  free(geo);
}


/**
 * Creates a new model object and sets it's values to initialised known
 * values. An optional name value can be supplied (NULL if no name). The
 * name will be copied in and space allocated for it.
 */
model *new_model(char *name, int anims)
{
  int i;
  model *mdl;
  
  NEW(mdl);
  CHECK(mdl);

  /* Create space for name if required. */
  if(!name)
    mdl->name = NULL;
  else
  {
    mdl->name =  malloc(sizeof(char) * strlen(name));
    if(mdl->name == NULL)
    {
      free(mdl);
      return NULL;
    }
    strcpy(mdl->name, name);
  }

  /* Init other variables. */
  mdl->root = NULL;
  mdl->bone_array = NULL;
  mdl->n_bones = mdl->n_anims = 0;
  mdl->p_frame = mdl->n_frame = NULL;
  mdl->p_index = mdl->n_index = 0;
  mdl->p_time = mdl->n_time = 0;
  mdl->curr_anim = NULL;

  if(anims < 0) anims = 1;
  
  mdl->n_anims = anims;
  mdl->anims = calloc(anims, sizeof(anim));
  if(mdl->anims == NULL)
  {
    free_model(mdl);
    return NULL;
  }

  for(i = 0; i < anims; i++)
    mdl->anims[i] = NULL;

  v_clear(mdl->pos);
  v_clear(mdl->pos + 3);

  return mdl;
}


/**
 * Frees all allocated memory associated with a supplied model. Also sets the
 * supplied pointer to NULL to avoid dangling pointers.
 */
void free_model(model *mdl)
{
  int i;

  if(!mdl) return;

  free_skel(mdl->root);
  for(i = 0; i < mdl->n_anims; i++)
    free_anim(mdl->anims[i], true);

  FREE(mdl->anims);
  FREE(mdl->bone_array);
  FREE(mdl->name);

  FREE(mdl);
}


/**
 * Performs a 'shallow' free, only frees the model and it's bones. Useful
 * when cloning birds with clone_model and then freeing them after.
 */
void model_shallow_free(model *mdl)
{
  if(!mdl) return;

  skel_shallow_free(mdl->root);

  FREE(mdl->anims);
  FREE(mdl->bone_array);
  FREE(mdl->name);

  FREE(mdl);
}


/**
 * Takes a pointer to a model clone and creates a new model. It's mostly
 * a shallow copy so pointers to meshes and such are maintained to
 * save resources.
 */
model *clone_model(model *mdl)
{
  model *clone;
  int i = 0;

  clone = new_model(mdl->name, mdl->n_anims);
  if(clone == NULL) return NULL;

  clone->root       = clone_skel(mdl->root);
  clone->n_bones    = mdl->n_bones;
  clone->bone_array = skel_make_array(clone->root, clone->n_bones);
  clone->texture    = mdl->texture;

  v_clear(clone->pos);
  v_clear(clone->pos + 3);

  for(i = 0; i < mdl->n_anims; i++)
  {
    if(!mdl->anims[i]) break;
    clone->anims[i] = mdl->anims[i];
  }

  return clone;
}

