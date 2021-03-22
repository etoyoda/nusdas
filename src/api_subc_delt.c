/** @file
 * @brief nusdas_subc_delt() �μ���
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
# define NEED_POKE_FLOAT
#include "sys_endian.h"
#include <string.h>
#include "sys_time.h"
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

/** @brief ZHYB �� SUBC ���ɤ߼�뤿��˼����Ϥ����� */
struct subcdelt_get_param {
	const nusdims_t *dims;
	float delt;
};

	static int
subcdelt_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct subcdelt_get_param *param = vparam;
	N_UI4	dummy;
	memcpy_ntoh4(&dummy, rec, 1);
	memcpy_ntoh4(&param->delt, rec, 1);
	if (~dummy == 0) {
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	if (siz != 4) {
		return nus_err((NUSERR_SC_SizeMismatch, "Broken SUBC/DELT size"));
	}
	return 0;
}

	static int
subcdelt_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct subcdelt_get_param *param = vparam;
	int r;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, SYM4_DELT,
			subcdelt_get_decode, vparam);
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
subcdelt_get(nustype_t *type,
		struct subcdelt_get_param *param)
{
	int r;
	r = nusglb_dsscan_nustype(subcdelt_get_dsselect, type, param);
	if (r > 0) {
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

/** @brief ZHYB �� SUBC ��񤭽Ф�����˼����Ϥ����� */
struct subcdelt_put_param {
	float	delt;
};

	static int
subcdelt_put_encode(void *rec, N_UI4 siz UNUSED, void *arg,
		union nusdset_t *ds UNUSED)
{
	struct subcdelt_put_param *param = arg;
	N_UI1 *buf = rec;
	POKE_float(buf, param->delt);
	return 0;
}


/** @brief ZHYB �� SUBC �񤭹��ߤμ�Ư */
	static int
subcdelt_put(nustype_t *type,
		nusdims_t *dims,
		struct subcdelt_put_param *param)
{
	union nusdset_t *ds;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, dims, SYM4_SUBC, SYM4_DELT, 4,
			subcdelt_put_encode, param);
}

/** @brief SUBC DELT �ؤΥ������� */
	N_SI4
NuSDaS_subc_delt2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		float *delt, /**< DELT ���ͤؤΥݥ��� */
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
		struct subcdelt_get_param param;
		param.dims = &dims;
		r = subcdelt_get(&type, &param);
		if (r == 0) nuserr_cancel(MARK_FOR_DSET);
		*delt = param.delt;
	} else if (op == SYM4_PUT) {
		struct subcdelt_put_param param;
		param.delt = *delt;
		r = subcdelt_put(&type, &dims, &param);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC DELT �ؤΥ������� 
 * ��ǥ�λ�����ʬ�ֳ֤������������˵�Ͽ���Ƥ�����ΤǤ��롣
 * @retval 0 ���ｪλ
 * @retval -2 �쥳���ɤ�¸�ߤ��ʤ����ޤ��Ͻ񤭹��ޤ�Ƥ��ʤ���
 * @retval -3 �쥳���ɥ�����������
 * @retval -5 �����ϻؼ�������
 * <H3> ���� </H3>
 * ���δؿ��� NuSDaS1.2��Ƴ�����줿��
*/
	N_SI4
NuSDaS_subc_delt(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		float *delt, /**< DELT ���ͤؤΥݥ��� */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4	unity = 1;
	return nusdas_subc_delt2(type1, type2, type3,
			basetime, member, validtime, &unity,
			delt, getput);
}

/** @brief SUBC DELT �Υǥե��������
 * �ե����뤬���������������ݤ�DELT�쥳���ɤ˽񤭹����ͤ����ꤹ�롣
 * DELT �쥳���ɤ�����ˤĤ��Ƥ�nusdas_subc_delt �򻲾ȡ�
 * @retval 0 ���ｪλ
 * @retval -1 ����ե������ "DELT" ����Ͽ����Ƥ��ʤ�
 * @retval -2 ����γ��ݤ˼��Ԥ���
 * <H3> �ߴ��� </H3>
 * NuSDaS1.1 �Ǥϡ���Ĥ�NuSDaS�ǡ������åȤ�����Ǥ�������������ο��Ϻ���
 * 10 �����¤���Ƥ��ꡢ�����Ķ�����-2���֤��줿��������NuSDaS1.3 �Ǥ�
 * ���꤬���ݤǤ���¤�������¤Ϥʤ���-2 �������ݼ��ԤΥ��顼�����ɤ�
 * �ɤ��ؤ��Ƥ��롣
 */
	N_SI4
NuSDaS_subc_delt_preset1(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const float *delt) /**< DELT ���ͤؤΥݥ��� */
{
	struct subcdelt_put_param param;
	nustype_t nustype;
	nusdset_t *ds;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	param.delt = *delt;
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
	r = ds_subcpreset(ds, SYM4_DELT, 4,
			subcdelt_put_encode, &param);
	NUSDAS_CLEANUP;
	return r;
}

