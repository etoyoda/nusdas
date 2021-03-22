#include <libsrf.h>
#ifndef N_SI4
# define N_SI4	int
#endif

#undef srf_lv_trans__
	void
srf_lv_trans__(const N_SI4 *idat,
	float *fdat,
	const N_SI4 *dnum,
	const N_SI4 *ispec,
	N_SI4 *result)
{
	*result = srf_lv_trans(idat,
		fdat,
		*dnum,
		ispec);
}
