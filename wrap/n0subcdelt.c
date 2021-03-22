#include <nusdas.h>

#undef nusdas_subc_delt__
	void
nusdas_subc_delt__(const char *type1,
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
