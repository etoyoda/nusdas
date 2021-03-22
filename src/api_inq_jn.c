/** @file
 * @brief nusdas_subc_rgau_inq_jn() �μ���
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_time.h"
#include "sys_kwd.h"
#include <ctype.h>
#include <stddef.h>
#include "sys_err.h"
# define NEED_MEMCPY_NTOH4
#include "sys_endian.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
#include "sys_sym.h"
#include "dset.h"
#include "glb.h"

/** @brief RGAU �� SUBC ���ɤ߼�뤿��˼����Ϥ����� */
struct inq_jn_param {
	const nusdims_t *dims;
	N_SI4	j_n;
};

	static int
sigmrgau_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct inq_jn_param *param = vparam;
	N_UI4	nj;
	memcpy_ntoh4(&nj, rec + 8, 1);
	if (~nj == 0) {
		param->j_n = -1;
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	if (siz != (nj * 16 + 12)) {
		return nus_err((NUSERR_SC_SizeMismatch, "Broken SUBC/RGAU"));
	}
	param->j_n = nj;
	return 0;
}

	static int
sigmrgau_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct inq_jn_param *param = vparam;
	int r;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, SYM4_RGAU,
			sigmrgau_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* �ߤĤ���ʤ��Τ�õ��³�� */
		return 0;
	}
	return 1;
}

/** @brief SUBC RGAU ��Ͽ���礭������礻 */
	N_SI4
NuSDaS_subc_rgau_inq_jn2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		N_SI4 *j_n) /**< INTENT(OUT) ���̳ʻҿ� */
{
	struct inq_jn_param param;
	nusdims_t	dims;
	nustype_t	type;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	param.dims = &dims;
	r = nusglb_dsscan_nustype(sigmrgau_get_dsselect, &type, &param);
	*j_n = param.j_n;
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	if (r > 0) {
		return r;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

/** @brief SUBC RGAU ��Ͽ���礭������礻
 * RGAU �˵�Ͽ����Ƥ��� j_n (���̳ʻҿ�) ���䤤��碌�롣
 * @retval �� ���ｪλ
 * @retval -2 �׵ᤵ�줿�쥳���ɤ�¸�ߤ��ʤ����ޤ��Ͻ񤭹��ޤ�Ƥ��ʤ���
 * @retval -3 �쥳���ɤΥ�����������
 * <H3> ���� </H3>
 * ���δؿ��� NuSDaS1.2��Ƴ�����줿��
*/
	N_SI4
NuSDaS_subc_rgau_inq_jn(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		N_SI4 *j_n) /**< INTENT(OUT) ���̳ʻҿ� */
{
	N_SI4 unity = 1;
	return NuSDaS_subc_rgau_inq_jn2(type1, type2, type3,
			basetime, member, validtime, &unity, j_n);
}
