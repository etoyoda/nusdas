#include <nusdas.h>

#undef NUSDAS_GZIP
	void
NUSDAS_GZIP(const void *in_data,
	const N_UI4 *in_nbytes,
	void *out_buf,
	const N_UI4 *out_nbytes,
	N_SI4 *result)
{
	*result = NuSDaS_gzip(in_data,
		*in_nbytes,
		out_buf,
		*out_nbytes);
}
