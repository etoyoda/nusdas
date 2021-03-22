#include <nusdas.h>

#undef ENDIAN_SWAB2
	void
ENDIAN_SWAB2(void *ary,
	const N_UI4 *count)
{
	NuSDaS_swab2(ary,
		*count);
}
