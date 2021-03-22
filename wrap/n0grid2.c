#include <nusdas.h>

#undef nusdas_grid2__
	void
nusdas_grid2__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	char *proj,
	N_SI4 *gridsize,
	float *gridinfo,
	char *value,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_grid2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		proj,
		gridsize,
		gridinfo,
		value,
		getput);
}
