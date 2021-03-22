#include <nusdas.h>

#undef NUSDAS_SUBC_DELT2
	void
NUSDAS_SUBC_DELT2(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	float *delt,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_delt2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		delt,
		getput);
}
