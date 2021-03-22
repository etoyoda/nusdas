#include <nusdas.h>

#undef nusdas_subc_zhyb_
	void
nusdas_subc_zhyb_(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *basetime,
	const char *member,
	const N_SI4 *validtime,
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
	*result = NuSDaS_subc_zhyb(type1,
		type2,
		type3,
		basetime,
		member,
		validtime,
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
