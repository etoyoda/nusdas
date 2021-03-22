#include <nusdas.h>

#undef NUSDAS_SUBC_DELT_PRESET1
	void
NUSDAS_SUBC_DELT_PRESET1(const char *type1,
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
