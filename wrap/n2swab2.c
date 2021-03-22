#include <nusdas.h>

#undef endian_swab2
	void
endian_swab2(void *ary,
	const N_UI4 *count)
{
	NuSDaS_swab2(ary,
		*count);
}
