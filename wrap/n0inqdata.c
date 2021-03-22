#include <nusdas.h>

#undef nusdas_inq_data__
	void
nusdas_inq_data__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const char *plane,
	const char *element,
	const N_SI4 *param,
	void *data,
	const N_SI4 *datasize,
	N_SI4 *result)
{
	*result = NuSDaS_inq_data(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		plane,
		element,
		*param,
		data,
		datasize);
}
