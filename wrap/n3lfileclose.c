#include <nusdas.h>

#undef NUSDAS_ALLFILE_CLOSE
	void
NUSDAS_ALLFILE_CLOSE(const N_SI4 *param,
	N_SI4 *result)
{
	*result = NuSDaS_allfile_close(*param);
}
