#include <nusdas.h>

#undef NUSDAS_INQ_CNTL
	void
NUSDAS_INQ_CNTL(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const N_SI4 *param,
	void *data,
	const N_SI4 *datasize,
	N_SI4 *result)
{
	*result = NuSDaS_inq_cntl(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		*param,
		data,
		datasize);
}
