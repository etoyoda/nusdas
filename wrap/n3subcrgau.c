#include <nusdas.h>

#undef NUSDAS_SUBC_RGAU
	void
NUSDAS_SUBC_RGAU(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	N_SI4 *j,
	N_SI4 *j_start,
	N_SI4 *j_n,
	N_SI4 *i,
	N_SI4 *i_start,
	N_SI4 *i_n,
	float *lat,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_rgau(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		j,
		j_start,
		j_n,
		i,
		i_start,
		i_n,
		lat,
		getput);
}
