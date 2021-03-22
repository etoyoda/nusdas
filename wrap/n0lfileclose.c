#include <nusdas.h>

#undef nusdas_allfile_close__
	void
nusdas_allfile_close__(const N_SI4 *param,
	N_SI4 *result)
{
	*result = NuSDaS_allfile_close(*param);
}
