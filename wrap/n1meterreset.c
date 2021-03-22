#include <nusdas.h>

#undef nusdas_parameter_reset_
	void
nusdas_parameter_reset_(const N_SI4 *param,
	N_SI4 *result)
{
	*result = NuSDaS_parameter_reset(*param);
}
