/** @file
 * @brief API nusdas_inq_subcinfo() �μ���
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_err.h"
#include "sys_time.h"
#include "dset.h"
#include "glb.h"

struct inq_aux_info {
	nusdims_t	nusdims;
	N_SI4		query;
	sym4_t		group;
	void		*buf;
	N_UI4		bufnelems;
};

	static int
inq_aux_dsselect(nusdset_t *ds, void *arg)
{
	struct inq_aux_info *info = arg;
	int r;
	r = ds_inq_aux(ds, &info->nusdims, info->query, info->group,
			info->buf, info->bufnelems); 
	return r;
}

/** @brief SUBC/INFO ����礻
 *
 * ���� @p type1 ���� @p validtime �ǻ��ꤵ���ǡ����ե�����˽񤫤줿
 * SUBC �ޤ��� INFO ��Ͽ�ˤĤ��ơ�
 * ���� @p query �ǻ��ꤵ�����礻��Ԥ���
 * <DL>
 * <DT>N_SUBC_NUM<DD>
 * SUBC ��Ͽ�θĿ���4�Х����������ѿ� @p buf �˽񤫤�롣
 * ���� @p group ��̵�뤵��롣
 * <DT>N_SUBC_LIST<DD>
 * �ǡ����ե������������줿 SUBC ��Ͽ�η�̾������ @p buf �˽񤫤�롣
 * ���� @p buf ��Ĺ�� 4 ʸ����ʸ������
 * @p N_SUBC_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * ���� @p group ��̵�뤵��롣
 * <DT>N_SUBC_NBYTES<DD>
 * ��̾ @p group �� SUBC ��Ͽ�ΥХ��ȿ���4�Х����������ѿ� @p buf �˽񤫤�롣
 * <DT>N_SUBC_CONTENT<DD>
 * ��̾ @p group �� SUBC ��Ͽ������ @p buf �˽񤫤�롣
 * ��ҤΥХ��ȿ�������Ĺ������ݤ��Ƥ����ͤФʤ�ʤ���
 * <DT>N_INFO_NUM<DD>
 * INFO ��Ͽ�θĿ���4�Х����������ѿ� @p buf �˽񤫤�롣
 * ���� @p group ��̵�뤵��롣
 * <DT>N_INFO_LIST<DD>
 * �ǡ����ե������������줿 INFO ��Ͽ�η�̾������ @p buf �˽񤫤�롣
 * ���� @p buf ��Ĺ�� 4 ʸ����ʸ������
 * @p N_INFO_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * ���� @p group ��̵�뤵��롣
 * <DT>N_INFO_NBYTES<DD>
 * ��̾ @p group �� INFO ��Ͽ�ΥХ��ȿ���4�Х����������ѿ� @p buf �˽񤫤�롣
 * </DL>
 *
 * @retval �� ��Ǽ���ǿ�
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 �ǿ��ߤ��줿��
 */
	N_SI4
NuSDaS_inq_subcinfo(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������ */
		const char member[4], /**< ���С� */
		const N_SI4 *validtime, /**< �оݻ��� */
		N_SI4 query, /**< ��礻���� */
		const char group[4], /**< ��̾ */
		void *buf, /**< INTENT(OUT) ��̳�Ǽ���� */
		const N_SI4 bufnelems) /**< ��̳�Ǽ��������ǿ� */
{
	struct inq_aux_info info;
	nustype_t	type;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_inq_subcinfo"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime, 1, &info.nusdims);
	info.query = query;
	info.group = MEM2SYM4(group);
	info.buf = buf;
	info.bufnelems = bufnelems;
	r = nusglb_dsscan_nustype(inq_aux_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	if (r == 0) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %#ys not found", &type));
	}
	return r;
}

