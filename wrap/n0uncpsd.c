#include <nusdas.h>

#undef nusdas_uncpsd__
	void
nusdas_uncpsd__(const void *pdata,
	void *cdata,
	const N_SI4 *csize,
	N_SI4 *result)
{
	*result = NuSDaS_uncpsd(pdata,
		cdata,
		*csize);
}
