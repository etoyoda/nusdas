/** @file
 * @brief nusdas_parameter_reset() �μ���
 *
 * @note SIZEX, SIZEY �η��� SI4 �Ǥ��뤫�ɤ������䡣
 */
#include "config.h"
#include "nusdas.h"

#include <stddef.h>
#include "internal_types.h"
#include "sys_err.h"
#include "glb.h"


/** @brief ���ץ���������ͤ��᤹
 *
 * nusdas_parameter_change()
 * �����ꤵ�줿�ѥ�᥿������ͤ��ᤷ�ޤ���
 *
 * <H3>����</H3>
 * ���δؿ� �� NuSDaS 1.3 ��Ƴ������ޤ�����
 * ��������ΥС������Ǥ� nusdas_parameter_change() �˴�����
 * �ޤ������ NULL ��Ϳ������ˡ���Ȥ��Ƥ��ޤ�����
 * */
	N_SI4
NuSDaS_parameter_reset(N_SI4 param /**< ������ܥ����� */)
{
	NUSDAS_INIT;
	switch (param) {
		case N_PC_MISSING_UI1:
			GlobalConfig(pc_missing_ui1) = UCHAR_MAX;
			break;
		case N_PC_MISSING_SI2:
			GlobalConfig(pc_missing_si2) = -32768;
			break;
		case N_PC_MISSING_SI4:
#if SIZEOF_INT == 4
			GlobalConfig(pc_missing_si4) = INT_MIN;
#elif SIZEOF_LONG == 4
			GlobalConfig(pc_missing_si4) = LONG_MIN;
#else
			GlobalConfig(pc_missing_si4) = 0x80000000L;
#endif
			break;
		case N_PC_MISSING_R4:
			GlobalConfig(pc_missing_r4) = FLT_MAX;
			break;
		case N_PC_MISSING_R8:
			GlobalConfig(pc_missing_r8) = DBL_MAX;
			break;
		case N_PC_MASK_BIT:
			GlobalDSConfig(pc_mask_bit) = NULL;
			break;
		case N_PC_PACKING:
			GlobalDSConfig(pc_packing) = 0;
			break;
		case N_PC_SIZEX:
			GlobalDSConfig(pc_sizex) = 0;
			break;
		case N_PC_SIZEY:
			GlobalDSConfig(pc_sizey) = 0;
			break;
		case N_PC_ID_SET:
			GlobalConfig(nrd_override) = NRD_UNFIX;
			break;
		case N_PC_WBUFFER:
			GlobalDSConfig(pc_wbuffer) = 0;
			break;
		case N_PC_RBUFFER:
			GlobalDSConfig(pc_rbuffer) = 17;
			break;
		default:
			NUSDAS_CLEANUP;
			return -1;
	}
	NUSDAS_CLEANUP;
	return 0;
}
