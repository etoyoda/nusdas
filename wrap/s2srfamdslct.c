#include <libsrf.h>
#ifndef N_SI4
# define N_SI4	int
#endif

#undef SRF_AMD_SLCT
	void
SRF_AMD_SLCT(const N_SI4 *st_r,
	N_SI4 *n_st_r,
	void *d_r,
	const char *t_r,
	N_SI4 *st_n,
	N_SI4 *n_st_n,
	void *d_n,
	const char *t_n,
	N_SI4 *sort_f,
	N_SI4 *result)
{
	*result = srf_amd_slct(st_r,
		*n_st_r,
		d_r,
		t_r,
		st_n,
		*n_st_n,
		d_n,
		t_n,
		*sort_f);
}
