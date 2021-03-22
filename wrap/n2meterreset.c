#include <nusdas.h>

#undef nusdas_parameter_reset
	void
nusdas_parameter_reset(const N_SI4 *param,
	N_SI4 *result)
{
	*result = NuSDaS_parameter_reset(*param);
}
