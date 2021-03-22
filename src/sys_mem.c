/** @file
 * @brief メモリ管理
 */

#include "config.h"
#include "nusdas.h"
#include <stdio.h>
#include <stdlib.h>
#include "internal_types.h"
#include "glb.h"

#undef nus_malloc
#undef nus_realloc
#undef nus_free

void *
nus_malloc(size_t nbytes)
{
	void *r;
	static unsigned long i = 0;
	r = malloc(nbytes);
	i++;
#if 0
	/* メモリが足りている状況で gc を模倣する */
	if (i == 1000) {
		nusglb_garbage_collect();
		i = 0;
	}
#endif
	if (r == NULL) {
		nusglb_garbage_collect();
		r = malloc(nbytes);
	}
	return r;
}

void *
nus_realloc(void *ptr, size_t nbytes)
{
	void *r;
	r = realloc(ptr, nbytes);
	if (r == NULL) {
		nusglb_garbage_collect();
		r = realloc(ptr, nbytes);
	}
	return r;
}

void
nus_free(void *ptr)
{
	free(ptr);
}
