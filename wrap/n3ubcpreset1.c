#include <nusdas.h>

#undef NUSDAS_SUBC_PRESET1
	void
NUSDAS_SUBC_PRESET1(const char *type1,
	const char *type2,
	const char *type3,
	const char *group,
	const N_SI4 *n_levels,
	float *a,
	float *b,
	float *c,
	N_SI4 *result)
{
	*result = NuSDaS_subc_preset1(type1,
		type2,
		type3,
		group,
		n_levels,
		a,
		b,
		c);
}
