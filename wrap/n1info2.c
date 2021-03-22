#include <nusdas.h>

#undef nusdas_info2_
	void
nusdas_info2_(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	const char *group,
	char *info,
	const N_SI4 *bytesize,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_info2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		group,
		info,
		bytesize,
		getput);
}
