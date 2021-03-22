#include <nusdas.h>

#undef NUSDAS_PARAMETER_CHANGE
	void
NUSDAS_PARAMETER_CHANGE(const N_SI4 *param,
	const void *value,
	N_SI4 *result)
{
	*result = NuSDaS_parameter_change(*param,
		value);
}
