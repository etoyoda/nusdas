#include <nusdas.h>

#undef nusdas_subc_delt_preset1__
	void
nusdas_subc_delt_preset1__(const char *type1,
	const char *type2,
	const char *type3,
	const float *delt,
	N_SI4 *result)
{
	*result = NuSDaS_subc_delt_preset1(type1,
		type2,
		type3,
		delt);
}
