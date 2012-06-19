/**
 * load_obj.h
 *
 * Header file for the basic code library for loading obj files. This is a
 * VERY simple implementation of obj loading and loads minimal features,
 * only those required for this assignment.
 * Generally the following is supported:
 *
 * - Loading of vertex info and texture coordinates from an obj file.
 * - Loading of TRIANGLE only faces from an obj file.
 *
 * Any other operations are generally not supported although may work. Normals
 * are NOT supported. Instead the mesh struct stores the vertexes in such
 * a way that they can be calculated later.
 */

#ifndef _LOAD_OBJ_H
#define _LOAD_OBJ_H

#include "3d.h"
#include "global.h"

#include <stdio.h>
#include <string.h>

enum {
  VERTEX,
  TEXTURE,
  FACE,
  OTHER
};

mesh *load_obj(char *filename);

#endif

