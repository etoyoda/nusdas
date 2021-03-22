#include <nusdas.h>

#undef NUSDAS_INQ_SUBCINFO
	void
NUSDAS_INQ_SUBCINFO(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const N_SI4 *query,
	const char *group,
	void *buf,
	const N_SI4 *bufnelems,
	N_SI4 *result)
{
	*result = NuSDaS_inq_subcinfo(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		*query,
		group,
		buf,
		*bufnelems);
}
