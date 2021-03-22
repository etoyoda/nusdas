/** @file
 * @brief nusdas_inq_cntl() �μ���
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
#include "sys_sym.h"
#include "sys_time.h"
#include <fcntl.h>
#include <stddef.h>
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

struct inq_info {
	nusdims_t	nusdims;
	N_SI4		param;
	void		*data;
	N_SI4		datasize;
};

	static int
inqcntl_dsselect(nusdset_t *ds, void *arg)
{
	struct inq_info *info = arg;
	return ds_inq_cntl(ds, &info->nusdims, info->param,
			info->data, &info->datasize, 0);
}

/** @brief �ǡ����ե�����ν�����礻 
 * ���� @p type1 ���� @p validtime �ǻ��ꤵ���ǡ����ե�����˽񤫤줿
 * CNTL ��Ͽ�ˤĤ��ơ�
 * ���� @p param �ǻ��ꤵ�����礻��Ԥ���
 * <DL>
 * <DT>N_MEMBER_NUM<DD>
 * ���С��θĿ���4�Х����������ѿ� @p data �˽񤫤�롣
 * <DT>N_MEMBER_LIST<DD>
 * �ǡ����ե������������줿���С�̾������ @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 4 ʸ����ʸ������
 * @p N_MEMBER_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_VALIDTIME_NUM<DD>
 * validtime�θĿ���4�Х����������ѿ� @p data �˽񤫤�롣
 * <DT>N_VALIDTIME_LIST<DD>
 * �ǡ����ե������������줿validtime������ @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 4 byte��������
 * @p N_VALIDTIME_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_VALIDTIME_LIST2<DD>
 * �ǡ����ե������������줿validtime2������ @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 4 byte��������
 * @p N_VALIDTIME_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_PLANE_NUM<DD>
 * �̤θĿ���4�Х����������ѿ� @p data �˽񤫤�롣
 * <DT>N_PLANE_LIST<DD>
 * �ǡ����ե������������줿�̤�̾�������� @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 6 ʸ����ʸ������
 * @p N_PLANE_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_PLANE_LIST2<DD>
 * N_PLANE_LIST ������Ʊ��ư��Ǥ��롣
 * <DT>N_ELEMENT_NUM<DD>
 * ���ǤθĿ���4�Х����������ѿ� @p data �˽񤫤�롣
 * <DT>N_ELEMENT_LIST<DD>
 * �ǡ����ե������������줿���Ǥ�̾�������� @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 6 ʸ����ʸ������
 * @p N_ELEMENT_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT> N_NUSD_NBYTES <DD>
 * NUSD �쥳���ɤΥ�����(ñ�̥Х���)��4�Х����������ѿ� @p data �˽�
 * ��롣(��Ƭ���������ղä����쥳����Ĺ���礭��(4*2�Х���)��ޤ�)
 * <DT> N_NUSD_CONTENT <DD>
 * NUSD �쥳���ɤ����Ƥ����� @p data �˳�Ǽ���롣���� @p data ��
 * <BR>N_NUSD_NBYTES �Х���¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (��Ƭ���������ղä����쥳����Ĺ��ޤ�)
 * <DT> N_CNTL_NBYTES <DD>
 * CNTL �쥳���ɤΥ�����(ñ�̥Х���)��4�Х����������ѿ� @p data �˽�
 * ��롣(��Ƭ���������ղä����쥳����Ĺ���礭��(4*2�Х���)��ޤ�)
 * <DT> N_CNTL_CONTENT <DD>
 * CNTL �쥳���ɤ����Ƥ����� @p data �˳�Ǽ���롣���� @p data ��
 * <BR>N_CNTL_NBYTES �Х���¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (��Ƭ���������ղä����쥳����Ĺ��ޤ�)
 * <DT> N_PROJECTION <DD>
 * �Ͽ����ˡ�ξ����4ʸ����ʸ���� @p data �˳�Ǽ����
 * (����ΰ�̣�ϴ�����ɽ����)��
 * <DT> N_GRID_SIZE <DD>
 * X������Y�����γʻҿ������ν����4�Х��������������� @p data ��
 * �񤫤�롣���� @p data �� 2 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_GRID_BASEPOINT <DD>
 * �������x��ɸ��y��ɸ�����١����٤����ν����4�Х���ñ������ư������������
 * �� @p data �˽񤫤�롣���� @p data �� 4 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_GRID_DISTANCE <DD>
 * X������Y�����γʻҴֳ֤����ν����4�Х���ñ������ư��������������
 * @p data �˽񤫤�롣���� @p data �� 2 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_STAND_LATLON <DD>
 * ɸ����١�ɸ����١���2ɸ����١���2ɸ����٤����ν����
 * 4�Х���ñ������ư�������������� @p data �˽񤫤�롣
 * ���� @p data �� 4 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_SPARE_LATLON <DD>
 * ����1������1������2������2�����ν����
 * 4�Х���ñ������ư�������������� @p data �˽񤫤�롣
 * ���� @p data �� 4 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_INDX_SIZE <DD>
 * INDX �θĿ��� 4�Х������������ѿ� @p data �˽񤫤�롣
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_ELEMENT_MAP <DD>
 * �ǡ����γ�Ǽ�����Ƥ���Ƥ��뤫�ݤ���1 or 0 �ˤ�äơ�1�Х���������
 * ������ @p data �˽񤫤�롣���� @p data �� @p N_INDX_SIZE ����¸��
 * ���ʤ��ƤϤʤ�ʤ���@p data�ϥ��С���validtime, �̡����Ǥ򥤥�ǥå�
 * ���ˤ�������ǡ����줾��ν���� @p N_MEMBER_LIST, 
 * @p N_VALIDTIME_LIST, @p N_PLANE_LIST, @p N_ELEMENT_LIST���䤤��碌
 * ��̤Ȱ��פ��롣
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_DATA_MAP <DD>
 * �ǡ������񤭹��ޤ�Ƥ��뤫�ݤ���1 or 0 �ˤ�äơ�1�Х���������
 * ������ @p data �˽񤫤�롣���� @p data �� @p N_INDX_SIZE ����¸��
 * ���ʤ��ƤϤʤ�ʤ���@p data�ϥ��С���validtime, �̡����Ǥ򥤥�ǥå�
 * ���ˤ�������ǡ����줾��ν���� @p N_MEMBER_LIST, 
 * @p N_VALIDTIME_LIST, @p N_PLANE_LIST, @p N_ELEMENT_LIST���䤤��碌
 * ��̤Ȱ��פ��롣
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * </DL>
 *
 * @retval �� ��Ǽ���ǿ�
 * @retval -1 �ǡ��������������­���Ƥ��롣
 * @retval -2 �ǡ��������󤬳��ݤ���Ƥ��ʤ���
 * @retval -3 �䤤��碌���ܤ�����
 * <H3> ��� </H3>
 * NuSDaS1.1�����Ǥϡ�Ʊ����¤�Υǡ������åȤǤ�
 * N_VALIDTIME_NUM, N_VALIDTIME_LIST ���䤤��碌��̤�
 * 1�Ĥ� basetime ��ʣ���� validtime ���Ǽ���뤫�ݤ��ˤ�äưۤʤäƤ�����
 * ����ϡ�validtime �ǥե������ʬ����
 * (�ۤʤ� validtime �Υե����뤬�ۤʤ�) ����ʤ��
 * �ǡ����ե�����ˤ� 1 �Ĥ� validtime �������񤫤�Ƥ�������Ǥ��롣
 * ������ NuSDaS1.3�Ǥ�����ե�����˻��ꤵ�줿���٤Ƥ� validtime ��
 * �ƥǡ����ե������ validtime �˳�Ǽ����Ƥ���Τǡ��䤤��碌��̤�
 * ��Ǽ���֤���鷺����Ǥ��롣
 */
	N_SI4
NuSDaS_inq_cntl(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		N_SI4 param, /**< ��礻���ܥ����� */
		void *data, /**< INTENT(OUT) ��礻������� */
		const N_SI4 *datasize) /**< ��礻�����������ǿ� */
{
	nustype_t nustype;
	struct inq_info info;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime, 1, &info.nusdims);
	info.param = param;
	info.data = data;
	info.datasize = *datasize;
	r = nusglb_dsscan_nustype(inqcntl_dsselect, &nustype, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	return r <= 0 ? NUS_ERR_CODE() : r;
}

/** @brief �ǡ����ե�����ν�����礻 */
	N_SI4
NuSDaS_inq_cntl2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		N_SI4 param, /**< ��礻���ܥ����� */
		void *data, /**< INTENT(OUT) ��礻������� */
		const N_SI4 *datasize) /**< ��礻�����������ǿ� */
{
	nustype_t nustype;
	struct inq_info info;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &info.nusdims);
	info.param = param;
	info.data = data;
	info.datasize = *datasize;
	r = nusglb_dsscan_nustype(inqcntl_dsselect, &nustype, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	return r <= 0 ? NUS_ERR_CODE() : r;
}
