/** @file
 * @brief nusdas_subc_tdif �μ���
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

struct subctdif_param {
	nusdims_t dims;
	N_SI4 diff_time;
	N_SI4 total_sec;
};

	INLINE int
subctdif_recsize(nusdset_t *ds, N_UI4 siz,
		struct subctdif_param *param,
		N_UI4 *ofs, N_UI4 *arynelems)
{
	N_UI4 nm, nv, iv, im;
	N_UI8 vt;
	vt = make_ui8(param->dims.validtime1, param->dims.validtime2);
	ds_inq_cntl(ds, &param->dims, SYM4_MIDX, &im, &param->dims.member, 1);
	ds_inq_cntl(ds, &param->dims, SYM4_VIDX, &iv, &vt, 1);
	ds_inq_cntl(ds, &param->dims, N_MEMBER_NUM, &nm, NULL, 1);
	ds_inq_cntl(ds, &param->dims, N_VALIDTIME_NUM, &nv, NULL, 1);
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
subctdif_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds, N_SI4 ofs_flag)
{
	const N_UI4 *rec = vrec;
	struct subctdif_param *param = vparam;
	N_UI4 ofs, arynelems;
	int r;
	if (ofs_flag == 1){
		if ((r = subctdif_recsize(ds, siz, param, &ofs, &arynelems)) != 0) {
			return r;
		}
	} else{
		ofs = 0;
		arynelems = 1;
	}
	/* decode */
	param->diff_time = NTOH4(rec[ofs]);
	param->total_sec = NTOH4(rec[ofs + arynelems]);
	return 0;
}

	static int
subctdif_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct subctdif_param *param = vparam;
	int r;
	r = ds_read_aux(ds, &param->dims, SYM4_SUBC, SYM4_TDIF,
			subctdif_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		return 0;
	}
	return 1;
}

	static int
subctdif_get(nustype_t *type, struct subctdif_param *param)
{
	int r;
	r = nusglb_dsscan_nustype(subctdif_get_dsselect, type, param);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

	static int
subctdif_put_encode(void *vrec, N_UI4 siz, void *vparam, union nusdset_t *ds)
{
	N_UI4 *rec = vrec;
	struct subctdif_param *param = vparam;
	N_UI4 ofs, arynelems;
	int r;
	if ((r = subctdif_recsize(ds, siz, param, &ofs, &arynelems)) != 0) {
		return r;
	}
	rec[ofs] = HTON4(param->diff_time);
	rec[ofs + arynelems] = HTON4(param->total_sec);
	return 0;
}

	static int
subctdif_put(nustype_t *type, struct subctdif_param *param)
{
	nusdset_t *ds;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, &param->dims, SYM4_SUBC, SYM4_TDIF,
			~(size_t)0,
			subctdif_put_encode, param);
}

/** @brief SUBC TDIF �ؤΥ������� */
	N_SI4
NuSDaS_subc_tdif2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		N_SI4	*diff_time, /**< �оݻ��狼��Τ���(��) */
		N_SI4	*total_sec, /**< ��ͽ�����(��) */
		const char getput[3])
		/**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	struct subctdif_param param;
	nustype_t type;
	sym4_t op;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	op = str3sym4upcase(getput);
	nus_debug(("--- subc_tdif %Ps", op));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &param.dims);
	if (op == SYM4_GET) {
		r = subctdif_get(&type, &param);
		*diff_time = param.diff_time;
		*total_sec = param.total_sec;
	} else if (op == SYM4_PUT) {
		param.diff_time = *diff_time;
		param.total_sec = *total_sec;
		r = subctdif_put(&type, &param);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC TDIF �ؤΥ�������
 * ��Ǽ�����ͤλ�����оݻ��֤ȤΤ��졢�ѻ����֤��Ǽ������������� TDIF 
 * �ؤΥ����������󶡤��롣
 * @retval 0 ���ｪλ
 * @retval -2 �׵ᤵ�줿�쥳���ɤ�¸�ߤ��ʤ����ޤ��Ͻ񤭹��ޤ�Ƥ��ʤ���
 * @retval -3 �쥳���ɥ�����������
 * @retval -5 �����ϻؼ�������
 *
 * <H3>��­</H3>
 * <UL>
 * <LI> diff_time = �����ϰϻ��� - �оݻ��� [��ñ��]
 * <LI> total_sec = �����ϰϽ��� - �����ϰϻ��� [��ñ��]
 * </UL>
 *
 * <H3>����</H3>
 * ���δؿ��� NuSDaS1.0 ����¸�ߤ�����
 */
	N_SI4
NuSDaS_subc_tdif(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		N_SI4	*diff_time, /**< �оݻ��狼��Τ���(��) */
		N_SI4	*total_sec, /**< ��ͽ�����(��) */
		const char getput[3])
		/**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4 unity = 1;
	return NuSDaS_subc_tdif2(type1, type2, type3, basetime, member,
			validtime, &unity, diff_time, total_sec, getput);
}
