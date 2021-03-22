#include <nusdas.h>

#undef endian_swab4_
	void
endian_swab4_(void *ary,
	const N_UI4 *count)
{
	NuSDaS_swab4(ary,
		*count);
}
