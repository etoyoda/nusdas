#include <nusdas.h>

#undef endian_swab8__
	void
endian_swab8__(void *ary,
	const N_UI4 *count)
{
	NuSDaS_swab8(ary,
		*count);
}
