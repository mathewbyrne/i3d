/**
 * flight.c
 *
 * Creates some simple code to create multiple instances of birds and
 * animate them.
 */

#ifndef _FLIGHT_H_
#define _FLIGHT_H_


extern void flight_init();
extern void flight_cleanup();
extern void flight_add_bird(int now);
extern void flight_update(int now);


#endif

