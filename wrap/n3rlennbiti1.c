#include <nusdas.h>

#undef N_DECODE_RLEN_NBIT_I1
	void
N_DECODE_RLEN_NBIT_I1(unsigned char *udata,
	const unsigned char *compressed_data,
	const N_SI4 *compressed_nbytes,
	const N_SI4 *udata_nelems,
	const N_SI4 *maxvalue,
	const N_SI4 *nbit,
	N_SI4 *result)
{
	*result = NuSDaS_decode_rlen_nbit_I1(udata,
		compressed_data,
		*compressed_nbytes,
		*udata_nelems,
		*maxvalue,
		*nbit);
}
