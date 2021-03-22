#include <nusdas.h>

#undef NUSDAS_CUT_RAW
	void
NUSDAS_CUT_RAW(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const char *plane,
	const char *element,
	void *udata,
	const N_SI4 *usize,
	const N_SI4 *ixstart,
	const N_SI4 *ixfinal,
	const N_SI4 *iystart,
	const N_SI4 *iyfinal,
	N_SI4 *result)
{
	*result = NuSDaS_cut_raw(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		plane,
		element,
		udata,
		usize,
		ixstart,
		ixfinal,
		iystart,
		iyfinal);
}
