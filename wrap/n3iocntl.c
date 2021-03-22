#include <nusdas.h>

#undef NUSDAS_IOCNTL
	void
NUSDAS_IOCNTL(const N_SI4 *param,
	const N_SI4 *value,
	N_SI4 *result)
{
	*result = NuSDaS_iocntl(*param,
		*value);
}
