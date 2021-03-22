/** @file
 * @brief nusdas_subc_srf_ship �μ���
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_kwd.h"
#include "sys_time.h"
#include <stddef.h>
#include "glb.h"
#include "dset.h"
#include "sys_err.h"
#include <string.h>
#include <ctype.h>
# define NEED_STR3SYM4UPCASE
# define NEED_PACK2NUSBMV
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "sys_endian.h"
#include "sys_kwd.h"
# define NEED_MAKE_UI8
#include "sys_int.h"

struct subcship_param {
	nusdims_t dims;
	N_SI4 lat;
	N_SI4 lon;
};

	INLINE int
subcship_recsize(nusdset_t *ds, N_UI4 siz,
		struct subcship_param *param,
		N_UI4 *ofs, N_UI4 *arynelems)
{
	N_UI4 nm, nv, iv, im;
	N_UI8 vt;
	vt = make_ui8(param->dims.validtime1, param->dims.validtime2);
	ds_inq_cntl(ds, &param->dims, SYM4_MIDX, &im, &param->dims.member, 1);
	ds_inq_cntl(ds, &param->dims, SYM4_VIDX, &iv, &vt, 1);
	ds_inq_cntl(ds, &param->dims, N_MEMBER_NUM, &nm, 0, 1);
	ds_inq_cntl(ds, &param->dims, N_VALIDTIME_NUM, &nv, 0, 1);
	if ((N_SI4)im < 0) {
		return nus_err((NUSERR_SC_BadArg, "member \"%Ps\" not found in %#ys",
			param->dims.member, &ds->comm.nustype));
	}
	if ((N_SI4)iv < 0) {
		return nus_err((NUSERR_SC_BadArg, "validtime \"%T/%T\" not found in %#ys",
			param->dims.validtime1, param->dims.validtime2, &ds->comm.nustype));
	}
	*ofs = nv * im + iv;
	*arynelems = nm * nv;
	/* check size */
	if (siz < *arynelems * 8) {
		return nus_err((NUSERR_SC_SizeMismatch,
				"record size %Pu < needed %Pu",
				siz, *arynelems * 8));
	}
	return 0;
}

	static int
subcship_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds, N_SI4 ofs_flg)
{
	const N_UI4 *rec = vrec;
	struct subcship_param *param = vparam;
	N_UI4 ofs, arynelems;
	int r;
	if (ofs_flg == 1){
		if ((r = subcship_recsize(ds, siz, param, &ofs, &arynelems)) != 0) {
			return r;
		}
	} else{
		ofs = 0;
		arynelems = 1;
	}
	param->lat = NTOH4(rec[ofs]);
	param->lon = NTOH4(rec[ofs + arynelems]);
	return 0;
}

	static int
subcship_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct subcship_param *param = vparam;
	int r;
	r = ds_read_aux(ds, &param->dims, SYM4_SUBC, SYM4_LOCA,
			subcship_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		return 0;
	}
	return 1;
}

	static int
subcship_get(nustype_t *type, struct subcship_param *param)
{
	int r;
	r = nusglb_dsscan_nustype(subcship_get_dsselect, type, param);
	if (r > 0) { 
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

	static int
subcship_put_encode(void *vrec, N_UI4 siz, void *vparam, union nusdset_t *ds)
{
	N_UI4 *rec = vrec;
	struct subcship_param *param = vparam;
	N_UI4 ofs, arynelems;
	int r;
	if ((r = subcship_recsize(ds, siz, param, &ofs, &arynelems)) != 0) {
		return r;
	}
	rec[ofs] = HTON4(param->lat);
	rec[ofs + arynelems] = HTON4(param->lon);
	return 0;
}

	static int
subcship_put(nustype_t *type, struct subcship_param *param)
{
	union nusdset_t *ds;
	ds = nusglb_find_dset(type);
	if (ds == 0) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, &param->dims, SYM4_SUBC, SYM4_LOCA,
			~(size_t)0,
			subcship_put_encode, param);
}

/** @brief SUBC LOCA �ؤΥ������� */
	N_SI4
NuSDaS_subc_srf_ship2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		N_SI4	*lat, /**< ���� */
		N_SI4	*lon, /**< ���� */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	struct subcship_param param;
	nustype_t type;
	sym4_t op;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	op = str3sym4upcase(getput);
	nus_debug(("--- subc_ship %Ps", op));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &param.dims);
	if (op == SYM4_GET) {
		r = subcship_get(&type, &param);
		*lat = param.lat;
		*lon = param.lon;
	} else if (op == SYM4_PUT) {
		param.lat = *lat;
		param.lon = *lon;
		r = subcship_put(&type, &param);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC LOCA �ؤΥ�������
 * ���졼�����δ�¬�ǡ����˴ؤ��������������(���١�����)�ؤ�
 * �����������󶡤��롣
 * @retval 0 ���ｪλ
 * @retval -2 �׵ᤵ�줿�쥳���ɤ�¸�ߤ��ʤ����ޤ��Ͻ񤫤�Ƥ��ʤ���
 * @retval -3 �쥳���ɥ�����������
 * @retval -5 �����ϻؼ�������
 * <H3> ���� </H3>
 * ���δؿ��� NuSDaS1.0 ����¸�ߤ��������ɥ�����Ȥ���Ƥ��ʤ��ä���
 */
	N_SI4
NuSDaS_subc_srf_ship(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		N_SI4	*lat, /**< ���� */
		N_SI4	*lon, /**< ���� */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4 unity = 1;
	return NuSDaS_subc_srf_ship2(type1, type2, type3, basetime, member,
			validtime, &unity, lat, lon, getput);
}
