#include <nusdas.h>

#undef nusdas_uncpsd_nbytes__
	void
nusdas_uncpsd_nbytes__(const void *pdata,
	N_SI4 *result)
{
	*result = NuSDaS_uncpsd_nbytes(pdata);
}
