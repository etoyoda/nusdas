#include <nusdas.h>

#undef NUSDAS_WRITE2
	void
NUSDAS_WRITE2(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	const char *plane1,
	const char *plane2,
	const char *element,
	const void *udata,
	const char *utype,
	const N_SI4 *usize,
	N_SI4 *result)
{
	*result = NuSDaS_write2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		plane1,
		plane2,
		element,
		udata,
		utype,
		usize);
}
