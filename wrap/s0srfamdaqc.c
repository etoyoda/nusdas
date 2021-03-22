#include <libsrf.h>
#include <string.h>
#ifndef N_SI4
# define N_SI4	int
#endif

#undef srf_amd_aqc__
	void
srf_amd_aqc__(const N_UI2 *aqc_in,
	N_SI4 *num,
	N_SI2 *aqc_out,
	const char *param,
	unsigned L_param)
{
	char param0[1024];
	memcpy(param0, param, (L_param >= 1024) ? 1023 : L_param);
	param0[(L_param >= 1024) ? 1023 : L_param] = '\0';
	srf_amd_aqc(aqc_in,
		*num,
		aqc_out,
		param0);
}
