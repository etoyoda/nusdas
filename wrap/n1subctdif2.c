#include <nusdas.h>

#undef nusdas_subc_tdif2_
	void
nusdas_subc_tdif2_(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	N_SI4 *diff_time,
	N_SI4 *total_sec,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_tdif2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		diff_time,
		total_sec,
		getput);
}
