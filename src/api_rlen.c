/** @file
 * @brief RLE ���̴�Ϣ�Υ����ӥ����֥롼����
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
#include "dfile.h"
#include "ndf_codec.h"

/** @brief 1�Х��������� RLE ���̤���
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.0 ����¸�ߤ��뤬���ɥ�����Ȥ���Ƥ��ʤ��ä���
 * NuSDaS 1.3 ���� Fortran API ��ȼ��
 * �����ӥ����֥롼����Ȥ��ƺ�Ͽ���줿��
 */
	N_SI4
NuSDaS_encode_rlen_8bit_I1(const unsigned char udata[], /**< ���ǡ������� */
		unsigned char compressed_data[], /**< INTENT(OUT) ��̳�Ǽ���� */
		N_SI4 udata_nelems, /**< ���ǡ��������ǿ� */
		N_SI4 max_compress_nbytes, /**< �������ΥХ��ȿ� */
		N_SI4 *maxvalue) /**< INTENT(OUT) ���ǡ����κ����� */
{
	return nus_compress_rlen_i1(udata, udata_nelems, maxvalue,
			compressed_data, max_compress_nbytes);
}

/** @brief 4�Х��������� RLE ���̤���
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.0 ����¸�ߤ��뤬���ɥ�����Ȥ���Ƥ��ʤ��ä���
 * NuSDaS 1.3 ���� Fortran API ��ȼ��
 * �����ӥ����֥롼����Ȥ��ƺ�Ͽ���줿��
 */
	N_SI4
NuSDaS_encode_rlen_8bit(const N_SI4 udata[], /**< ���ǡ������� */
		unsigned char compressed_data[], /**< INTENT(OUT) ��̳�Ǽ���� */
		N_SI4 udata_nelems, /**< ���ǡ��������ǿ� */
		N_SI4 max_compress_nbytes, /**< �������ΥХ��ȿ� */
		N_SI4 *maxvalue) /**< INTENT(OUT) ���ǡ����κ����� */
{
	return nus_compress_rlen_i4(udata, udata_nelems, maxvalue,
			compressed_data, max_compress_nbytes);
}
