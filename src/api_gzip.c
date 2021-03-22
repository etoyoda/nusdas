/** @file
 * @brief gzip ���̡�Ÿ����Ԥ������ӥ����֥롼����
 */
#include "config.h"
#include "nusdas.h"
#include "nus_gzlib.h"
#include <stddef.h>
#include "sys_err.h"

/** @brief gzip ����
 *
 * ���ϥǡ��� @p in_data �� gzip ���̤��� @p out_buf �˳�Ǽ���롣
 * @retval -98 NuSDaS �� ZLib ��Ȥ��褦�����ꤵ��Ƥ��ʤ���
 * @retval -9 ZLib �� deflateEnd �ؿ������顼�򵯤�������
 * @retval -4 ����ΰ��Ĺ�� @p out_nbytes ����­���Ƥ��롣
 * @retval -2 ZLib �� deflateInit2 �ؿ������顼�򵯤�������
 * @retval -1 ZLib �� deflate �ؿ������顼�򵯤�������
 * @retval ¾ ���̥ǡ�����Ĺ��
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.3 �ǿ��ߤ��줿��
 */
	N_SI4
NuSDaS_gzip(const void *in_data UNUSED, /**< ���ϥǡ��� */
		N_UI4 in_nbytes UNUSED, /**< ���ϥǡ����ΥХ��ȿ� */
		void *out_buf UNUSED, /**< INTENT(OUT) ���̷�̤��Ǽ�����ΰ� */
		N_UI4 out_nbytes UNUSED /**< ����ΰ�ΥХ��ȿ� */)
{
#ifdef USE_ZLIB
	return nusgz_compress((void *)in_data, in_nbytes, out_buf, out_nbytes);
#else
	return NUSERR_NoZLib;
#endif
}

/** @brief gzip ���̥ǡ�����Ÿ�����Ĺ��������
 *
 * ���ϥǡ��� @p in_data �� gzip Ÿ������Ȥ���ɬ�פȤʤ�
 * ��̳�Ǽ�ΰ�ΥХ��ȿ����֤���
 * @retval -98 NuSDaS �� ZLib ��Ȥ��褦�����ꤵ��Ƥ��ʤ���
 * @retval �� Ÿ�����Ĺ��
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.3 �ǿ��ߤ��줿��
 */
	N_SI4
NuSDaS_gunzip_nbytes(const void *in_data UNUSED, /**< ���̥ǡ��� */
		N_UI4 in_nbytes UNUSED /**< ���̥ǡ����ΥХ��ȿ� */)
{
#ifdef USE_ZLIB
	return nusgz_inq_decompressed_size((void *)in_data, in_nbytes);
#else
	return NUSERR_NoZLib;
#endif
}

/** @brief gzip ���̥ǡ�����Ÿ��
 *
 * ���ϥǡ��� @p in_data �� gzip Ÿ������ @p out_buf �˳�Ǽ���롣
 * @retval -98 NuSDaS �� ZLib ��Ȥ��褦�����ꤵ��Ƥ��ʤ���
 * @retval -99 ���Ϥ� gzip ���̷����ǤϤʤ���
 * @retval -5 Ÿ����̤�Ĺ�������̥ǡ����������硣
 * @retval -4 ����ΰ��Ĺ�� @p out_nbytes ����­���Ƥ��롣
 * @retval -3 Ÿ����̤� CRC32 �����̥ǡ����������硣
 * @retval -2 ZLib �� inflateInit �ؿ������顼�򵯤�������
 * @retval -1 ZLib �� inflate �ؿ������顼�򵯤�������
 * @retval ¾ Ÿ���ǡ����ΥХ��ȿ�
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.3 �ǿ��ߤ��줿��
 */

	N_SI4
NuSDaS_gunzip(const void *in_data UNUSED, /**< ���̥ǡ��� */
		N_UI4 in_nbytes UNUSED, /**< ���̥ǡ����ΥХ��ȿ� */
		void *out_buf UNUSED, /**< INTENT(OUT) Ÿ����̤��Ǽ�����ΰ� */
		N_UI4 out_nbytes UNUSED /**< ����ΰ�ΥХ��ȿ� */
	     )
{
#ifdef USE_ZLIB
	return nusgz_decompress((void *)in_data, in_nbytes, out_buf, out_nbytes);
#else
	return NUSERR_NoZLib;
#endif
}
