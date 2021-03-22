#include <nusdas.h>

#undef nusdas_subc_rgau_preset1_
	void
nusdas_subc_rgau_preset1_(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *j,
	const N_SI4 *j_start,
	const N_SI4 *j_n,
	const N_SI4 *i,
	const N_SI4 *i_start,
	const N_SI4 *i_n,
	const float *lat,
	N_SI4 *result)
{
	*result = NuSDaS_subc_rgau_preset1(type1,
		type2,
		type3,
		j,
		j_start,
		j_n,
		i,
		i_start,
		i_n,
		lat);
}
