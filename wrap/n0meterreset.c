#include <nusdas.h>

#undef nusdas_parameter_reset__
	void
nusdas_parameter_reset__(const N_SI4 *param,
	N_SI4 *result)
{
	*result = NuSDaS_parameter_reset(*param);
}
