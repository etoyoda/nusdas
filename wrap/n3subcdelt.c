#include <nusdas.h>

#undef NUSDAS_SUBC_DELT
	void
NUSDAS_SUBC_DELT(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	float *delt,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_delt(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		delt,
		getput);
}
