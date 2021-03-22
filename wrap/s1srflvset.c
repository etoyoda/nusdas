#include <libsrf.h>
#include <string.h>
#ifndef N_SI4
# define N_SI4	int
#endif

#undef srf_lv_set_
	void
srf_lv_set_(N_SI4 *idat,
	const float *fdat,
	N_SI4 *dnum,
	N_SI4 *ispec,
	const char *param,
	N_SI4 *result,
	unsigned L_param)
{
	char param0[1024];
	memcpy(param0, param, (L_param >= 1024) ? 1023 : L_param);
	param0[(L_param >= 1024) ? 1023 : L_param] = '\0';
	*result = srf_lv_set(idat,
		fdat,
		*dnum,
		ispec,
		param0);
}
