#include <nusdas.h>

#undef nusdas_info__
	void
nusdas_info__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const char *group,
	char *info,
	const N_SI4 *bytesize,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_info(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		group,
		info,
		bytesize,
		getput);
}
