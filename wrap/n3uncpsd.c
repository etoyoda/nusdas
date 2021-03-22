#include <nusdas.h>

#undef NUSDAS_UNCPSD
	void
NUSDAS_UNCPSD(const void *pdata,
	void *cdata,
	const N_SI4 *csize,
	N_SI4 *result)
{
	*result = NuSDaS_uncpsd(pdata,
		cdata,
		*csize);
}
