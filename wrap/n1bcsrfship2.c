#include <nusdas.h>

#undef nusdas_subc_srf_ship2_
	void
nusdas_subc_srf_ship2_(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	N_SI4 *lat,
	N_SI4 *lon,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_srf_ship2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		lat,
		lon,
		getput);
}
