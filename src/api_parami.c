/** @file
 * @brief nusdas_parameter_change() �μ���
 *
 * @note SIZEX, SIZEY �η��� SI4 �Ǥ��뤫�ɤ������䡣
 */
#include "config.h"
#include "nusdas.h"

#include <string.h>
#include <stddef.h>
#include "internal_types.h"
#include "sys_err.h"
#include "glb.h"

/** @brief ���ץ�������
 *
 * nusdas_parameter_change() �ι��� @p param �����ꤵ���
 * �ѥ�᡼�����ͤ� @p value �λؤ��ΰ� (���ϰʲ��򻲾�) �˽񤭹��ࡣ
 * <DL>
 * <DT>N_PC_MISSING_UI1<DD>1�Х��������η�»��
 * <DT>N_PC_MISSING_SI2<DD>2�Х��������η�»��
 * <DT>N_PC_MISSING_SI4<DD>4�Х��������η�»��
 * <DT>N_PC_MISSING_R4<DD>4�Х��ȼ¿��η�»��
 * <DT>N_PC_MISSING_R8<DD>8�Х��ȼ¿��η�»��
 * <DT>N_PC_SIZEX<DD>4�Х��������� x ���������ʻҥ�������Ϳ����
 * <DT>N_PC_SIZEY<DD>4�Х��������� y ���������ʻҥ�������Ϳ����
 * <DT>N_PC_MASK_BIT<DD>
 * �ӥåȥޥ���������֤���
 * ������礻�������ͤ� nusdas_make_mask() �Ǻ��줿���ˤ�����ǽ���ʤ���
 * <DT>N_PC_PACKING<DD>
 *  4�Х��Ȥ�ʸ����˶����ѥå�����̾��Ϳ���롣
 *  ���ꤵ��Ƥ��ʤ����� 4 ʸ���Υ��ڡ������񤭹��ޤ�롣
 * <DT>N_PC_ID_SET<DD>
 *  NRD �ֹ����󤬤����äƤ����礽���͡������äƤ��ʤ���� -1 ��Ϳ���롣
 * <DT>N_PC_WBUFFER<DD>
 *  4�Х��������˽񤭹��ߥХåե������� (�¹Ի����ץ���� FWBF) ��Ϳ���롣
 * <DT>N_PC_RBUFFER<DD>
 *  4�Х����������ɤ߼��Хåե������� (�¹Ի����ץ���� FRBF) ��Ϳ���롣
 * </DL>
 *
 * @retval 0 ���ｪλ
 * @retval -1 ���ݡ��Ȥ���Ƥ��ʤ��ѥ�᥿�Ǥ���
 * @retval -2 �ӥåȥޥ�����������ꤵ��Ƥ��ʤ�
 * @retval -3 �ӥåȥޥ�����������ꤵ��Ƥ��뤬Ĺ�����狼��ʤ�
 *
 * <H3>����</H3>
 * NuSDaS 1.3 ��Ƴ�����줿��
 * */
	N_SI4
NuSDaS_inq_parameter(N_SI4 param, /**< ������ܥ����� */
		void *value) /**< INTENT(OUT) ������ */
{
	NUSDAS_INIT;
	switch (param) {
		case N_PC_MISSING_UI1:
			*(N_UI1 *)value = GlobalConfig(pc_missing_ui1);
			break;
		case N_PC_MISSING_SI2:
			*(N_SI2 *)value = GlobalConfig(pc_missing_si2);
			break;
		case N_PC_MISSING_SI4:
			*(N_SI4 *)value = GlobalConfig(pc_missing_si4);
			break;
		case N_PC_MISSING_R4:
			*(float *)value = GlobalConfig(pc_missing_r4);
			break;
		case N_PC_MISSING_R8:
			*(double *)value = GlobalConfig(pc_missing_r8);
			break;
		case N_PC_MASK_BIT:
			if (GlobalDSConfig(pc_mask_bit) == NULL) {
				NUSDAS_CLEANUP;
				return -2;
			} else if (GlobalDSConfig(pc_mask_bit)
					== GlobalConfig(saved_mask)) {
				memcpy(value, GlobalConfig(saved_mask),
					GlobalConfig(saved_mask_size));
			} else {
				NUSDAS_CLEANUP;
				return -3;
			}
			break;
		case N_PC_PACKING:
			*(N_SI4 *)value = GlobalDSConfig(pc_packing);
			break;
		case N_PC_SIZEX:
			*(N_SI4 *)value = GlobalDSConfig(pc_sizex);
			break;
		case N_PC_SIZEY:
			*(N_SI4 *)value = GlobalDSConfig(pc_sizey);
			break;
		case N_PC_ID_SET:
			*(N_SI4 *)value = GlobalConfig(nrd_override);
			break;
		case N_PC_WBUFFER:
			*(N_SI4 *)value = GlobalDSConfig(pc_wbuffer);
			break;
		case N_PC_RBUFFER:
			*(N_SI4 *)value = GlobalDSConfig(pc_rbuffer);
			break;
		default:
			NUSDAS_CLEANUP;
			return -1;
	}
	NUSDAS_CLEANUP;
	return 0;
}
