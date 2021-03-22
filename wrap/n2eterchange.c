#include <nusdas.h>

#undef nusdas_parameter_change
	void
nusdas_parameter_change(const N_SI4 *param,
	const void *value,
	N_SI4 *result)
{
	*result = NuSDaS_parameter_change(*param,
		value);
}
