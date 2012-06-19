/**
 * load_mdl.h
 *
 */
#ifndef _LOADER_H_
#define _LOADER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "3d.h"
#include "load_obj.h"
#include "texture.h"

#define BUFF_LEN 1024
#define MAX_ARGS 16
#define SEPERATOR ' '

model *load_model(const char *file_name);

#endif
