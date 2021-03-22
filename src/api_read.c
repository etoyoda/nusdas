/** \file
 * \brief nusdas_read �μ�����
 */

#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "internal_types.h"
# define NEED_PACK2NUSDIMS
# define NEED_PACK2NUSTYPE
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_time.h"
#include "dset.h"
#include "dfile.h"
#include "glb.h"
#include "sys_err.h"

/** @brief nusdas_read() ���� read_dsselect() ���Ϥ�������
 */
struct read_dsselect_info {
	nusdims_t	nusdims;
	struct ibuffer_t buf;
	int		readsize;
};

/** �ƥǡ������åȤ��Ф��� read ����.
 * @todo ���������� 1 ���֤��� nusglb_dsscan() ��ߤ�뤳��
 */
	static int
read_dsselect(nusdset_t *ds, void *arg)
{
	struct read_dsselect_info *info = arg;
	info->readsize = ds_readdata(ds, &(info->nusdims), &(info->buf));
	nus_debug(("ds_readdata => %d", info->readsize));
	if (info->readsize < 0) {
		/* �̤�ݤ��ɤˤ�����Τǥ��顼�����ɤ򥻡��� */
		SETERR(info->readsize);
		return 0;
	}
	SETERR(0);
	return 1;
}

/** @brief �ǡ�����Ͽ���ɼ�
 * �����ǻ��ꤷ��TYPE, ��������С����оݻ���̡����ǤΥǡ�����
 * �ɤ߽Ф��� 
 * @retval �� �ɤ߽Ф��Ƴ�Ǽ�����ʻҿ�
 * @retval 0 ���ꤷ���ǡ�����̤��Ͽ(����ե������ elementmap �ˤ�äƽ񤭹��ޤ�뤳�Ȥϵ��Ƥ���Ƥ��뤬���ޤ��ǡ������񤭹��ޤ�Ƥ��ʤ�)
 * @retval -2 ���ꤷ���ǡ����ϵ�Ͽ���뤳�Ȥ����Ƥ���Ƥ��ʤ�(elementmap �ˤ�äƶػߤ���Ƥ�����Ȼ��ꤷ����̾������̾����Ͽ����Ƥ��ʤ�����ξ����ޤ�)��
 * @retval -4 ��Ǽ������­
 * @retval -5 ��Ǽ����η��ȥ쥳���ɤε�Ͽ������������
 *
 * <H3> ��� </H3>
 * nusdas_read �Ǥϡ��ֵ��� 0 �ϥ��顼�Ǥ��뤳�Ȥ���դ�ɬ�ס�
 * nusdas_read �Υ��顼�����å����ֵ��ͤ����Ƥ���ʻҿ��Ȱ��פ��Ƥ��뤳��
 * ���ǧ����Τ�˾�ޤ�����
 * <H3> �ߴ��� </H3>
 * NuSDaS1.1 �Ǥϡ֥���󥰥����̤ǡ��ǡ�������������ͤ�Ķ���Ƥ����
 * (�ֵ���-6)���������Ƥ��������ϥǡ����κǽ������
 * ���Ƥ�������ǰ�̣���ʤ��Ȼפ���Τǡ�NuSDaS1.3 �ǤϤ��Υ��顼��
 * �֤��ʤ����ޤ����֥桼���������ץ�ե�����δ��������ϥ��ɥ쥹��������
 * �Ǥ����(�ֵ���-7)�ϡ�������ʬ��-54��-57���б�����Τǡ����Υ��顼���֤��ʤ�
*/
	N_SI4
NuSDaS_read(
		const char utype1[8], /**< ����1 */
		const char utype2[4], /**< ����2 */
		const char utype3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С� */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		const char plane[6], /**< �̤�̾�� */
		const char element[6], /**< ����̾ */
		void *data, /**< INTENT(OUT) ��̳�Ǽ���� */
		const char fmt[2], /**< ��̳�Ǽ����η� */
		const N_SI4 *size) /**< ��̳�Ǽ��������ǿ� */
{
	struct read_dsselect_info info;
	nustype_t	type;
	int		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(utype1, utype2, utype3, &type);
	pack2nusdims(*basetime, member, *validtime, 1, plane, NULL,
			element, &info.nusdims);
	nus_debug(("--- NuSDaS_read %#ys/%#ms", &type, &info.nusdims));
	info.buf.ib_ptr = data;
	info.buf.ib_fmt = mem2sym4(fmt);
	info.buf.nelems = *size;
	cut_rectangle_disable(&info.buf.ib_cut);
	r = nusglb_dsscan_nustype(read_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		NUSDAS_CLEANUP;
		return info.readsize;
	} else {
		NUSDAS_CLEANUP;
		return NUS_ERR_CODE();
	}
}

/** @brief �ǡ�����Ͽ���ɼ�
 */
	N_SI4
NuSDaS_read2(
		const char utype1[8], /**< ����1 */
		const char utype2[4], /**< ����2 */
		const char utype3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С� */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		const char plane1[6], /**< �̤�̾��1 */
		const char plane2[6], /**< �̤�̾��2 */
		const char element[6], /**< ����̾ */
		void *data, /**< INTENT(OUT) ��̳�Ǽ���� */
		const char fmt[2], /**< ��̳�Ǽ����η� */
		const N_SI4 *size) /**< ��̳�Ǽ��������ǿ� */
{
	struct read_dsselect_info info;
	nustype_t	type;
	int		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- NuSDaS_read2"));
	pack2nustype(utype1, utype2, utype3, &type);
	pack2nusdims(*basetime, member, *validtime1, *validtime2,
			plane1, plane2,
			element, &(info.nusdims));
	info.buf.ib_ptr = data;
	info.buf.ib_fmt = mem2sym4(fmt);
	info.buf.nelems = *size;
	cut_rectangle_disable(&info.buf.ib_cut);
	r = nusglb_dsscan_nustype(read_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	if (r > 0) {
		return info.readsize;
	} else {
		return NUS_ERR_CODE();
	}
}
