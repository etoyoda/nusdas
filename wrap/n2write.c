#include <nusdas.h>

#undef nusdas_write
	void
nusdas_write(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const char *plane,
	const char *element,
	const void *udata,
	const char *utype,
	const N_SI4 *usize,
	N_SI4 *result)
{
	*result = NuSDaS_write(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
		plane,
		element,
		udata,
		utype,
		usize);
}
