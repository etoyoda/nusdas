#include <nusdas.h>

#undef NUSDAS_SUBC_SIGM2
	void
NUSDAS_SUBC_SIGM2(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	N_SI4 *n_levels,
	float *a,
	float *b,
	float *c,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_sigm2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		n_levels,
		a,
		b,
		c,
		getput);
}
