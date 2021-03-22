#include <nusdas.h>

#undef NUSDAS_UNPACK
	void
NUSDAS_UNPACK(const void *pdata,
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
