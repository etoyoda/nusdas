#include <nusdas.h>

#undef NUSDAS_ESF_FLUSH
	void
NUSDAS_ESF_FLUSH(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	N_SI4 *result)
{
	*result = NuSDaS_esf_flush(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1);
}
