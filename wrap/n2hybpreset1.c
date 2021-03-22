#include <nusdas.h>

#undef nusdas_subc_zhyb_preset1
	void
nusdas_subc_zhyb_preset1(const char *type1,
	const char *type2,
	const char *type3,
	const N_SI4 *nz,
	const float *ptrf,
	const float *presrf,
	const float *zrp,
	const float *zrw,
	const float *vctrans_p,
	const float *vctrans_w,
	const float *dvtrans_p,
	const float *dvtrans_w,
	N_SI4 *result)
{
	*result = NuSDaS_subc_zhyb_preset1(type1,
		type2,
		type3,
		nz,
		ptrf,
		presrf,
		zrp,
		zrw,
		vctrans_p,
		vctrans_w,
		dvtrans_p,
		dvtrans_w);
}
