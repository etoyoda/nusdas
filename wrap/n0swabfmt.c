#include <nusdas.h>
#include <string.h>

#undef endian_swab_fmt__
	void
endian_swab_fmt__(void *ary,
	const char *fmt,
	unsigned L_fmt)
{
	char fmt0[1024];
	memcpy(fmt0, fmt, (L_fmt >= 1024) ? 1023 : L_fmt);
	fmt0[(L_fmt >= 1024) ? 1023 : L_fmt] = '\0';
	NuSDaS_swab_fmt(ary,
		fmt0);
}
