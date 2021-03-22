#include <libsrf.h>
#ifndef N_SI4
# define N_SI4	int
#endif

#undef srf_search_amdstn_
	void
srf_search_amdstn_(const SRF_AMD_SINFO *amd,
	N_SI4 *ac,
	N_SI4 *stn,
	N_SI4 *amd_type,
	N_SI4 *result)
{
	*result = srf_search_amdstn(amd,
		*ac,
		*stn,
		*amd_type);
}
