#include <nusdas.h>

#undef nusdas_subc_rgau_inq_jn__
	void
nusdas_subc_rgau_inq_jn__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	N_SI4 *j_n,
	N_SI4 *result)
{
	*result = NuSDaS_subc_rgau_inq_jn(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		j_n);
}
