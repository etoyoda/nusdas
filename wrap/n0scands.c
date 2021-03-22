#include <nusdas.h>

#undef nusdas_scan_ds__
	void
nusdas_scan_ds__(char *type1,
	char *type2,
	char *type3,
	N_SI4 *nrd,
	N_SI4 *result)
{
	*result = NuSDaS_scan_ds(type1,
		type2,
		type3,
		nrd);
}
