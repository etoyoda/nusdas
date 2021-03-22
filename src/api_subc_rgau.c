/** @file
 * @brief nusdas_subc_rgau() �μ���
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <ctype.h>
#include "sys_kwd.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
# define NEED_STR3SYM4UPCASE
#include "sys_sym.h"
#include <stddef.h>
# define NEED_MEMCPY_NTOH4
# define NEED_MEMCPY_HTON4
#include "sys_endian.h"
#include <string.h>
#include "sys_time.h"
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

/** @brief RGAU �� SUBC ���ɤ߼�뤿��˼����Ϥ����� */
struct subcrgau_get_param {
	const nusdims_t *dims;
	N_SI4	*j;
	N_SI4	*j_start;
	N_SI4	*j_n;
	N_SI4	*i;
	N_SI4	*i_start;
	N_SI4	*i_n;
	float	*lat;
};

	static int
subcrgau_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct subcrgau_get_param *param = vparam;
	N_UI4	nj;
	memcpy_ntoh4(&nj, rec + 8, 1);
	if (~nj == 0) {
		nj = *param->j_n;
		memset(param->j, 0xFF, sizeof(float));
		memset(param->j_start, 0xFF, sizeof(float));
		memset(param->i, 0xFF, sizeof(float) * nj);
		memset(param->i_start, 0xFF, sizeof(float) * nj);
		memset(param->i_n, 0xFF, sizeof(float) * nj);
		memset(param->lat, 0xFF, sizeof(float) * nj);
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	if (siz != (nj * 16 + 12)) {
		return nus_err((NUSERR_SC_SizeMismatch, "Broken SUBC/RGAU"));
	}
	if (nj > (N_UI4)(*param->j_n)) {
		return nus_err((NUSERR_SC_BadArg,
					"SUBC nj %Pd > given buffer %Pd",
					nj, param->j_n));
	}
	memcpy_ntoh4(param->j, rec + 0, 1);
	memcpy_ntoh4(param->j_start, rec + 4, 1);
	*param->j_n = nj;
	memcpy_ntoh4(param->i, rec + 12, nj);
	memcpy_ntoh4(param->i_start, rec + 12 + nj * 4, nj);
	memcpy_ntoh4(param->i_n, rec + 12 + nj * 8, nj);
	memcpy_ntoh4(param->lat, rec + 12 + nj * 12, nj);
	return 0;
}

	static int
subcrgau_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct subcrgau_get_param *param = vparam;
	int r;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, SYM4_RGAU,
			subcrgau_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* �ߤĤ���ʤ��Τ�õ��³�� */
		return 0;
	}
	return 1;
}

	static N_SI4
subcrgau_get(nustype_t *type,
		struct subcrgau_get_param *param)
{
	int r;
	r = nusglb_dsscan_nustype(subcrgau_get_dsselect, type, param);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

/** @brief RGAU �� SUBC ��񤭽Ф�����˼����Ϥ����� */
struct subcrgau_put_param {
	const nusdims_t *dims;
	N_SI4	j;
	N_SI4	j_start;
	N_SI4	j_n;
	const N_SI4	*i;
	const N_SI4	*i_start;
	const N_SI4	*i_n;
	const float	*lat;
};

	static int
subcrgau_put_encode(void *rec, N_UI4 siz UNUSED, void *arg,
		union nusdset_t *ds UNUSED)
{
	struct subcrgau_put_param *param = arg;
	N_UI1 *buf = rec;
	POKE_N_UI4(buf, param->j);
	POKE_N_UI4(buf + 4, param->j_start);
	POKE_N_UI4(buf + 8, param->j_n);
	memcpy_hton4(buf + 12, param->i, param->j_n);
	memcpy_hton4(buf + 12 + param->j_n * 4, param->i_start, param->j_n);
	memcpy_hton4(buf + 12 + param->j_n * 8, param->i_n, param->j_n);
	memcpy_hton4(buf + 12 + param->j_n * 12, param->lat, param->j_n);
	return 0;
}

static int
subcrgau_put_arg_check(struct subcrgau_put_param *param)
{
	int counter;

	/* --- Checking input data --- */
	if ( param->j < 1 ) {
		return nus_err((NUSERR_SC_BadInput, 
				" Parameter j must be larger than 1. "
				"Your input j = %d", param->j));
	} else if ((param->j_start > param->j) || param->j_start < 1 ) {
		return  nus_err((NUSERR_SC_BadInput, 
				 " Parameter j_start must be smaller than j "
				 "and larger than 1. "
				 "Your input j_start = %d, j = %d",
				 param->j_start, param->j));
	} else if ((param->j_n > ( param->j - (param->j_start) + 1 )) 
		   || param->j_n < 1) {
		return  nus_err((NUSERR_SC_BadInput, 
				 " Parameter j_n must be smaller than "
				 "j - j_start and larger than 1. "
				 "Your input j_n = %d, j = %d, j_start = %d.",
				 param->j_n, param->j, param->j_start));
	} else {
		for(counter = 0; counter < param->j_n ; counter++){
			if ( param->i[counter] < 1 ) {
				return nus_err((NUSERR_SC_BadInput, 
						" Parameter i must be larger "
						"than 1. "
						"Your input i[%d] = %d",
						counter, param->i[counter]));
			}
			if ((param->i_start[counter] > param->i[counter]) 
			    || (param->i_start[counter] < 1)) {
				return nus_err((NUSERR_SC_BadInput, 
						" Parameter i_start must be "
						"smaller than i and larger "
						"than 1. Your input "
						"i_start[%d] = %d, i[%d] = %d",
						counter, 
						param->i_start[counter], 
						counter, 
						param->i[counter]));
			}
			if ((param->i_n[counter] > 
			     (param->i[counter] - param->i_start[counter] + 1)) 
			    || (param->i_n[counter] < 1)) {
				return nus_err((NUSERR_SC_BadInput, 
						"Parameter i_n must be "
						"smaller than i - i_start "
						"and larger than 1. "
						"Your input i_n[%d] = %d, "
						"i[%d] = %d, i_start[%d] = %d.",
						counter, 
						param->i_n[counter], 
						counter, 
						param->i[counter], 
						counter, 
						param->i_start[counter]));
			}
		}
	}
	return 0;
}


/** @brief RGAU �� SUBC �񤭹��ߤμ�Ư */
	static int
subcrgau_put(nustype_t *type,
		nusdims_t *dims,
		struct subcrgau_put_param *param)
{
	union nusdset_t *ds;
	int r;

	if ((r = subcrgau_put_arg_check(param)) != 0) {
		return r;
	}
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, dims, SYM4_SUBC, SYM4_RGAU,
			param->j_n * 16 + 12,
			subcrgau_put_encode, param);
}

/** @brief SUBC RGAU �ؤΥ������� */
	N_SI4
NuSDaS_subc_rgau2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		N_SI4 *j, /**< ���������ʬ��� */
		N_SI4 *j_start, /**< �ǡ����κ��̳ʻҤ��ֹ�(1�Ϥޤ�) */
		N_SI4 *j_n, /**< �ǡ��������̳ʻҿ� */
		N_SI4 i[], /**< ����������ʻҿ� */
		N_SI4 i_start[], /**< �ǡ����κ����ʻҤ��ֹ�(1�Ϥޤ�) */
		N_SI4 i_n[], /**< �ǡ����������ʻҿ� */
		float lat[], /**< ���� */
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
		struct subcrgau_get_param param;
		param.dims = &dims;
		param.j = j;
		param.j_start = j_start;
		param.j_n = j_n;
		param.i = i;
		param.i_start = i_start;
		param.i_n = i_n;
		param.lat = lat;
		r = subcrgau_get(&type, &param);
	} else if (op == SYM4_PUT) {
		struct subcrgau_put_param param;
		param.j = *j;
		param.j_start = *j_start;
		param.j_n = *j_n;
		param.i = i;
		param.i_start = i_start;
		param.i_n = i_n;
		param.lat = lat;
		r = subcrgau_put(&type, &dims, &param);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC RGAU �ؤΥ������� 
 * Reduced Gauss �ʻҤ�Ȥ����������������ؤΥ����������󶡤��롣
 * �����ϻؼ��� @p GET �ξ��ˤ����Ƥ⡢j_n ���ͤϥ��åȤ��롣���� j_n ���ͤ�
 * nusdas_subc_rgau_inq_jn ��Ȥä��䤤��碌�Ǥ��롣
 * i, i_start, i_n, lat �� j_n ���Ǥ��ä�������Ѱդ��롣
 * @retval 0 ���ｪλ
 * @retval -2 �쥳���ɤ�¸�ߤ��ʤ����ޤ��Ͻ񤭹��ޤ�Ƥ��ʤ���
 * @retval -3 �������ξ��󤬰���������ե�������԰���
 * @retval -4 ���ꤷ��������(j_n, j_start, j_n, i, i_start, i_n)������(PUT�ΤȤ��Τ�)
 * @retval -5 �����ϻؼ�������
 * @retval -6 ���ꤷ��������(j_n)������(GET�ΤȤ��Τ�)
 * <H3> ��� </H3>
 * Reduced Gauss �ʻҤ�Ȥ�����1�����ǥǡ������Ǽ����Τǡ�����ե������
 * size(�ʻҿ�)�ˤ� (�ºݤγʻҿ�) 1 �Ȼ��ꤹ�롣�ޤ���SUBC �Υ������� 
 * 16 * j_n + 12 ��׻������ͤ�����ե�����˽񤯡�
 * <H3> ���� </H3>
 * ���δؿ���NuSDaS1.2�Ǽ������줿
*/
	N_SI4
NuSDaS_subc_rgau(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		N_SI4 *j, /**< ���������ʬ��� */
		N_SI4 *j_start, /**< �ǡ����κ��̳ʻҤ��ֹ�(1�Ϥޤ�) */
		N_SI4 *j_n, /**< �ǡ��������̳ʻҿ� */
		N_SI4 i[], /**< ����������ʻҿ� */
		N_SI4 i_start[], /**< �ǡ����κ����ʻҤ��ֹ�(1�Ϥޤ�) */
		N_SI4 i_n[], /**< �ǡ����������ʻҿ� */
		float lat[], /**< ���� */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4	unity = 1;
	return nusdas_subc_rgau2(type1, type2, type3,
			basetime, member, validtime, &unity,
			j, j_start, j_n, i, i_start, i_n, lat, getput);
}

/** @brief SUBC RGAU �Υǥե�����ͤ�����
 * �ե����뤬���������������ݤ�RGAU�쥳���ɤ˽񤭹����ͤ����ꤹ�롣
 * RGAU �쥳���ɤ�����ˤĤ��Ƥ� nusdas_subc_rgau �򻲾ȡ�
 * @retval 0 ���ｪλ
 * @retval -1 ����ե������ "RGAU" ����Ͽ����Ƥ��ʤ�
 * @retval -2 ����γ��ݤ˼��Ԥ���
 * <H3> �ߴ��� </H3>
 * NuSDaS1.1 �Ǥϡ���Ĥ�NuSDaS�ǡ������åȤ�����Ǥ�������������ο��Ϻ���
 * 10 �����¤���Ƥ��ꡢ�����Ķ�����-2���֤��줿��������NuSDaS1.3 �Ǥ�
 * ���꤬���ݤǤ���¤�������¤Ϥʤ���-2 �������ݼ��ԤΥ��顼�����ɤ�
 * �ɤ��ؤ��Ƥ��롣
 */
	N_SI4
NuSDaS_subc_rgau_preset1(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *j, /**< ���������ʬ��� */
		const N_SI4 *j_start, /**< �ǡ����κ��̳ʻҤ��ֹ�(1�Ϥޤ�) */
		const N_SI4 *j_n, /**< �ǡ��������̳ʻҿ� */
		const N_SI4 i[], /**< ����������ʻҿ� */
		const N_SI4 i_start[], /**< �ǡ����κ����ʻҤ��ֹ�(1�Ϥޤ�) */
		const N_SI4 i_n[], /**< �ǡ����������ʻҿ� */
		const float lat[]) /**< ���� */
{
	struct subcrgau_put_param param;
	nustype_t nustype;
	nusdset_t *ds;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	param.j = *j;
	param.j_start = *j_start;
	param.j_n = *j_n;
	param.i = i;
	param.i_start = i_start;
	param.i_n = i_n;
	param.lat = lat;
	if ((r = subcrgau_put_arg_check(&param)) != 0) {
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
	r = ds_subcpreset(ds, SYM4_RGAU,
			*j_n * 16 + 12,
			subcrgau_put_encode, &param);
	NUSDAS_CLEANUP;
	return r;
}

