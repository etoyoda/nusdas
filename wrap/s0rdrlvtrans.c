#include <libsrf.h>
#include <string.h>
#ifndef N_SI4
# define N_SI4	int
#endif

#undef rdr_lv_trans__
	void
rdr_lv_trans__(N_SI4 *idat,
	float *fdat,
	N_SI4 *dnum,
	const char *param,
	N_SI4 *result,
	unsigned L_param)
{
	char param0[1024];
	memcpy(param0, param, (L_param >= 1024) ? 1023 : L_param);
	param0[(L_param >= 1024) ? 1023 : L_param] = '\0';
	*result = rdr_lv_trans(idat,
		fdat,
		*dnum,
		param0);
}
