/** @file
 * @brief nusdas_make_mask() �μ���
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_kwd.h"
#include <stddef.h>
#include "sys_err.h"
#include "glb.h"

/** @brief �ޥ����ӥå�����κ���
 *
 * ���� @p udata �����Ƥ�����å����ƥޥ����ӥå���������
 * @p maskbit �˽񤭹��ࡣ
 * ���� @p utype �ȷ�»�ͤ�����η��˱����Ƽ��Τ褦�˻��ꤹ�롣
 * <DL>
 * <DT>1�Х���������<DD>
 * ���� @p utype �� N_I1 ����ꤹ�롣
 * ������η�»�������������Ǥ� N_MV_UI1 �����ꤷ�Ƥ�����
 * <DT>2�Х���������<DD>
 * ���� @p utype �� N_I2 ����ꤹ�롣
 * ������η�»�������������Ǥ� N_MV_SI2 �����ꤷ�Ƥ�����
 * <DT>4�Х���������<DD>
 * ���� @p utype �� N_I4 ����ꤹ�롣
 * ������η�»�������������Ǥ� N_MV_SI4 �����ꤷ�Ƥ�����
 * <DT>4�Х��ȼ¿���<DD>
 * ���� @p utype �� N_R4 ����ꤹ�롣
 * ������η�»�������������Ǥ� N_MV_R4 �����ꤷ�Ƥ�����
 * <DT>8�Х��ȼ¿���<DD>
 * ���� @p utype �� N_R8 ����ꤹ�롣
 * ������η�»�������������Ǥ� N_MV_R8 �����ꤷ�Ƥ�����
 * </DL>
 *
 * @retval 0 ���ｪλ
 * @retval -1 ����Ĺ @p mb_bytes ����­���Ƥ���
 * @retval -5 ̤�Τη�̾ @p utype ��Ϳ����줿
 *
 * <H3>�������׷�</H3>
 * @p mb_bytes �Ͼ��ʤ��Ȥ� (@p usize + 7) / 8 �Х��Ȱʾ�ɬ�פǤ��롣
 *
 * <H3>����</H3>
 * nusdas_make_mask() �� NuSDaS 1.0 ����¸�ߤ��롣
 *
 */
	N_SI4
NuSDaS_make_mask(const void *udata, /**< �ʻҥǡ��� */
		const char utype[2], /**< �ʻҥǡ����η� */
		const N_SI4 *usize, /**< �ʻҥǡ��������ǿ� */
		void *output, /**< INTENT(OUT) �ޥ����ӥå����� */
		const N_SI4 *mb_bytes) /**< �ޥ����ӥå�����ΥХ��ȿ� */
{
	sym4_t	utype_s = mem2sym4(utype);
	const double	*ddata;
	const float	*fdata;
	const N_SI4	*idata;
	const N_SI2	*hdata;
	const N_UI1	*cdata;
	N_UI1 *maskbits = output;
	size_t	i, round;
	N_SI4 r;

	NUSDAS_INIT;
	if (*mb_bytes < (*usize - 1) / 8 + 1) {
		r = nus_err((NUSERR_MM_SmallBuf,
				"mask bit array (%Pd bytes given) "
				"is too short (%Pd required)",
				*mb_bytes,
				(*usize - 1) / 8 + 1));
		NUSDAS_CLEANUP;
		return r;
	}
	round = *usize / 8;
	switch (utype_s) {
	case SYM4_R4:
#define EXPAND(xdata, miss) \
		xdata = udata; \
		for (i = 0; i < round; i++) { \
			maskbits[i] = (\
			  ((xdata[i * 8    ] != miss) << 7) \
			| ((xdata[i * 8 + 1] != miss) << 6) \
			| ((xdata[i * 8 + 2] != miss) << 5) \
			| ((xdata[i * 8 + 3] != miss) << 4) \
			| ((xdata[i * 8 + 4] != miss) << 3) \
			| ((xdata[i * 8 + 5] != miss) << 2) \
			| ((xdata[i * 8 + 6] != miss) << 1) \
			|  (xdata[i * 8 + 7] != miss)); \
		} \
		if ((*usize % 8) != 0) { \
			unsigned char c; \
			for (c = 0, i = round * 8; i < (size_t)*usize; i++) { \
				c |= ((xdata[i] != miss) << (7 - (i % 8))); \
			} \
			maskbits[round] = c; \
		}
		EXPAND(fdata, N_MV_R4)
		break;
	case SYM4_R8:
		EXPAND(ddata, N_MV_R8)
		break;
	case SYM4_I4:
		EXPAND(idata, N_MV_SI4)
		break;
	case SYM4_I2:
		EXPAND(hdata, N_MV_SI2)
		break;
	case SYM4_I1:
		EXPAND(cdata, N_MV_UI1)
		break;
	default:
		r = nus_err((NUSERR_MM_BadType,
				"unrecognized user data type <%.2s>", utype));
		NUSDAS_CLEANUP;
		return r;
	}
	GlobalConfig(saved_mask) = maskbits;
	GlobalConfig(saved_mask_size) = *mb_bytes;
	NUSDAS_CLEANUP;
	return 0;
}
