#include <nusdas.h>

#undef endian_swab4
	void
endian_swab4(void *ary,
	const N_UI4 *count)
{
	NuSDaS_swab4(ary,
		*count);
}
