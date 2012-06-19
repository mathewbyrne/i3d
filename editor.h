/**
 * editor.h
 */

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "3d.h"


/* Interface. */
extern void edit_init();
extern void edit_update(int now);
extern void edit_cleanup();
char *edit_get_string();
extern void edit_keyboard(unsigned char key, int mx, int my);
extern void edit_save_anim(const char *filename);


#endif

