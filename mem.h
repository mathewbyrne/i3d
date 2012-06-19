/**
 * mem.h
 *
 * A set of macros which make the process of allocating, checking and freeing
 * memory a lot simpler.
 */

#ifndef _MEM_H_
#define _MEM_H_

#define NEW(p) (p) = malloc(sizeof(*(p)))
#define MEW(p, count) (p) = malloc(sizeof((*(p)) * (count)))
#define CHECK(p) \
  if(!p) { \
    fprintf(stderr, "Memory Allocation Failed at %s, line %d\n", __FILE__, \
        __LINE__); \
    return NULL; \
  }
#define CHECK_NR(p) \
  if(!p) { \
    fprintf(stderr, "Memory Allocation Failed at %s, line %d\n", __FILE__, \
        __LINE__); \
    return; \
  }
#define FREE(p) if(p) free(p)

#endif

