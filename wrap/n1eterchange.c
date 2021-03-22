#include <nusdas.h>

#undef nusdas_parameter_change_
	void
nusdas_parameter_change_(const N_SI4 *param,
	const void *value,
	N_SI4 *result)
{
	*result = NuSDaS_parameter_change(*param,
		value);
}
