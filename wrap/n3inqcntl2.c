#include <nusdas.h>

#undef NUSDAS_INQ_CNTL2
	void
NUSDAS_INQ_CNTL2(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	const N_SI4 *param,
	void *data,
	const N_SI4 *datasize,
	N_SI4 *result)
{
	*result = NuSDaS_inq_cntl2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		*param,
		data,
		datasize);
}
