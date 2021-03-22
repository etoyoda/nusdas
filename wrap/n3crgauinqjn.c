#include <nusdas.h>

#undef NUSDAS_SUBC_RGAU_INQ_JN
	void
NUSDAS_SUBC_RGAU_INQ_JN(const char *type1,
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
