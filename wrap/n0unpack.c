#include <nusdas.h>

#undef nusdas_unpack__
	void
nusdas_unpack__(const void *pdata,
	void *udata,
	const char *utype,
	const N_SI4 *usize,
	N_SI4 *result)
{
	*result = NuSDaS_unpack(pdata,
		udata,
		utype,
		*usize);
}
