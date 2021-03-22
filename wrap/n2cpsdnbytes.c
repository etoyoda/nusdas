#include <nusdas.h>

#undef nusdas_uncpsd_nbytes
	void
nusdas_uncpsd_nbytes(const void *pdata,
	N_SI4 *result)
{
	*result = NuSDaS_uncpsd_nbytes(pdata);
}
