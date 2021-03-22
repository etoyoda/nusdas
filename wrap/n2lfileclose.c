#include <nusdas.h>

#undef nusdas_allfile_close
	void
nusdas_allfile_close(const N_SI4 *param,
	N_SI4 *result)
{
	*result = NuSDaS_allfile_close(*param);
}
