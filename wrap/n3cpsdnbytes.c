#include <nusdas.h>

#undef NUSDAS_UNCPSD_NBYTES
	void
NUSDAS_UNCPSD_NBYTES(const void *pdata,
	N_SI4 *result)
{
	*result = NuSDaS_uncpsd_nbytes(pdata);
}
