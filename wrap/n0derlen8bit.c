#include <nusdas.h>

#undef n_encode_rlen_8bit__
	void
n_encode_rlen_8bit__(const N_SI4 *udata,
	unsigned char *compressed_data,
	const N_SI4 *udata_nelems,
	const N_SI4 *max_compress_nbytes,
	N_SI4 *maxvalue,
	N_SI4 *result)
{
	*result = NuSDaS_encode_rlen_8bit(udata,
		compressed_data,
		*udata_nelems,
		*max_compress_nbytes,
		maxvalue);
}
