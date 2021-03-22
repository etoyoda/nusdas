#include <nusdas.h>

#undef nusdas_subc_zhyb2
	void
nusdas_subc_zhyb2(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime1,
	const N_SI4 *validtime2,
	N_SI4 *nz,
	float *ptrf,
	float *presrf,
	float *zrp,
	float *zrw,
	float *vctrans_p,
	float *vctrans_w,
	float *dvtrans_p,
	float *dvtrans_w,
	const char *getput,
	N_SI4 *result)
{
	*result = NuSDaS_subc_zhyb2(type1,
		type2,
		type3,
		basetime,
		member,
		validtime1,
		validtime2,
		nz,
		ptrf,
		presrf,
		zrp,
		zrw,
		vctrans_p,
		vctrans_w,
		dvtrans_p,
		dvtrans_w,
		getput);
}
