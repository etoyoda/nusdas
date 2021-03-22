#include <nusdas.h>

#undef nusdas_subc_rgau2__
	void
nusdas_subc_rgau2__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
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
	*result = NuSDaS_subc_rgau2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		j,
		j_start,
		j_n,
		i,
		i_start,
		i_n,
		lat,
		getput);
}
