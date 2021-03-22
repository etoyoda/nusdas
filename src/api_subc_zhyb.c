/** @file
 * @brief nusdas_subc_zhyb() �μ���
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <string.h>
#include <stddef.h>
# define NEED_MEMCPY_NTOH4
# define NEED_MEMCPY_HTON4
# define NEED_POKE_FLOAT
#include "sys_endian.h"
#include <ctype.h>
#include "sys_kwd.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
# define NEED_STR3SYM4UPCASE
#include "sys_sym.h"
#include "sys_time.h"
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

/** @brief ZHYB �� SUBC ���ɤ߼�뤿��˼����Ϥ����� */
struct subczhyb_get_param {
	const nusdims_t *dims;
	N_SI4	nz;
	float	ptrf;
	float	presrf;
	float	*zrp;
	float	*zrw;
	float	*vctrans_p;
	float	*vctrans_w;
	float	*dvtrans_p;
	float	*dvtrans_w;
};

/** @brief ZHYB �� SUBC �ɼ� (�Хǡ����ե����륳����Хå�)
 * @note rec, siz ������ե����� subcntl ʸ�˽�Ĺ�����б�
 */
	static int
subczhyb_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct subczhyb_get_param *param = vparam;
	N_UI4	nlev;
	memcpy_ntoh4(&nlev, rec, 1);
	if (~nlev == 0) {
		/* ��������줿�ޤ޽񤫤�Ƥ��ʤ���Ͽ���ɤ����� */
		nlev = param->nz;
		memset(&param->ptrf, 0xFF, sizeof(float));
		memset(&param->presrf, 0xFF, sizeof(float));
		memset(param->zrp, 0xFF, sizeof(float) * nlev);
		memset(param->zrw, 0xFF, sizeof(float) * nlev);
		memset(param->vctrans_p, 0xFF, sizeof(float) * nlev);
		memset(param->vctrans_w, 0xFF, sizeof(float) * nlev);
		memset(param->dvtrans_p, 0xFF, sizeof(float) * nlev);
		memset(param->dvtrans_w, 0xFF, sizeof(float) * nlev);
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	if (siz != (nlev * 24 + 12)) {
		return nus_err((NUSERR_SC_SizeMismatch, "Broken SUBC/ZHYB"));
	}
	if (nlev > (N_UI4)param->nz) {
		return nus_err((NUSERR_SC_BadArg,
					"SUBC levels %Pd > given buffer %Pd",
					nlev, param->nz));
	}
	param->nz = nlev;
	memcpy_ntoh4(&param->ptrf, rec + 4, 1);
	memcpy_ntoh4(&param->presrf, rec + 8, 1);
	memcpy_ntoh4(param->zrp, rec + 12, nlev);
	memcpy_ntoh4(param->zrw, rec + 12 + nlev * 4, nlev);
	memcpy_ntoh4(param->vctrans_p, rec + 12 + nlev * 4 * 2, nlev);
	memcpy_ntoh4(param->vctrans_w, rec + 12 + nlev * 4 * 3, nlev);
	memcpy_ntoh4(param->dvtrans_p, rec + 12 + nlev * 4 * 4, nlev);
	memcpy_ntoh4(param->dvtrans_w, rec + 12 + nlev * 4 * 5, nlev);
	return 0;
}

/** @brief ZHYB �� SUBC �ɼ� (�Хǡ������åȥ�����Хå�) */
	static int
subczhyb_get_dsselect(nusdset_t *ds, void *vparam)
{
	int	r;
	struct subczhyb_get_param *param = vparam;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, SYM4_ZHYB,
			subczhyb_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* ���դ���ʤ����õ��³�� */
		return 0;
	}
	return 1;
}

/** @brief ZHYB �� SUBC �ɼ�μ�Ư */
	static N_SI4
subczhyb_get(nustype_t *type,
		nusdims_t *dims,
		N_SI4 *nz,
		float *ptrf,
		float *presrf,
		float zrp[],
		float zrw[],
		float vctrans_p[],
		float vctrans_w[],
		float dvtrans_p[],
		float dvtrans_w[])
{
	struct subczhyb_get_param param;
	int	r;
	param.dims = dims;
	param.nz = *nz;
	param.zrp = zrp;
	param.zrw = zrw;
	param.vctrans_p = vctrans_p;
	param.vctrans_w = vctrans_w;
	param.dvtrans_p = dvtrans_p;
	param.dvtrans_w = dvtrans_w;
	r = nusglb_dsscan_nustype(subczhyb_get_dsselect, type, &param);
	if (r > 0) {
		*nz = param.nz;
		*ptrf = param.ptrf;
		*presrf = param.presrf;
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

/** @brief ZHYB �� SUBC ��񤭽Ф�����˼����Ϥ����� */
struct subczhyb_put_param {
	const nusdims_t *dims;
	N_SI4	nz;
	float	ptrf;
	float	presrf;
	const float	*zrp;
	const float	*zrw;
	const float	*vctrans_p;
	const float	*vctrans_w;
	const float	*dvtrans_p;
	const float	*dvtrans_w;
};

	static int
subczhyb_put_encode(void *rec, N_UI4 siz UNUSED, void *arg,
		union nusdset_t *ds UNUSED)
{
	struct subczhyb_put_param *param = arg;
	N_UI1 *buf = rec;
	POKE_N_UI4(buf, param->nz);
	POKE_float(buf + 4, param->ptrf);
	POKE_float(buf + 8, param->presrf);
	memcpy_hton4(buf + 12, param->zrp, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 4, param->zrw, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 8, param->vctrans_p, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 12, param->vctrans_w, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 16, param->dvtrans_p, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 20, param->dvtrans_w, param->nz);
	return 0;
}

static int
subczhyb_put_arg_check(struct subczhyb_put_param *param)
{
	/* --- Checking input data --- */
	if ( param->nz < 1 ){
		return nus_err((NUSERR_SC_BadInput, 
				" Parameter nz must be larger than 1. "
				"Your input nz = %d",
				param->nz));
	} else if ( param->presrf <= 0.e0 ){
		return nus_err((NUSERR_SC_BadInput, 
				" Parameter presrf must be larger than 0. "
				"Your input presrf = %f",
				param->presrf));
	}
	return 0;
}


/** @brief ZHYB �� SUBC �񤭹��ߤμ�Ư */
	static int
subczhyb_put(nustype_t *type,
		nusdims_t *dims,
		N_SI4 nz,
		float ptrf,
		float presrf,
		float zrp[],
		float zrw[],
		float vctrans_p[],
		float vctrans_w[],
		float dvtrans_p[],
		float dvtrans_w[])
{
	struct subczhyb_put_param param;
	union nusdset_t *ds;
	int r;
	param.nz = nz;
	param.ptrf = ptrf;
	param.presrf = presrf;
	param.zrp = zrp;
	param.zrw = zrw;
	param.vctrans_p = vctrans_p;
	param.vctrans_w = vctrans_w;
	param.dvtrans_p = dvtrans_p;
	param.dvtrans_w = dvtrans_w;
	if ((r = subczhyb_put_arg_check(&param)) != 0) {
		return r;
	}
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, dims, SYM4_SUBC, SYM4_ZHYB,
			nz * 24 + 12,
			subczhyb_put_encode, &param);
}

/** @brief SUBC ZHYB �ؤΥ������� */
	N_SI4
NuSDaS_subc_zhyb2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		N_SI4 *nz, /**< ��ľ�ؿ� */
		float *ptrf, /**< ���̤λ����� */
		float *presrf, /**< �����λ����� */
		float zrp[], /**< ��ǥ��̹��� (�ե��٥�) */
		float zrw[], /**< ��ǥ��̹��� (�ϡ��ե�٥�) */
		float vctrans_p[], /**< ��ɸ�Ѵ��ؿ� (�ե��٥�) */
		float vctrans_w[], /**< ��ɸ�Ѵ��ؿ� (�ϡ��ե�٥�) */
		float dvtrans_p[], /**< ��ɸ�Ѵ��ؿ��α�ľ��ʬ (�ե��٥�) */
		float dvtrans_w[], /**< ��ɸ�Ѵ��ؿ��α�ľ��ʬ (�ϡ��ե�٥�) */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	nustype_t	type;
	nusdims_t	dims;
	sym4_t		op;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	op = str3sym4upcase(getput);
	if (op == SYM4_GET) {
		r = subczhyb_get(&type, &dims, nz, ptrf, presrf, zrp, zrw,
				vctrans_p, vctrans_w, dvtrans_p, dvtrans_w);
	} else if (op == SYM4_PUT) {
		r = subczhyb_put(&type, &dims, *nz, *ptrf, *presrf, zrp, zrw,
				vctrans_p, vctrans_w, dvtrans_p, dvtrans_w);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC ZHYB �ؤΥ������� 
 * ��ľ��ɸ�˱�ľ�ϥ��֥�åɺ�ɸ���Ȥ����������������ZHYB
 * �ؤΥ����������󶡤��롣
 * �����ϻؼ��� @p GET �ξ��ˤ����Ƥ⡢nz ���ͤϥ��åȤ��롣���� nz ���ͤ�
 * nusdas_subc_eta_inq_nz ��Ȥä��䤤��碌�Ǥ��롣
 * zrp, zrw, vctrans_p, vctrans_w, dvtrans_p, dvtrans_w �� 
 * nz ���Ǥ��ä�������Ѱդ��롣
 * @retval 0 ���ｪλ
 * @retval -2 �쥳���ɤ�¸�ߤ��ʤ����ޤ��Ͻ񤭹��ޤ�Ƥ��ʤ���
 * @retval -3 �������ξ��󤬰���������ե�������԰���
 * @retval -4 ���ꤷ��������(ptrf, presrf)������(PUT�ΤȤ��Τ�)
 * @retval -5 �����ϻؼ�������
 * @retval -6 ���ꤷ��������(nz)������(GET�ΤȤ��Τ�)
 * <H3> ��� </H3>
 * SUBC �Υ������� 24 * nz + 12 ��׻������ͤ�����ե�����˽񤯡�
 * <H3> ���� </H3>
 * ���δؿ���NuSDaS1.2�Ǽ������줿
*/
	N_SI4
NuSDaS_subc_zhyb(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		N_SI4 *nz, /**< ��ľ�ؿ� */
		float *ptrf, /**< ���̤λ����� */
		float *presrf, /**< �����λ����� */
		float zrp[], /**< ��ǥ��̹��� (�ե��٥�) */
		float zrw[], /**< ��ǥ��̹��� (�ϡ��ե�٥�) */
		float vctrans_p[], /**< ��ɸ�Ѵ��ؿ� (�ե��٥�) */
		float vctrans_w[], /**< ��ɸ�Ѵ��ؿ� (�ϡ��ե�٥�) */
		float dvtrans_p[], /**< ��ɸ�Ѵ��ؿ��α�ľ��ʬ (�ե��٥�) */
		float dvtrans_w[],
		/**< ��ɸ�Ѵ��ؿ��α�ľ��ʬ (�ϡ��ե�٥�) */
		const char getput[3])
		/**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4	unity = 1;
	return nusdas_subc_zhyb2(type1, type2, type3,
			basetime, member, validtime, &unity,
			nz, ptrf, presrf, zrp, zrw,
			vctrans_p, vctrans_w,
			dvtrans_p, dvtrans_w,
			getput);
}

/** @brief SUBC ZHYB �Υǥե�����ͤ�����
 * �ե����뤬���������������ݤ�ZHYB�쥳���ɤ˽񤭹����ͤ����ꤹ�롣
 * ZHYB �쥳���ɤ�����ˤĤ��Ƥ�nusdas_subc_zhyb �򻲾ȡ�
 * @retval 0 ���ｪλ
 * @retval -1 ����ե������ "ZHYB" ����Ͽ����Ƥ��ʤ�
 * @retval -2 ����γ��ݤ˼��Ԥ���
 * <H3> �ߴ��� </H3>
 * NuSDaS1.1 �Ǥϡ���Ĥ�NuSDaS�ǡ������åȤ�����Ǥ�������������ο��Ϻ���
 * 10 �����¤���Ƥ��ꡢ�����Ķ�����-2���֤��줿��������NuSDaS1.3 �Ǥ�
 * ���꤬���ݤǤ���¤�������¤Ϥʤ���-2 �������ݼ��ԤΥ��顼�����ɤ�
 * �ɤ��ؤ��Ƥ��롣
*/
	N_SI4
NuSDaS_subc_zhyb_preset1(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *nz, /**< ��ľ�ؿ� */
		const float *ptrf, /**< ���̤λ����� */
		const float *presrf, /**< �����λ����� */
		const float zrp[], /**< ��ǥ��̹��� (�ե��٥�) */
		const float zrw[], /**< ��ǥ��̹��� (�ϡ��ե�٥�) */
		const float vctrans_p[], /**< ��ɸ�Ѵ��ؿ� (�ե��٥�) */
		const float vctrans_w[], /**< ��ɸ�Ѵ��ؿ� (�ϡ��ե�٥�) */
		const float dvtrans_p[],
		/**< ��ɸ�Ѵ��ؿ��α�ľ��ʬ (�ե��٥�) */
		const float dvtrans_w[])
		/**< ��ɸ�Ѵ��ؿ��α�ľ��ʬ (�ϡ��ե�٥�) */
{
	struct subczhyb_put_param param;
	nustype_t nustype;
	nusdset_t *ds;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	param.nz = *nz;
	param.ptrf = *ptrf;
	param.presrf = *presrf;
	param.zrp = zrp;
	param.zrw = zrw;
	param.vctrans_p = vctrans_p;
	param.vctrans_w = vctrans_w;
	param.dvtrans_p = dvtrans_p;
	param.dvtrans_w = dvtrans_w;
	if ((r = subczhyb_put_arg_check(&param)) != 0) {
		NUSDAS_CLEANUP;
		/* for compatibility with NuSDaS1.2 */
		if (r == NUSERR_SC_BadInput) r = -3; 
		return r;
	}
	pack2nustype(type1, type2, type3, &nustype);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		r = nus_err((NUSERR_DsetNotFound, 
			     "missing dataset %Qs.%Ps.%Ps",
			     nustype.type1, nustype.type2,
			     nustype.type3));
		NUSDAS_CLEANUP;
		return r;
	}
	r = ds_subcpreset(ds, SYM4_ZHYB,
			*nz * 24 + 12,
			subczhyb_put_encode, &param);
	NUSDAS_CLEANUP;
	return r;
}
