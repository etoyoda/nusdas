#include <libsrf.h>
#ifndef N_SI4
# define N_SI4	int
#endif

#undef srf_amd_rdic__
	void
srf_amd_rdic__(SRF_AMD_SINFO *amd,
	N_SI4 *amdnum,
	N_SI4 *btime,
	N_SI4 *amd_type,
	N_SI4 *result)
{
	*result = srf_amd_rdic(amd,
		*amdnum,
		*btime,
		*amd_type);
}
