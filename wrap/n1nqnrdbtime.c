#include <nusdas.h>

#undef nusdas_inq_nrdbtime_
	void
nusdas_inq_nrdbtime_(const char *type1,
	const char *type2,
	const char *type3,
	N_SI4 *btlist,
	const N_SI4 *btmax,
	const N_SI4 *pflag,
	N_SI4 *result)
{
	*result = NuSDaS_inq_nrdbtime(type1,
		type2,
		type3,
		btlist,
		btmax,
		*pflag);
}
