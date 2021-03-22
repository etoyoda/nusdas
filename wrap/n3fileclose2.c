#include <nusdas.h>

#undef NUSDAS_ONEFILE_CLOSE2
	void
NUSDAS_ONEFILE_CLOSE2(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	N_SI4 *result)
{
	*result = NuSDaS_onefile_close2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2);
}
