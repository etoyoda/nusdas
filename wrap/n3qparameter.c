#include <nusdas.h>

#undef NUSDAS_INQ_PARAMETER
	void
NUSDAS_INQ_PARAMETER(const N_SI4 *item,
	void *data,
	N_SI4 *result)
{
	*result = NuSDaS_inq_parameter(*item,
		data);
}
