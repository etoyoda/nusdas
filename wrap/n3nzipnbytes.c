#include <nusdas.h>

#undef NUSDAS_GUNZIP_NBYTES
	void
NUSDAS_GUNZIP_NBYTES(const void *in_data,
	const N_UI4 *in_nbytes,
	N_SI4 *result)
{
	*result = NuSDaS_gunzip_nbytes(in_data,
		*in_nbytes);
}
