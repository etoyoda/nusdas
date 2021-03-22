#include <libsrf.h>
#ifndef N_SI4
# define N_SI4	int
#endif

#undef srf_rd_rdic__
	void
srf_rd_rdic__(N_SI4 *stnum,
	N_SI4 *iseq,
	float *lat,
	float *lon,
	float *hh,
	N_SI4 *offx,
	N_SI4 *offy,
	N_SI4 *type1,
	N_SI4 *type2,
	N_SI4 *result)
{
	*result = srf_rd_rdic(*stnum,
		*iseq,
		lat,
		lon,
		hh,
		offx,
		offy,
		type1,
		type2);
}
