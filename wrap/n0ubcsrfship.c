#include <nusdas.h>

#undef nusdas_subc_srf_ship__
	void
nusdas_subc_srf_ship__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	N_SI4 *lat,
	N_SI4 *lon,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_srf_ship(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		lat,
		lon,
		getput);
}
