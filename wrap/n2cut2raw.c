#include <nusdas.h>

#undef nusdas_cut2_raw
	void
nusdas_cut2_raw(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	const char *plane1,
	const char *plane2,
	const char *element,
	void *udata,
	const N_SI4 *usize,
	const N_SI4 *ixstart,
	const N_SI4 *ixfinal,
	const N_SI4 *iystart,
	const N_SI4 *iyfinal,
	N_SI4 *result)
{
	*result = NuSDaS_cut2_raw(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		plane1,
		plane2,
		element,
		udata,
		usize,
		ixstart,
		ixfinal,
		iystart,
		iyfinal);
}
