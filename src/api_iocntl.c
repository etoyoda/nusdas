/** @file
 * @brief nusdas_iocntl() �μ���
 */
#include "config.h"
#include "nusdas.h"

#include <stddef.h>
#include "internal_types.h"
#include "sys_time.h"
#include "glb.h"
#include "sys_err.h"

/** @brief �����ϥե饰����
 *
 * �����Ϥˤ������ե饰�����ꤹ��.
 * <DL>
 * <DT>N_IO_MARK_END<DD>������1.
 * ��ˤ���� nusdas_write() �ʤɤν��ϴؿ���ƤӽФ����Ӥ�
 * �ǡ����ե�����ؤν��Ϥ򴰷뤵�� END ��Ͽ��񤭽Ф��Τ����.
 * <DT>N_IO_W_FCLOSE<DD>������1.
 * ��ˤ���� nusdas_write() �ʤɤν��ϴؿ���ƤӽФ����Ӥ�
 * �񤭹����Ѥ˳������ե�������Ĥ���Τ����.
 * ®�پ�ͭ���������ǡ����ե����������λ�������
 * �ե�������Ĥ���ؿ� nusdas_allfile_close() �ޤ���
 * nusdas_onefile_close() ��Ŭ�ڤ˸Ƥ�ǥե�������Ĥ��ʤ���
 * ���ϥե����뤬�Դ����Ȥʤꡢ����ɤळ�Ȥ��Ǥ��ʤ�.
 * �ʤ������Υե饰���ѹ������ N_IO_MARK_END ��Ϣư����.
 * <DT>N_IO_R_FCLOSE<DD>������1.
 * ��ˤ���� nusdas_read() �ʤɤ����ϴؿ���ƤӽФ����Ӥ�
 * �ɤ߹����Ѥ˳������ե�������Ĥ���Τ����.
 * ®�پ�ͭ��������¿���Υե����뤫�����Ϥ���ץ����Ǥ�
 * �ե�����ϥ�ɥ뤬�ϳ餹���ǰ������Τ�
 * �ե����������Ū���Ĥ��뤳�Ȥ��侩�����.
 * <DT>N_IO_WARNING_OUT<DD>������1.
 * 0 �ˤ���ȥ��顼��å��������������Ϥ����.
 * 1 �ˤ���ȡ�����˲ä��Ʒٹ��å���������Ϥ����褦�ˤʤ�.
 * 2 �ˤ���ȡ�����˲ä��ƥǥХå���å���������Ϥ����褦�ˤʤ�.
 * <DT>N_IO_BADGRID<DD>������0.
 * 1 �ˤ�������ˡ�ѥ�᥿�θ�������Ŭ�ڤ��ͤ����Ф���Ƥ�
 * �ǡ����ե����뤬�����Ǥ���褦�ˤʤ롣
 * </DL>
 *
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.0 ����¸�ߤ���.
 * <B>N_IO_WARNING_OUT</B> ���� 2 �� NuSDaS 1.3 �γ�ĥ�Ǥ���.
 * <B>N_IO_BADGRID</B> �� NuSDaS 1.3 �γ�ĥ�Ǥ���.
 * */
	N_SI4
NuSDaS_iocntl(N_SI4 param, /**< ������ܥ����� */
		N_SI4 value) /**< ������ */
{
	/** @note ���δؿ��ϥ饤�֥��ν�����򤷤ʤ�.
	 * */
	switch (param) {
		case N_IO_MARK_END:
			GlobalConfig(io_mark_end) = (value ? 1 : 0);
			break;
		case N_IO_W_FCLOSE:
			GlobalConfig(io_mark_end) =
			GlobalConfig(io_w_fclose) = (value ? 1 : 0);
			break;
		case N_IO_R_FCLOSE:
			GlobalConfig(io_r_fclose) = (value ? 1 : 0);
			break;
		case N_IO_WARNING_OUT:
			nus_wrn_enabled = !!value;
			nus_dbg_enabled = (value > 1);
			break;
		case N_IO_BADGRID:
			GlobalConfig(io_badgrid) = (value ? 1 : 0);
			break;
		default:
			return -1;
	}
	return 0;
}
