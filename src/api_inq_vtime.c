#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include "sys_err.h"
#include "sys_time.h"
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "dset.h"
#include "glb.h"

/** @brief nusdas_inq_nrdvtime() ���� vtlist_dsselect() ���Ϥ�������
 */
struct vtlist_dsselect_info {
	N_SI4           *buf;
	N_SI4           bufnelems;
	N_SI4           basetime;
	int             verbose;
};

/** �ƥǡ������åȤ��Ф��� vtlist ����.
 * @todo ���������� 1 ���֤��� nusglb_dsscan() ��ߤ�뤳��
 */
	static int
vtlist_dsselect(nusdset_t *ds, void *arg)
{
	struct vtlist_dsselect_info *info = arg;
	int r;

	r = ds_vtlist(ds, info->buf, info->bufnelems, info->basetime, 
		      info->verbose);
	nus_debug(("ds_vtlist => %d", r));
	if (r < 0) {
		/* �̤�ݤ��ɤˤ�����Τǥ��顼�����ɤ򥻡��� */
		SETERR(r);
		return 0;
	}
	return r;
}

/** @brief �ǡ������åȤ��оݻ���ꥹ�ȼ���
 *
 * ����1������3�ǻؼ������ǡ������åȤ˴����� @p basetime �Τ�Ȥ�
 * ¸�ߤ����оݻ�������� @p vtlist �˽񤭹��ࡣ
 * ���� @p pflag �������ͤ����ꤹ���ư������ξ����ٹ��å������Ȥ���
 * ��������褦�ˤʤ롣
 * @retval ���� �оݻ���θĿ�
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.0 ����¸�ߤ������ɥ�����Ȥ���Ƥ��ʤ��ä���
 * <H3>���</H3>
 * <UL>
 * <LI>����Ĺ @p vtlistsize ���¿�����оݻ��郎¸�ߤ�����ϡ�
 * ����Ĺ��ۤ��ƽ񤭹��ळ�ȤϤʤ����꥿���󥳡��ɤ�����Ĺ����Ӥ��ơ�
 * �꥿���󥳡��ɤ��礭���ä��餽�ο������������ݤ�ľ����
 * �ܴؿ���Ƥ�ľ�����Ȥˤ�ꡢ���٤ƤΥꥹ�Ȥ����뤳�Ȥ��Ǥ��롣
 * <LI>�оݻ����õ���ϥե������̵ͭ�ޤ��� CNTL �쥳���ɤˤ�롣
 * �ꥹ������оݻ���ˤĤ��ƥǡ����쥳���ɤ��񤫤�Ƥ��ʤ����⤢�ꤦ�롣
 * <LI>������ @p basetime �� -1 ����ꤹ��ȡ�
 * ����������ʤ������ˤʤ롣
 * <LI>�����ˤ����äƥ��С�̾�����ʤ���
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
 */
	N_SI4
NuSDaS_inq_nrdvtime(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		N_SI4 *vtlist, /**< INTENT(OUT) �оݻ��郎�񤫤������ */
		const N_SI4 *vtlistsize, /**< ��������ǿ� */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		N_SI4 pflag) /**< ư��ٰܺ����ե饰 */
{
	nustype_t	type;
	struct vtlist_dsselect_info info;	
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	info.buf = vtlist;
	info.bufnelems = *vtlistsize;
	info.basetime = *basetime;
	info.verbose = pflag;
	r = nusglb_dsscan_nustype(vtlist_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	return r;
}

