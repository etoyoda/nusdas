#include <nusdas.h>

#undef nusdas_gunzip
	void
nusdas_gunzip(const void *in_data,
	const N_UI4 *in_nbytes,
	void *out_buf,
	const N_UI4 *out_nbytes,
	N_SI4 *result)
{
	*result = NuSDaS_gunzip(in_data,
		*in_nbytes,
		out_buf,
		*out_nbytes);
}
