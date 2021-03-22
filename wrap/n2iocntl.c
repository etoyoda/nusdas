#include <nusdas.h>

#undef nusdas_iocntl
	void
nusdas_iocntl(const N_SI4 *param,
	const N_SI4 *value,
	N_SI4 *result)
{
	*result = NuSDaS_iocntl(*param,
		*value);
}
