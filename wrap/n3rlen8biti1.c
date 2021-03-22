#include <nusdas.h>

#undef N_ENCODE_RLEN_8BIT_I1
	void
N_ENCODE_RLEN_8BIT_I1(const unsigned char *udata,
	unsigned char *compressed_data,
	const N_SI4 *udata_nelems,
	const N_SI4 *max_compress_nbytes,
	N_SI4 *maxvalue,
	N_SI4 *result)
{
	*result = NuSDaS_encode_rlen_8bit_I1(udata,
		compressed_data,
		*udata_nelems,
		*max_compress_nbytes,
		maxvalue);
}
