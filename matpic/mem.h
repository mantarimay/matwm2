#ifndef __MEM_H__
#define __MEM_H__

/************************
 * generic memory stuff *
 ************************/

#define BLOCK 2048  /* amount of memory to allocate in one go */
#define ARG_MAX 256 /* maximum number of arguments */

typedef struct {
	void *data;
	int size, count, space;
} arr_t;

extern void arr_new(arr_t *a, int size);
extern void arr_add(arr_t *a, void *data);
extern void arr_free(arr_t *a);

typedef struct {
	char *data;
	int len, res;
} string_t;

#include "host.h" /* strlen() */
#define vstr_add(s, str) (vstr_addl(s, str, strlen(str)))

void vstr_new(string_t *s);
void vstr_addl(string_t *s, char *str, int len);
void vstr_free(string_t *s);

#endif /* __MEM_H__ */

