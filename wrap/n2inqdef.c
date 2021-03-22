#include <nusdas.h>

#undef nusdas_inq_def
	void
nusdas_inq_def(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *param,
	void *data,
	const N_SI4 *datasize,
	N_SI4 *result)
{
	*result = NuSDaS_inq_def(type1,
		type2,
		type3,
		*param,
		data,
		datasize);
}
