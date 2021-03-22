#include <nusdas.h>

#undef nusdas_inq_data2__
	void
nusdas_inq_data2__(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	const char *plane1,
	const char *plane2,
	const char *element,
	const N_SI4 *param,
	void *data,
	const N_SI4 *datasize,
	N_SI4 *result)
{
	*result = NuSDaS_inq_data2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		plane1,
		plane2,
		element,
		*param,
		data,
		datasize);
}
