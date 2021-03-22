#include <nusdas.h>

#undef NUSDAS_SUBC_RGAU_INQ_JN2
	void
NUSDAS_SUBC_RGAU_INQ_JN2(const char *type1,
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
