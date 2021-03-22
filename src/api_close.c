/** @file
 * @brief nusdas_onefile_close(), nusdas_allfile_close() �μ���
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include "dset.h"
#include "glb.h"
#include "sys_time.h"
# define NEED_PACK2NUSBMV
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "sys_err.h" /* for NUS_ERR_CODE() */

/** @brief �ǡ������åȤ��Ĥ���ݤΥ��ץ����ȷ�̾���
 */
struct All_File_Close_Info {
	N_SI4	param;		/**< @brief �Ĥ���ǡ����ե�����μ���
				  @todo ���ΤȤ���̵�뤵��Ƥ��� */
	int	ok;		/**< @brief �Ĥ����ǡ����ե������ */
	int	ng;		/**< @brief �Ĥ���ݤ˥��顼�ˤʤä�
				  �ǡ����ե������ */
};

/** @brief �ҤȤĤΥǡ������åȤ��Ĥ���
 */
static int
All_File_Close_Callback(union nusdset_t *ds, void *arg)
{
	struct All_File_Close_Info *info = arg;
	int	r;
	r = ds_close(ds, info->param);
	if (r > 0) {
		info->ok += r;
	} else if (r < 0) {
		info->ng += r;
	}
	return 0;
}

/** @brief ���ƤΥǡ����ե�������Ĥ���
 *
 * ���ޤǤ� NuSDaS ���󥿡��ե������ǳ��������ƤΥե�������Ĥ���.
 * ���� param �ϼ��Τ����줫���Ѥ���:
 * <DL>
 * <DT>N_FOPEN_READ<DD>�ɤ߹����Ѥ˳������ե�����������Ĥ���
 * <DT>N_FOPEN_WRITE<DD>�񤭹��߲Ĥǳ������ե�����������Ĥ���
 * <DT>N_FOPEN_ALL<DD>���٤ƤΥե�������Ĥ���
 * </DL>
 *
 * @retval �� ������Ĥ���줿�ե�����ο�
 * @retval 0 �Ĥ���٤��ե����뤬�ʤ��ä�
 * @retval �� �Ĥ���ݤ˥��顼�������ä��ե�����ο�
 *
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.0 ����¸�ߤ���.
 */
	N_SI4
NuSDaS_allfile_close(N_SI4 param /**< �Ĥ���ե�����μ��� */)
{
	struct All_File_Close_Info info;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	info.param = param;
	info.ok = info.ng = 0;
	nus_debug(("--- NuSDaS_allfile_close %Pd", param));
	nusglb_dsscan(All_File_Close_Callback, &info);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return info.ng ? info.ng : info.ok;
}

/** @brief ����ǡ����ե�������Ĥ���
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.0 ����¸�ߤ���.
 */
	N_SI4
NuSDaS_onefile_close(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime) /**< �оݻ��� */
{
	nustype_t nustype;
	nusdims_t dims;
	union nusdset_t *ds;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime, 1, &dims);
	nus_debug(("--- NuSDaS_onefile_close %#ys/%#ms", &nustype, &dims));
	SETERR(NUSERR_DsetNotFound);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUS_ERR_CODE();
	}
	r = ds_close_file(ds, &dims);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief �ҤȤĤΥե�������Ĥ���
 */
	N_SI4
NuSDaS_onefile_close2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2) /**< �оݻ���2(�̻�ʬ) */
{
	nustype_t nustype;
	nusdims_t dims;
	union nusdset_t *ds;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	nus_debug(("--- NuSDaS_onefile_close2 %#ys/%#ms", &nustype, &dims));
	SETERR(NUSERR_DsetNotFound);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUS_ERR_CODE();
	}
	r = ds_close_file(ds, &dims);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}
