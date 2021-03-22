#include <nusdas.h>

#undef nusdas_inq_nrdvtime
	void
nusdas_inq_nrdvtime(const char *type1,
	const char *type2,
	const char *type3,
	N_SI4 *vtlist,
	const N_SI4 *vtlistsize,
	const N_SI4 *basetime,
	const N_SI4 *pflag,
	N_SI4 *result)
{
	*result = NuSDaS_inq_nrdvtime(type1,
		type2,
		type3,
		vtlist,
		vtlistsize,
		basetime,
		*pflag);
}
