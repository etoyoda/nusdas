#include <nusdas.h>

#undef nusdas_make_mask_
	void
nusdas_make_mask_(const void *udata,
	const char *utype,
	const N_SI4 *usize,
	void *maskbits,
	const N_SI4 *mb_bytes,
	N_SI4 *result)
{
	*result = NuSDaS_make_mask(udata,
		utype,
		usize,
		maskbits,
		mb_bytes);
}
