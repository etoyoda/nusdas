#include <nusdas.h>

#undef nusdas_uncpsd
	void
nusdas_uncpsd(const void *pdata,
	void *cdata,
	const N_SI4 *csize,
	N_SI4 *result)
{
	*result = NuSDaS_uncpsd(pdata,
		cdata,
		*csize);
}
