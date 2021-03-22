#include <nusdas.h>

#undef nusdas_subc_sigm__
	void
nusdas_subc_sigm__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	N_SI4 *n_levels,
	float *a,
	float *b,
	float *c,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_sigm(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		n_levels,
		a,
		b,
		c,
		getput);
}
