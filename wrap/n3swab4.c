#include <nusdas.h>

#undef ENDIAN_SWAB4
	void
ENDIAN_SWAB4(void *ary,
	const N_UI4 *count)
{
	NuSDaS_swab4(ary,
		*count);
}
