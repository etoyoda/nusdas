/** @file
 * @brief nusdas_parameter_change() �μ���
 *
 * @note SIZEX, SIZEY �η��� SI4 �Ǥ��뤫�ɤ������䡣
 */
#include "config.h"
#include "nusdas.h"

#include <stddef.h>
#include "internal_types.h"
#include "sys_err.h"
#include "glb.h"

/** @brief ��إå��Ȥκ����ɻ��ѥ��ߡ�
 *
 * NuSDaS 1.2 �ޤǤ� nusdas_fort.h �ˤ�����
 *
 *   INTEGER:: NULL
 *   COMMON /NUSDAS/ NULL
 *
 * �Τ褦��������Ƥ��ꡢ������ nusdas_parameter_change() �μ����Ǥ�
 * �ܿ����˵��䤬���� hack �ˤ�ä���������Ȥ��Υ����֥�å��Υݥ���
 * ����Ӥ��Ƶ��̥��Ƚ�ꤷ�Ƥ�����
 *
 * NAPS8 �ˤ����� NuSDaS 1.1 �� NuSDaS 1.3 ��ʻ¸���뤿�ᡢ��إå��ե�����
 * ���Ѥ��ƥ���ѥ��뤵�줿���֥������ȥ⥸�塼�뤬���饤�֥�����
 * ���ʤ��褦�˰ʲ��δؿ����������Ƥ��롣
 * 
 * ��إå��ե�����ˤ�륪�֥������ȥ⥸�塼��ˤϥǡ�������������
 * NUSDAS (Fortran ����ѥ���ˤ�äƤϾ�ʸ���ʤ�) �Ȥ�������ܥ뤬
 * �Ǥ��롣����ѥ���ˤ�äƤϥǡ������������Ȥϰ㤦���⤷��ʤ���
 * ������ʤ�Ǥ�ƥ����ȥ��������Ȥ������ȤϤʤ�������
 * ���������Υ����������ɤϥƥ����ȥ��������� NUSDAS ���Υ���ܥ��
 * ������롣���ʤ��Ȥ���Ω��Ŭ�� Fortran �ǤϤ���ˤ�äƥ�󥫤�
 * ���Τ褦�ʥ��顼�򵯤������Ȥˤ�������Ƚ�ꤹ�뤳�Ȥ��Ǥ��롣
 * ¾�Υ���ѥ���ǤϷٹ�ɤޤ�Τ��Ȥ�¿�� (���ʤ��Ȥ� Linux ifort/g77)
 * �������뤤�ϲ��ⵯ����ʤ����⤷��ʤ� (���줬���ε�ǽ���ѻߤ��줿��ͳ)
 * �����Ȥ⤢�� NAPS8 �ΰ��걿�Ѥ��Ż뤵���Τ�����ʤ����ޥ�������

ld: 0711-224 WARNING: Duplicate symbol: NUSDAS
ld: 0711-345 Use the -bloadmap or -bnoquiet option to obtain more information.

 */

#if 1
int NUSDAS(void) { return -1; }

/** @brief �ؿ� NUSDAS() ��Ʊ�� */
int NUSDAS_(void) { return -1; }

/** @brief �ؿ� NUSDAS() ��Ʊ�� */
int nusdas(void) { return -1; }

/** @brief �ؿ� NUSDAS() ��Ʊ�� */
int nusdas_(void) { return -1; }
#else
/* ����嵭�ΰ��ϰ����Թ礬�����ʤä���ʲ����Ѥ��衣
 * ����˰ܿ���������¤ꡢFortran API �� NULL ��������ʤ���������
 * ����Ω���ʤ��ΤǤϤ��뤬��
 * */
int NUSDAS, NUSDAS_, nusdas, nusdas_;
#endif

/** @brief ���ץ��������
 *
 * @p param �ǻ��ꤵ���ѥ�᡼������ @p value �����ꤹ�롣
 * �����ͤι��ܤˤĤ��Ƥϡ�
 * �ߴ����Τ����ͥ���Τ�����̾�� N_OFF ���Ѥ��뤳�Ȥ��Ǥ��롣
 * <DL>
 * <DT>N_PC_MISSING_UI1<DD>1�Х��������η�»�� (������: N_MV_UI1)
 * <DT>N_PC_MISSING_SI2<DD>2�Х��������η�»�� (������: N_MV_SI2)
 * <DT>N_PC_MISSING_SI4<DD>4�Х��������η�»�� (������: N_MV_SI4)
 * <DT>N_PC_MISSING_R4<DD>4�Х��ȼ¿��η�»�� (������: N_MV_R4)
 * <DT>N_PC_MISSING_R8<DD>8�Х��ȼ¿��η�»�� (������: N_MV_R8)
 * <DT>N_PC_MASK_BIT<DD>�ӥåȥޥ�������ؤΥݥ���
 * (�����ͤ� NULL �ݥ��󥿤��� Fortran �Ǥ�ľ������Ǥ��ʤ��Τ�
 * nusdas_parameter_reset() ���Ѥ���줿��)
 * <DT>N_PC_SIZEX<DD>
 *  �����ͤ����ꤹ��ȶ���Ū�˥ǡ����쥳���ɤ� @p x ����
 *  �ʻҿ������ꤹ�� (0)
 * <DT>N_PC_SIZEY<DD>
 *  �����ʻҥ�����:
 *  ������ (0) �ʳ������ꤹ��ȥǡ����쥳���ɤ� @p y ����
 *  �ʻҿ������ꤹ��
 * <DT>N_PC_PACKING<DD>
 *  4ʸ���Υѥå���̾�����ꤹ��ȡ�����ե�����λ���ˤ�����餺
 *  nusdas_write() ���ǡ�����Ͽ�񤭹��ߤκݤ��Ѥ�����
 *  �ѥå����������ѹ�����롣
 *  �����ͤ��᤹ (����ե�����ɤ���˽񤫤���) �ˤ�
 *  4 �Х��������� 0 �����ꤹ�롣
 * <DT>N_PC_ID_SET<DD>
 *  NRD �ֹ�����:
 *  ������ (-1) �ʳ������ꤹ��ȡ������ֹ�� NRD ������
 *  �����Ϥ��Ѥ���褦�ˤʤ�
 * <DT>N_PC_WBUFFER<DD>
 *  �񤭹��ߥХåե������� (������: 0)
 *  �¹Ի����ץ���� FWBF ��Ʊ����
 * <DT>N_PC_RBUFFER<DD>
 *  �ɤ߼��Хåե������� (������: 17)
 *  �¹Ի����ץ���� FRBF ��Ʊ����
 * <DT>N_PC_KEEP_CFILE<DD>
 *  �ե�������Ĥ������� CNTL/INDX �ʤɤΥإå�����򥭥�å��夷�Ƥ���
 *  ������ˤ���ȥ���å���������ʤ��ʤ�ʴ�����: -1�ˡ�
 *  �¹Ի����ץ���� GKCF ��Ʊ����
 * <DT>N_PC_OPTIONS<DD>
 *  ����Τߤǥꥻ�åȤϤǤ��ʤ���
 *  �̥뽪ü����ʸ�����Ϳ����ȼ¹Ի����ץ����Ȥ������ꤹ�롣
 *  Fortran ���󥿡��ե������Ǥ�̥뽪ü���ʤ���Фʤ�ʤ����Ȥ���ա�
 * </DL>
 *
 * @retval 0 ���ｪλ
 * @retval -1 ���ݡ��Ȥ���Ƥ��ʤ��ѥ�᥿�Ǥ���
 *
 * <H3>����</H3>
 * NuSDaS 1.0 ����¸�ߤ��롣
 *
 * NuSDaS 1.1 �Ǥϥǡ������å�õ���Υ���å������������꤬���ꡢ
 * N_PC_ID_SET �� NRD �ֹ�����򤫤��������Ϥ�Ԥä����
 * NRD �ֹ������������Ʊ�����̤˥����������Ƥ�õ�����Ԥ��ʤ�
 * (���餫���� NRD ����򤫤��������������򤷤Ƥ����õ�������)��
 * ��������� NuSDaS 1.3 �Ǥϲ�褵��Ƥ��롣
 * */
	N_SI4
NuSDaS_parameter_change(N_SI4 param, /**< ������ܥ����� */
		const void *value) /**< ������ */
{
	NUSDAS_INIT;
	if (value == NULL) {
		return NuSDaS_parameter_reset(param);
	}
	/* NuSDaS 1.1 Fortran �ѥإå��� NULL ��Ȥä�
	 * ���褿�ݥ��󥿤�����᤹�롣AIX �Ǥϥ�󥯥��顼����
	 * Linux ���̤äƤ��ޤ��Τǡ���Ϥꥵ�ݡ��Ȥ���Τ��ȤȻפ�
	 * �ʤ���������ǽ���ʤ����⤷��ʤ����������ޤǤ��ݾڤ��ʤ���
	 */
	if (value == (const void *)NUSDAS
			|| value == (const void *)NUSDAS_
			|| value == (const void *)nusdas
			|| value == (const void *)nusdas_) {
		nus_warn(("deprecated: use of NULL in nusdas_fort.h"));
		return NuSDaS_parameter_reset(param);
	}
	switch (param) {
		case N_PC_MISSING_UI1:
			GlobalConfig(pc_missing_ui1) = *(const N_UI1 *)value;
			break;
		case N_PC_MISSING_SI2:
			GlobalConfig(pc_missing_si2) = *(const N_SI2 *)value;
			break;
		case N_PC_MISSING_SI4:
			GlobalConfig(pc_missing_si4) = *(const N_SI4 *)value;
			break;
		case N_PC_MISSING_R4:
			GlobalConfig(pc_missing_r4) = *(const float *)value;
			break;
		case N_PC_MISSING_R8:
			GlobalConfig(pc_missing_r8) = *(const double *)value;
			break;
		case N_PC_MASK_BIT:
			GlobalDSConfig(pc_mask_bit) = value;
			break;
		case N_PC_PACKING:
			GlobalDSConfig(pc_packing) = *(const N_SI4 *)value;
			break;
		case N_PC_SIZEX:
			GlobalDSConfig(pc_sizex) = *(const N_SI4 *)value;
			break;
		case N_PC_SIZEY:
			GlobalDSConfig(pc_sizey) = *(const N_SI4 *)value;
			break;
		case N_PC_ID_SET:
			GlobalConfig(nrd_override) = *(const N_SI4 *)value;
			break;
		case N_PC_WBUFFER:
			GlobalDSConfig(pc_wbuffer) = *(const N_SI4 *)value;
			break;
		case N_PC_RBUFFER:
			GlobalDSConfig(pc_rbuffer) = *(const N_SI4 *)value;
			break;
		case N_PC_KEEP_CFILE:
			GlobalConfig(pc_keep_closed_file)
				= *(const N_SI4 *)value;
			break;
		case N_PC_OPTIONS:
			if (value) {
				nusdas_opts(NULL, value);
			}
			break;
		default:
			NUSDAS_CLEANUP;
			return -1;
	}
	NUSDAS_CLEANUP;
	return 0;
}
