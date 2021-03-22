#include <nusdas.h>

#undef nusdas_subc_eta_inq_nz__
	void
nusdas_subc_eta_inq_nz__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const char *group,
	N_SI4 *n_levels,
	N_SI4 *result)
{
	*result = NuSDaS_subc_eta_inq_nz(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		group,
		n_levels);
}
