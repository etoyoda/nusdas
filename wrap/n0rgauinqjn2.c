#include <nusdas.h>

#undef nusdas_subc_rgau_inq_jn2__
	void
nusdas_subc_rgau_inq_jn2__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	N_SI4 *j_n,
	N_SI4 *result)
{
	*result = NuSDaS_subc_rgau_inq_jn2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		j_n);
}
