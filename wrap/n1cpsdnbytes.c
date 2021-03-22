#include <nusdas.h>

#undef nusdas_uncpsd_nbytes_
	void
nusdas_uncpsd_nbytes_(const void *pdata,
	N_SI4 *result)
{
	*result = NuSDaS_uncpsd_nbytes(pdata);
}
