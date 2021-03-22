#include <nusdas.h>

#undef nusdas_grid_
	void
nusdas_grid_(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	char *proj,
	N_SI4 *gridsize,
	float *gridinfo,
	char *value,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_grid(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		proj,
		gridsize,
		gridinfo,
		value,
		getput);
}
