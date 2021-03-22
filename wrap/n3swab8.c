#include <nusdas.h>

#undef ENDIAN_SWAB8
	void
ENDIAN_SWAB8(void *ary,
	const N_UI4 *count)
{
	NuSDaS_swab8(ary,
		*count);
}
