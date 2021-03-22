#include <nusdas.h>

#undef endian_swab8_
	void
endian_swab8_(void *ary,
	const N_UI4 *count)
{
	NuSDaS_swab8(ary,
		*count);
}
