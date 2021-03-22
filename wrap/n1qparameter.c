#include <nusdas.h>

#undef nusdas_inq_parameter_
	void
nusdas_inq_parameter_(const N_SI4 *item,
	void *data,
	N_SI4 *result)
{
	*result = NuSDaS_inq_parameter(*item,
		data);
}
