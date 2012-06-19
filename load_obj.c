/**
 * load_obj.c
 *
 * Implementation of my simple obj file loader.
 */

#include "load_obj.h"


/**
 * Function prototypes - these functions are generally not for use outside
 * this source file, and often depend on previous functions to work
 * correctly.
 */
mesh *ready_mesh(FILE *infile);
void parse_line(char *line, mesh *geo);
void get_vertex(mesh *geo);
void get_normal(mesh *geo);
void get_texture(mesh *geo);
void get_face(mesh *geo);
int line_type(char *line);


/**
 * Creates a new mesh object and populates it with data from the provided
 * .OBJ file. 
 */
mesh *load_obj(char *filename)
{
  FILE *infile = fopen(filename, "r");
  char buffer[1024];
  mesh *geo;

  if(infile == NULL)
  {
    fprintf(stderr, "ERROR(load_obj): Unable to open file %s.\n", filename);
    return NULL;
  }

  if(!(geo = ready_mesh(infile)))
    return NULL;

  rewind(infile);

  while(fgets(buffer, 1023, infile))
    parse_line(buffer, geo);

  fclose(infile);

  return geo;
}


/**
 * Creates a new mesh struct and allocates the required amount of memory
 * correctly before attempting to populate it with data.
 */
mesh *ready_mesh(FILE *infile)
{
  mesh *geo = new_mesh();
  char buffer[1024];
  int type, i;

  if(geo == NULL) return NULL;

  /* Count the number of vertices, normals etc.  */
  while(fgets(buffer, 1023, infile))
  {
    type = line_type(buffer);

    if(type == VERTEX)
      geo->n_v++;
    else if(type == TEXTURE)
      geo->n_vt++;
    else if(type == FACE)
      geo->n_faces++;
  }
  geo->n_elements = 3 * geo->n_faces;

  /* Next allocate memory now that we know exactly how much we need.
   * If there are no elements of a particular type found then nothing is
   * allocated. Note that the number of vertices is used to allocate
   * normal and vf space. */
  if(geo->n_v)
  {
    geo->v = malloc(sizeof(float) * 3 * geo->n_v);
    if(geo->v == NULL) return NULL;
    geo->vf = malloc(sizeof(vf_head) * geo->n_v);
    if(geo->vf == NULL) return NULL;

    for(i = 0; i < geo->n_v; i++)
    {
      geo->vf[i].nodes = 0;
      geo->vf[i].head = NULL;
    }
  }
  if(geo->n_vt)
  {
    geo->vt = malloc(sizeof(float) * 3 * geo->n_vt);
    if(geo->vt == NULL) return NULL;
  }
  if(geo->n_faces)
  {
    geo->faces = malloc(sizeof(int) * 6 * geo->n_faces);
    if(geo->faces == NULL) return NULL;
  }

  return geo;
}


/**
 * Takes a single line of an *.OBJ file and parses it, putting the result
 * into the provided mesh object.
 */
void parse_line(char *line, mesh *geo)
{
  char line_copy[1024];
  int type;

  /* Since strtok cuts a character buffer up using \0 characters, we
   * make a cpoy of it here in case we need the original string later. */
  strcpy(line_copy, line);
  type = line_type(line_copy);

  switch(type)
  {
    case VERTEX:
      get_vertex(geo);
      break;
    case TEXTURE:
      get_texture(geo);
      break;
    case FACE:
      get_face(geo);
      break;
  }
}


/**
 * Extracts the float values of the current line and puts them into the
 * provided mesh object.
 */
void get_vertex(mesh *geo)
{
  char *value;
  int i;

  /* Get the vertex values from the current line. */
  for(i = 0; i < 3; i++)
  {
    value = strtok(NULL, WHITESPACE);

    if(value)
      geo->v[geo->c_v++] = atof(value);
    else
      geo->v[geo->c_v++] = 0.0;
  }
}


/**
 * Extracts the float values of the current line and puts them into the
 * provided mesh object.
 */
void get_texture(mesh *geo)
{
  char *value;
  int i;

  /* Get the tex coord values from the current line. */
  for(i = 0; i < 3; i++)
  {
    value = strtok(NULL, WHITESPACE);

    if(value)
      geo->vt[geo->c_vt++] = atof(value);
    else
      geo->vt[geo->c_vt++] = 0.0;
  }
}


/**
 * Adds a new association between a vertex and a face. Does this by adding
 * a new vertex face node to the head of the list for that vertex.
 */
void add_vf(mesh *geo, int v, int f)
{
  vf_node *new_vf;
  vf_head *list;

  if(geo == NULL || v < 0 || f < 0) return;

  new_vf = malloc(sizeof(vf_node));
  if(new_vf == NULL) return;

  list = geo->vf + v;

  /* Assign value and reassign references, placing the new node at the
   * head of the list. */
  new_vf->face = f;
  new_vf->next = list->head;
  list->head = new_vf;
  list->nodes++;
}


/**
 * Converts a face line into indexes that are usable by the mesh. Note that
 * this does not handle relative indexes, ie in the form -2/-1/-8.
 */
void get_face(mesh *geo)
{
  char *tmp;
  int vert_index;
  int i;

  while((tmp = strtok(NULL, WHITESPACE)) != NULL)
  {
    for(i = 0; i < 2; i++)
    {
      vert_index = atoi(tmp) - 1;
      if(i == 0) add_vf(geo, vert_index, geo->c_face_index);
      geo->faces[geo->c_face++] = vert_index;

      while(*tmp && (*tmp != '/')) tmp++;
      tmp++;
    }
  }

  geo->c_face_index++;
}


/**
 * Returns the type of a particular line. Usupported types return OTHER.
 */
int line_type(char *line)
{
  char *type = strtok(line, WHITESPACE);

  if(!type) return OTHER;       /* Check for a NULL value. */

  if(streq(type, "v"))
    return VERTEX;
  else if(streq(type, "vt"))
    return TEXTURE;
  else if(streq(type, "f"))
    return FACE;
  else
    return OTHER;
}
