#include <nusdas.h>
#include <string.h>
#include <stdio.h>

#ifdef TESTHDR
#include "libsrf.h"
#endif

/** @brief AQC�̃p�b�N��W�J
 *
 * �A���_�X �f�R�[�h �f�[�^�Z�b�g�Ɋ܂܂�� AQC ����
 * �v�f�� @p param �Ŏw�肳���e�r�b�g�t�B�[���h�����o���B
 * <DL>
 * <DT>UNYOU��<DD>���d�E�x�~�E�^�p��� (-1:�x�~, 0:���d����, ��:���d��)
 * <DT>RRfr0��<DD>�~���ʂ̏��
 * (0:���d����, 1:�n�[�h�G���[�E�����E�x�~, 2:AQC�Y���l, 3:����l; �ȉ�����)
 * <DT>SSfr0��<DD>���Ǝ��Ԃ̏��
 * <DT>T����������<DD>�C���̏��
 * <DT>WindD��<DD>�����̏��
 * <DT>WindS��<DD>�����̏��
 * <DT>SnowD��<DD>�ϐ�[�̏��
 * </DL>
 * 
 * <H3>����</H3>
 * �v�f�����s���ȏꍇ�͌x����Ȃɂ������I������B
 * (NuSDaS 1.3 ���O�͕s�蓮��)
 * <H3>����</H3>
 * ���̊֐��� NAPS7 ���ォ�瑶�݂����B
 */

void
srf_amd_aqc(const N_UI2 aqc_in[], /**< AQC �z�� */
		int num, /**< �z��v�f�� */
		N_SI2 aqc_out[], /**< ���ʔz�� */
		const char *param /**< �v�f�� */)
{
    int i, shift_bit;

    if (strncmp(param, "UNYOU ", 6) == 0) {
	for (i = 0; i < num; i++) {
	    if (aqc_in[i] & 0x0008) {
	        aqc_out[i] = -1;
	    } else {
	        aqc_out[i] = (aqc_in[i] & 0x0007);
	    }
	}
	return;
    } else if (strncmp(param, "RRfr0 ", 6) == 0) {
	shift_bit = 14;
    } else if (strncmp(param, "SSfr0 ", 6) == 0) {
	shift_bit = 12;
    } else if (strncmp(param, "T     ", 6) == 0) {
	shift_bit = 10;
    } else if (strncmp(param, "WindD ", 6) == 0) {
	shift_bit = 8;
    } else if (strncmp(param, "WindS ", 6) == 0) {
	shift_bit = 6;
    } else if (strncmp(param, "SnowD ", 6) == 0) {
	shift_bit = 4;
    } else {
	fprintf(stderr, "srf_amd_aqc: invalid element name %.6s\n", param);
	return;
    }
    for (i = 0; i < num; i++) {
        aqc_out[i] = (short) ((aqc_in[i] >> shift_bit) & 0x0003);
    }
}
