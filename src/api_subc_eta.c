/** @file
 * @brief ����ͽ��� nusdas_subc_* �μ���
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
# define NEED_POKE_FLOAT
# define NEED_MEMCPY_NTOH4
#include "sys_endian.h"

struct sigmeta_get_param {
	sym4_t	group;
	const nusdims_t *dims;
	N_SI4	*n_levels;
	float	*a;
	float	*b;
	float	*c;
};

/** @brief ETA/SIGM �� SUBC �ɼ� (�Хǡ����ե����륳����Хå�)
 * @note rec, siz ������ե����� subcntl ʸ�˽�Ĺ��������������ʬ
 */
	static int
sigmeta_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char	*rec = vrec;
	struct sigmeta_get_param *param = vparam;
	N_UI4	nlev;
	memcpy_ntoh4(&nlev, rec, 1);
	if (~nlev == 0) {
		/* ��ϿĹ�ȵ�Ͽ��α�ľ�����������
		 * 0xFF �� memset ���줿 SUBC ���ɤ����������
		 */
		memset(param->a, 0xFF, sizeof(float) * (nlev + 1));
		memset(param->b, 0xFF, sizeof(float) * (nlev + 1));
		memset(param->c, 0xFF, sizeof(float));
		return nus_err((NUSERR_SC_Uninitialized,
			"Uninitialized SUBC %Ps",
			param->group));
	}
	if (siz != ((nlev + 1) * 8 + 8)) {
		return nus_err((NUSERR_SC_SizeMismatch,
			"Broken SUBC siz %Pu != expected %Pu",
			siz, (nlev + 1) * 8 + 8));
	}
	if (nlev > (N_UI4)*param->n_levels) {
		return nus_err((NUSERR_SC_ShortBuf,
			"SUBC levels %Pd > your buffer %Pd",
			nlev, *param->n_levels));
	}
	*param->n_levels = nlev;
	memcpy_ntoh4(param->a, rec + 4, nlev + 1);
	memcpy_ntoh4(param->b, rec + 4 + (nlev  + 1) * 4, nlev + 1);
	memcpy_ntoh4(param->c, rec + 4 + (nlev  + 1) * 8, 1);
	return 0;
}

/** @brief ETA/SIGM �� SUBC �ɼ� (�Хǡ������åȥ�����Хå�) */
	static int
sigmeta_get_dsselect(nusdset_t *ds, void *vparam)
{
	int r;
	struct sigmeta_get_param *param = vparam;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, param->group,
			sigmeta_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* ���դ���ʤ����õ��³�� */
		return 0;
	}
	/* ���դ���Ф��Υǡ������åȤǽ���� */
	return 1;
}

/** @brief ETA/SIGM �� SUBC �ɼ�μ�Ư���� */
	static int
sigmeta_get(nustype_t *type, const nusdims_t *dims, sym4_t group, 
		N_SI4 *n_levels, float *a, float *b, float *c)
{
	struct sigmeta_get_param param;
	int	r;
	param.group = group;
	param.dims = dims;
	param.n_levels = n_levels;
	param.a = a;
	param.b = b;
	param.c = c;
	r = nusglb_dsscan_nustype(sigmeta_get_dsselect, type, &param);
	nus_debug(("nusglb_dsscan_nustype => %d", r));
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		switch (r) {
			case 0:
			case NUSERR_OpenRFailed:
				r = NUSERR_NoDfileToRead;
			default:
				/* do nothing */;
		}
		return r;
	}
}

struct sigmeta_put_param {
	N_UI4	n_levels;
	const float *a;
	const float *b;
	float c;
};

	static int
sigmeta_put_encode(void *rec, N_UI4 siz UNUSED, void *arg,
		union nusdset_t *ds UNUSED)
{
	struct sigmeta_put_param *param = arg;
	N_UI1 *buf = rec;
	unsigned i;
	float *xa, *xb;
	xa = (float *)(buf + 4);
	xb = xa + (param->n_levels + 1);
	POKE_N_UI4(buf, param->n_levels);
	for (i = 0; i < param->n_levels + 1; i++) {
		POKE_float(&xa[i], param->a[i]);
		POKE_float(&xb[i], param->b[i]);
	}
	POKE_float(&xb[param->n_levels + 1], param->c);
	return 0;
}

/** @brief ETA/SIGM �� SUBC �񤭹��ߤμ�Ư���� */
	static int
sigmeta_put(nustype_t *type, nusdims_t *dims, sym4_t group,
		N_UI4 n_levels, const float *a, const float *b, float c)
{
	union nusdset_t *ds;
	struct sigmeta_put_param param;
	param.n_levels = n_levels;
	param.a = a;
	param.b = b;
	param.c = c;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, dims, SYM4_SUBC, group, (n_levels + 2) * 8,
		sigmeta_put_encode, &param);
}

/** @brief SUBC ETA �ؤΥ������� */
	N_SI4
NuSDaS_subc_eta2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		N_SI4 *n_levels, /**< ��ľ�ؿ� */
		float a[], /**< ���� a */
		float b[], /**< ���� b */
		float *c, /**< ���� c */
		const char getput[3])
		/**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	nustype_t	type;
	nusdims_t	dims;
	sym4_t		op;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_subc_eta2"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	op = str3sym4upcase(getput);
	if (op == SYM4_GET) {
		r = sigmeta_get(&type, &dims, SYM4_ETA,
				n_levels, a, b, c);
	} else if (op == SYM4_PUT) {
		r = sigmeta_put(&type, &dims, SYM4_ETA,
				*n_levels, a, b, *c);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC ETA �ؤΥ�������
 * ��ľ��ɸ�� ETA ��ɸ�Ϥ��Ѥ���Ȥ��ˡ���ľ��ɸ������ѥ�᡼���ؤ�
 * �����������󶡤��롣 
 * �ѥ�᡼����4�Х���ñ������ư��������������@p a, @p b, @p c �ǹ������졢
 * @p a, @p b, �ϱ�ľ�ؿ� @p n_levels ���Ф��ơ�@p n_levels+1 ���Ǥ�����
 * @p c ��1���Ǥ�����(�ѿ�)����ݤ���ɬ�פ����롣
 * n_levels �� nusdas_subc_inq_nz ���䤤��碌�뤳�Ȥ��Ǥ��롣
 * @retval 0 ���ｪλ
 * @retval -2 �쥳���ɤ�¸�ߤ��ʤ����ޤ��ϥ쥳���ɤν񤭹��ߤ�����Ƥ��ʤ���
 * @retval -3 �쥳���ɥ�����������
 * @retval -4 �桼�����α�ľ�ؿ����ե��������α�ľ�ؿ���꾮����
 * @retval -5 �����ϻؼ���������
 * <H3> ���� </H3>
 * ���δؿ��� NuSDaS1.0 ����¸�ߤ�����
 * NuSDaS1.1�ޤǤϡ��쥳���ɤ��񤭹��ޤ줿���ξ���������碌�Ƥ��ʤ��ä�
 * �����̵��Ͽ�Υ쥳���ɤ�ե����뤫���ɤ�����ｪλ���Ƥ�����NuSDaS1.3�Ǥ�
 * �ե�����ν�������˥쥳���ɤ���������̤��Ͽ��Ƚ��Ǥ���褦�ˤ�����
 * ���ξ��Υ��顼��-2�Ȥ��Ƥ��롣
 * <H3> ��� </H3>
 * SUBC ETA �˻Ȥ��Ƥ����ľ�ؿ� @p n_levels �ϼºݤΥ�ǥ�α�ľ�ؿ���
 * �ۤʤäƤ����礬����Τǡ�������ݤκݤˤ�nusdas_subc_inq_nz���䤤
 * ��碌����̤��Ѥ��뤳�ȡ�
 *
*/
	N_SI4
NuSDaS_subc_eta(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		N_SI4 *n_levels, /**< ��ľ�ؿ� */
		float a[], /**< ���� a */
		float b[], /**< ���� b */
		float *c, /**< ���� c */
		const char getput[3])
		/**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4	unity = 1;
	return NuSDaS_subc_eta2(type1, type2, type3, basetime,
			member, validtime, &unity, n_levels, a, b, c,
			getput);
}

/** @brief SUBC ETA/SIGM �Υǥե���������� 
 * �ե����뤬���������������ݤ�ETA, SIGM�˽񤭹����ͤ����ꤹ�롣
 * SIGM ������ˤĤ��Ƥ� nusdas_subc_eta �򻲾ȡ�
 * �����Ρַ�̾�פˤϡ�"ETA " �ޤ��� "SIGM" ����ꤹ�롣
 * @retval 0 ���ｪλ
 * @retval -1 ����ե�����˻��ꤷ����̾����Ͽ����Ƥ��ʤ�
 * @retval -2 ����γ��ݤ˼��Ԥ���
 * @retval -3 �쥳���ɤΥ�����������
 *
 * <H3> �ߴ��� </H3>
 * NuSDaS1.1 �Ǥϡ���Ĥ�NuSDaS�ǡ������åȤ�����Ǥ�������������ο��Ϻ���
 * 10 �����¤���Ƥ��ꡢ�����Ķ�����-2���֤��줿��������NuSDaS1.3 �Ǥ�
 * ���꤬���ݤǤ���¤�������¤Ϥʤ���-2 �������ݼ��ԤΥ��顼�����ɤ�
 * �ɤ��ؤ��Ƥ��롣
*/
	N_SI4
NuSDaS_subc_preset1(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const char group[4], /**< ��̾ */
		const N_SI4 *n_levels, /**< ��ľ�ؿ� */
		float a[], /**< ���� a */
		float b[], /**< ���� b */
		float *c) /**< ���� c */
{
	struct sigmeta_put_param param;
	nustype_t nustype;
	nusdset_t *ds;
	sym4_t grp;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	param.n_levels = *n_levels;
	param.a = a;
	param.b = b;
	param.c = *c;
	pack2nustype(type1, type2, type3, &nustype);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		r = nus_err((NUSERR_DsetNotFound, 
			     "missing dataset %Qs.%Ps.%Ps",
			     nustype.type1, nustype.type2, nustype.type3));
		NUSDAS_CLEANUP;
		return r;
	}
	grp = str2sym4(group);
	r = ds_subcpreset(ds, grp,
			*n_levels * 8 + 16,
			sigmeta_put_encode, &param);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC SIGM �ؤΥ�������
 * ��ľ��ɸ�� ETA ��ɸ�Ϥ��Ѥ���Ȥ��ˡ���ľ��ɸ������ѥ�᡼���ؤ�
 * �����������󶡤��롣 
 * �ؿ��λ��ͤϡ�nusdas_subc_eta ��Ʊ���Ǥ��롣
 */
	N_SI4
NuSDaS_subc_sigm(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		N_SI4 *n_levels, /**< ��ľ�ؿ� */
		float a[], /**< ���� a */
		float b[], /**< ���� b */
		float *c, /**< ���� c */
		const char getput[3])
		/**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4	unity = 1;
	return NuSDaS_subc_sigm2(type1, type2, type3, basetime,
			member, validtime, &unity, n_levels, a, b, c,
			getput);
}

/** @brief SUBC SIGM �ؤΥ������� */
	N_SI4
NuSDaS_subc_sigm2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		N_SI4 *n_levels, /**< ��ľ�ؿ� */
		float a[], /**< ���� a */
		float b[], /**< ���� b */
		float *c, /**< ���� c */
		const char getput[3])
		/**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	nustype_t	type;
	nusdims_t	dims;
	sym4_t		op;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_subc_sigm2"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	op = str3sym4upcase(getput);
	if (op == SYM4_GET) {
		r = sigmeta_get(&type, &dims, SYM4_SIGM,
				n_levels, a, b, c);
	} else if (op == SYM4_PUT) {
		r = sigmeta_put(&type, &dims, SYM4_SIGM,
				*n_levels, a, b, *c);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}
