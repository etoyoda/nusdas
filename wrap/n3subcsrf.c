#include <nusdas.h>

#undef NUSDAS_SUBC_SRF
	void
NUSDAS_SUBC_SRF(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const char *plane,
	const char *element,
	const char *group,
	N_SI4 *data,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_srf(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		plane,
		element,
		group,
		data,
		getput);
}