#ifndef _UTIL_H_
#define _UTIL_H_

#define MY_PI 3.1415926535897932385E0
#define RAD(x) ((x) * MY_PI / 180)

void v_norm(float v[3]);
float v_dot(float v0[3], float v1[3]);
void v_scale(float v[3], float factor);
void v_add(float r[3], float v[3]);
void v_sub(float r[3], float v0[3], float v1[3]);
void v_cross(float r[3], float v0[3], float v1[3]);
void v_copy(float dest[3], float v[3]);
void v_print(float v[3]);
void v_clear(float v[3]);
float mod(float value, int mod);
float clamp(float value, float min, float max);

#endif
