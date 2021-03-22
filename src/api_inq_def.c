/** @file
 * @brief nusdas_inq_def() �μ���
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include <stddef.h>
#include <string.h>
#include "dset.h"
#include "glb.h"
#include "sys_err.h"
#include "sys_time.h"

/** @brief �ǡ������åȤν�����礻 
 * ���� @p type1 ���� @p type3�ǻ��ꤵ���ǡ������åȤ�����ե�����
 * �˽񤫤줿���ƤˤĤ��ơ����� @p param �ǻ��ꤵ�����礻��Ԥ���
 * <DL>
 * <DT>N_MEMBER_NUM<DD>
 * ����ե�����˽񤫤줿���С��θĿ���4�Х����������ѿ� @p data �˽񤫤�롣
 * <DT>N_MEMBER_LIST<DD>
 * ����ե�����˽񤫤줿���С�̾������ @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 4 ʸ����ʸ������
 * @p N_MEMBER_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_VALIDTIME_NUM<DD>
 * ����ե�����˽񤫤줿validtime�θĿ���4�Х����������ѿ� 
 * @p data �˽񤫤�롣
 * <DT>N_VALIDTIME_LIST<DD>
 * ����ե�����˽񤫤줿validtime������ @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 4 byte��������
 * @p N_VALIDTIME_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_VALIDTIME_LIST2<DD>
 * ����ե�����˽񤫤줿 validtime2 ������ @p data �˽񤫤�롣
 * <BR>���� @p data ��Ĺ�� 4 byte��������
 * @p N_VALIDTIME_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_VALIDTIME_UNIT<DD>
 * ����ե�����˽񤫤줿 validtime ��ñ�̤�4ʸ����ʸ�����ѿ� @p data 
 * �˽񤫤�롣
 * <DT>N_PLANE_NUM<DD>
 * ����ե�����˽񤫤줿�̤θĿ���4�Х����������ѿ� @p data �˽񤫤�롣
 * <DT>N_PLANE_LIST<DD>
 * ����ե�����˽񤫤줿�̤�̾�������� @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 6 ʸ����ʸ������
 * @p N_PLANE_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_PLANE_LIST2<DD>
 * ����ե�����˽񤫤줿��2��̾�������� @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 6 ʸ����ʸ������
 * @p N_PLANE_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_ELEMENT_NUM<DD>
 * ����ե�����˽񤫤줿���ǤθĿ���4�Х����������ѿ� @p data �˽񤫤�롣
 * <DT>N_ELEMENT_LIST<DD>
 * ����ե�����˽񤫤줿���Ǥ�̾�������� @p data �˽񤫤�롣
 * ���� @p data ��Ĺ�� 6 ʸ����ʸ������
 * @p N_ELEMENT_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT> N_PROJECTION <DD>
 * ����ե�����˽񤫤줿�Ͽ����ˡ�ξ����4ʸ����ʸ���� @p data �˳�Ǽ����
 * (����ΰ�̣�ϴ�����ɽ����)��
 * <DT> N_GRID_SIZE <DD>
 * ����ե�����˽񤫤줿X������Y�����γʻҿ������ν����4�Х�����������
 * ���� @p data �˽񤫤�롣���� @p data �� 2 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_GRID_BASEPOINT <DD>
 * ����ե�����˽񤫤줿�������x��ɸ��y��ɸ�����١����٤�
 * ���ν����4�Х���ñ������ư�������������� @p data �˽񤫤�롣
 * ���� @p data �� 4 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_GRID_DISTANCE <DD>
 * ����ե�����˽񤫤줿X������Y�����γʻҴֳ֤����ν����
 * 4�Х���ñ������ư��������������@p data �˽񤫤�롣
 * ���� @p data �� 2 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_STAND_LATLON <DD>
 * ����ե�����˽񤫤줿ɸ����١�ɸ����١���2ɸ����١���2ɸ����٤�
 * ���ν����4�Х���ñ������ư�������������� @p data �˽񤫤�롣
 * ���� @p data �� 4 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_SPARE_LATLON <DD>
 * ����ե�����˽񤫤줿����1������1������2������2�����ν����
 * 4�Х���ñ������ư�������������� @p data �˽񤫤�롣
 * ���� @p data �� 4 ����¸�ߤ��ʤ��ƤϤʤ�ʤ���
 * (�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_INDX_SIZE <DD>
 * ����ե����뤫�黻�Ф����INDX �θĿ��� 4�Х������������ѿ� @p data 
 * �˽񤫤�롣(�����䤤��碌��NuSDaS1.3���ɲ�)
 * <DT> N_ELEMENT_MAP <DD>
 * ����ե�����ǥǡ����γ�Ǽ�����Ƥ���Ƥ��뤫�ݤ���1 or 0 �ˤ�äơ�
 * 1�Х��������������� @p data �˽񤫤�롣
 * ���� @p data �� @p N_INDX_SIZE ����¸��
 * ���ʤ��ƤϤʤ�ʤ���@p data�ϥ��С���validtime, �̡����Ǥ򥤥�ǥå�
 * ���ˤ�������ǡ����줾��ν���� @p N_MEMBER_LIST, 
 * @p N_VALIDTIME_LIST, @p N_PLANE_LIST, @p N_ELEMENT_LIST���䤤��碌
 * ��̤Ȱ��פ��롣
 * <DT>N_SUBC_NUM<DD>
 * ����ե�����˽񤫤줿 SUBC ��Ͽ�θĿ���4�Х����������ѿ� @p buf �˽񤫤�롣
 * <DT>N_SUBC_LIST<DD>
 * ����ե�����˽񤫤줿 SUBC ��Ͽ�η�̾������ @p buf �˽񤫤�롣
 * ���� @p buf ��Ĺ�� 4 ʸ����ʸ������
 * @p N_SUBC_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * <DT>N_INFO_NUM<DD>
 * ����ե�����˽񤫤줿 INFO ��Ͽ�θĿ���4�Х����������ѿ� @p buf �˽񤫤�롣
 * <DT>N_INFO_LIST<DD>
 * ����ե�����˽񤫤줿 INFO ��Ͽ�η�̾������ @p buf �˽񤫤�롣
 * ���� @p buf ��Ĺ�� 4 ʸ����ʸ������
 * @p N_INFO_NUM ����¸�ߤ��ʤ���Фʤ�ʤ���
 * </DL>
 * @retval �� ��Ǽ���ǿ�
 * @retval -1 ��Ǽ������­
 * @retval -2 ��Ǽ���󤬳��ݤ���Ƥ��ʤ�
 * @retval -3 �䤤��碌������
 * <H3> ���� </H3>
 * ���δؿ��� NuSDaS1.0 ����������Ƥ�������NuSDaS1.3 �Ǥ����Ĥ����䤤��碌
 * ��ǽ���ɲä���Ƥ��롣
 */
	N_SI4
NuSDaS_inq_def(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 param, /**< ��礻���ܥ����� */
		void	*data, /**< INTENT(OUT) ��̳�Ǽ���� */
		const N_SI4 *datasize) /**< ��̳�Ǽ��������ǿ� */
{
	N_SI4 r;
	nusdset_t *ds;
	nustype_t nustype;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		NUSPROF_MARK(NP_USER);
		r = nus_err((NUSERR_DsetNotFound, "dataset %ys not found",
					&nustype));
		NUSDAS_CLEANUP;
		return r;
	}
	r = ds_inq_def(ds, param, data, *datasize);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}
