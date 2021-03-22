#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_time.h"
#include "sys_kwd.h"
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "glb.h"
#include <stddef.h>
#include "dset.h"
#include "sys_err.h"

/** @brief nusdas_inq_nrdbtime() ���� btlist_dsselect() ���Ϥ�������
 */
struct btlist_dsselect_info {
	N_SI4           *buf;
	N_SI4           bufnelems;
	int             verbose;
};

/** �ƥǡ������åȤ��Ф��� btlist ����.
 * @todo ���������� 1 ���֤��� nusglb_dsscan() ��ߤ�뤳��
 */
	static int
btlist_dsselect(nusdset_t *ds, void *arg)
{
	struct btlist_dsselect_info *info = arg;
	int r;

	r = ds_btlist(ds, info->buf, info->bufnelems,
		      info->verbose);
	nus_debug(("ds_btlist => %d", r));
	if (r < 0) {
		/* �̤�ݤ��ɤˤ�����Τǥ��顼�����ɤ򥻡��� */
		SETERR(r);
		return 0;
	}
	return r;
}

/** @brief �ǡ������åȤδ�����ꥹ�ȼ���
 *
 * ����1������3�ǻؼ������ǡ������åȤ�¸�ߤ���������
 * ���� @p btlist �˽񤭹��ࡣ
 * ���� @p pflag �������ͤ����ꤹ���ư������ξ����ٹ��å������Ȥ���
 * ��������褦�ˤʤ롣
 * @retval ���� ������θĿ�
 * @retval -1 �ե����� I/O ���顼
 * @retval -2 �ե�����˴�������¸�ߤ��ʤ�
 * @retval -3 �ե�����Υ쥳����Ĺ������
 * @retval -4 �ե����뤢�뤤�ϥǥ��쥯�ȥ�Υ����ץ�˼���
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.0 ����¸�ߤ�����
 * <H3>���</H3>
 * <UL>
 * <LI>
 * ����Ĺ @p btlistsize ���¿���δ����郎¸�ߤ�����ϡ�
 * ����Ĺ��ۤ��ƽ񤭹��ळ�ȤϤʤ����꥿���󥳡��ɤ�����Ĺ����Ӥ��ơ�
 * �꥿���󥳡��ɤ��礭���ä��餽�ο������������ݤ�ľ����
 * �ܴؿ���Ƥ�ľ�����Ȥˤ�ꡢ���٤ƤΥꥹ�Ȥ����뤳�Ȥ��Ǥ��롣
 * <LI>
 * NuSDaS 1.1 �ޤǤϸ��դ��ä��ǡ������åȤ��ͥåȥ���Ǥʤ���С�
 * ����ˤĤ��Ƥ���õ�����Ԥ�줿��
 * NuSDaS 1.3 ����ϡ�
 * ���ꤷ�����̤˥ޥå����뤹�٤ƤΥǡ������åȤˤĤ���õ�����Ԥ��롣
 * <LI>
 * ���̤��б�����ǡ������åȤ����Ĥ���ʤ����
 * (���Ȥ��м���̾��ְ㤨�����)��
 * �ֵ��ͤϥ���Ȥʤ롣
 * �ǡ������åȤ�¸�ߤ��ƶ��ξ��Ȱۤʤꡢ
 * ���ΤȤ� ``Can not find NUSDAS root directory for selected type1-3''
 * ``type1$<$...$>$ type2$<$...$>$ type3$<$...$>$ NRD=...''
 * �Ȥ�����å�������ɸ�२�顼���Ϥ�ɽ������롣
 * NRD= �θ�ο��ͤ� -1 �Ǥʤ���С�
 * NRD �ֹ����ꤷ�������¸�ߤ��Ƥ���ǡ������åȤ����Ĥ���ʤ��ʤäƤ���
 * ��ǽ�������롣
 * </UL>
 * */
	N_SI4
NuSDaS_inq_nrdbtime(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		N_SI4 *btlist, /**< INTENT(OUT) �����郎��Ǽ��������� */
		const N_SI4 *btlistsize, /**< ��������ǿ� */
		N_SI4 pflag) /**< ư����������ե饰 */
{
	nustype_t	type;
	N_SI4		r;
	struct btlist_dsselect_info info;

	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	info.buf = btlist;
	info.bufnelems = *btlistsize;
	info.verbose = pflag;
	r = nusglb_dsscan_nustype(btlist_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
	}
	NUSDAS_CLEANUP;
	return r;
}
