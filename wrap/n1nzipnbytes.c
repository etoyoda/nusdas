#include <nusdas.h>

#undef nusdas_gunzip_nbytes_
	void
nusdas_gunzip_nbytes_(const void *in_data,
	const N_UI4 *in_nbytes,
	N_SI4 *result)
{
	*result = NuSDaS_gunzip_nbytes(in_data,
		*in_nbytes);
}
