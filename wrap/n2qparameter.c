#include <nusdas.h>

#undef nusdas_inq_parameter
	void
nusdas_inq_parameter(const N_SI4 *item,
	void *data,
	N_SI4 *result)
{
	*result = NuSDaS_inq_parameter(*item,
		data);
}
