#include <nusdas.h>

#undef nusdas_set_mask
	void
nusdas_set_mask(const char *type1,
	const char *type2,
	const char *type3,
	const void *udata,
	const char *utype,
	const N_SI4 *usize,
	N_SI4 *result)
{
	*result = NuSDaS_set_mask(type1,
		type2,
		type3,
		udata,
		utype,
		*usize);
}
