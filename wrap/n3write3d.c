#include <nusdas.h>

#undef NUSDAS_WRITE_3D
	void
NUSDAS_WRITE_3D(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
	const char *plane,
	const char *element,
	const N_SI4 *nrecs,
	const void *udata,
	const char *utype,
	const N_SI4 *usize,
	N_SI4 *result)
{
	*result = NuSDaS_write_3d(type1,
		type2,
		type3,
		basetime,
		(const char (*)[4])member,
		validtime,
		(const char (*)[6])plane,
		(const char (*)[6])element,
		nrecs,
		udata,
		utype,
		usize);
}
