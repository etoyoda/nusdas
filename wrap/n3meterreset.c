#include <nusdas.h>

#undef NUSDAS_PARAMETER_RESET
	void
NUSDAS_PARAMETER_RESET(const N_SI4 *param,
	N_SI4 *result)
{
	*result = NuSDaS_parameter_reset(*param);
}
