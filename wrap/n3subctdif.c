#include <nusdas.h>

#undef NUSDAS_SUBC_TDIF
	void
NUSDAS_SUBC_TDIF(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	N_SI4 *diff_time,
	N_SI4 *total_sec,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_tdif(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		diff_time,
		total_sec,
		getput);
}
