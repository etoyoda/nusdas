#include <nusdas.h>

#undef NUSDAS_ONEFILE_CLOSE
	void
NUSDAS_ONEFILE_CLOSE(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	N_SI4 *result)
{
	*result = NuSDaS_onefile_close(type1,
		type2,
		type3,
		basetime,
		member,
		validtime);
}
