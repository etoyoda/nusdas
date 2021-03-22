#include <nusdas.h>

#undef n_encode_rlen_8bit_i1_
	void
n_encode_rlen_8bit_i1_(const unsigned char *udata,
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
