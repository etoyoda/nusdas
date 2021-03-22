#include <nusdas.h>

#undef nusdas_read2_raw_
	void
nusdas_read2_raw_(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	const char *plane1,
	const char *plane2,
	const char *element,
	void *buf,
	const N_SI4 *buf_nbytes,
	N_SI4 *result)
{
	*result = NuSDaS_read2_raw(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		plane1,
		plane2,
		element,
		buf,
		buf_nbytes);
}
