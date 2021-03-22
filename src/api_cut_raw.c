#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stdlib.h>
#include <string.h>
#include "sys_err.h"
#include "sys_endian.h"
#include "sys_kwd.h"
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_err.h"
#include "glb.h"
#include "sys_mem.h"

	static void
ArrayCut(char *dest, char *src, N_UI4 elemsize,
		unsigned nx,
		unsigned sx, unsigned wx, unsigned sy, unsigned wy)
{
	unsigned iy, linewidth;
	char *srcline;
	linewidth = wx * elemsize;
	srcline = src + (nx * sy + sx) * elemsize;
	for (iy = sy; iy < sy + wy; iy++) {
		memcpy(dest, srcline, linewidth);
		srcline += elemsize * nx;
		dest += linewidth;
	}
}

/** @brief �ΰ����� DATA ��Ͽľ���ɼ� 
 * nusdas_cut_raw ���ϰϻ����ǡ�nusdas_cut �򻲾ȡ�
*/
	N_SI4
NuSDaS_cut2_raw(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		const char plane1[6], /**< ��1 */
		const char plane2[6], /**< ��2 */
		const char element[6], /**< ����̾ */
		void *udata, /**< INTENT(OUT) �ǡ�����Ǽ������ */
		const N_SI4 *usize, /**< �ǡ�����Ǽ������ΥХ��ȿ� */
		const N_SI4 *ixstart, /**< $x$ �����ʻ��ֹ沼�� */
		const N_SI4 *ixfinal, /**< $x$ �����ʻ��ֹ��� */
		const N_SI4 *iystart, /**< $y$ �����ʻ��ֹ沼�� */
		const N_SI4 *iyfinal) /**< $y$ �����ʻ��ֹ��� */
{
	N_UI4 nx_total, *buf4, nbytes, skip, elemsize, packprefix;
	N_SI4 r, nx, ny;
	N_SI4 buflen;
	sym4_t packing, missing;
	char *tmpbuf;

	NUSDAS_INIT;
	nus_debug(("--- nusdas_cut_raw"));
#define width(a, b) ((a >= 0 && b >= 0) ? (a - b + 1) : -1)
	nx = width(*ixfinal, *ixstart);
	ny = width(*iyfinal, *iystart);
	if (nx <= 0 || ny <= 0) {
		r = nus_err((NUSERR_RD_BadCutRegion,
					"nusdas_cut: invalid geometry"));
		NUSDAS_CLEANUP;
		return r;
	}

	buflen = 1;
	r = nusdas_inq_data2(type1, type2, type3, basetime, member,
			validtime1, validtime2, plane1, plane2, element, 
			N_DATA_NBYTES, &nbytes, &buflen);
	if (r != 1) {
		NUSDAS_CLEANUP;
		return r;
	}

	tmpbuf = nus_malloc(nbytes); 
	if (tmpbuf == NULL) {
		r = nus_err((NUSERR_MemShort, "nusdas_cut"));
		NUSDAS_CLEANUP;
		return r;
	}
	buflen = nbytes;
	r = nusdas_inq_data2(type1, type2, type3, basetime, member,
			validtime1, validtime2, plane1, plane2, element,
			N_DATA_CONTENT, tmpbuf, &buflen);
	if (r != buflen) {
		goto Hell;
	}

	buf4 = (N_UI4 *)tmpbuf;
	nx_total = NTOH4(buf4[0]);
	packing = buf4[2];
	missing = buf4[3];

	if (packing == SYM4_RLEN || packing == SYM4_2UPJ || 
			packing ==SYM4_2UPP || missing == SYM4_MASK) {
		if (nbytes > (N_UI4)(*usize)) {
			r = nus_err((NUSERR_RD_SmallBuf, 
				     "nusdas_cut_raw %Pd < %Pd",
				     *usize, nbytes));
			goto Hell;
		}
		memcpy(udata, tmpbuf, nbytes);
		r = nbytes;
	} else {

		switch (packing) {
		case SYM4_I1: elemsize = 1; packprefix = 0; break;
		case SYM4_I2:
		case SYM4_N1I2: elemsize = 2; packprefix = 0; break;
		case SYM4_2PAC:
		case SYM4_2UPC: elemsize = 2; packprefix = 8; break;
		case SYM4_R4:
		case SYM4_I4: elemsize = 4; packprefix = 0; break;
		case SYM4_4PAC: elemsize = 8; packprefix = 16; break;
		case SYM4_R8: elemsize = 8; packprefix = 0; break;
		default:
			r = nus_err((NUSERR_RD_BadPackMiss,
				     "nusdas_cut: unknown packing=%Ps", 
				     packing));
			goto Hell;
		}
		switch (missing) {
		case SYM4_UDFV:
			skip = 16 + elemsize + packprefix;
			break;
		case SYM4_NONE:
			skip = 16 + packprefix;
			break;
		default:
			r = nus_err((NUSERR_RD_BadPackMiss,
				     "nusdas_cut: unknown missing=%Ps", 
				     missing));
			goto Hell;
		}

		buflen = skip + elemsize * nx * ny;
		if (*usize < buflen) {
			nus_debug(("%Pd * %Pd * %Pd + %Pd", 
				   nx, ny, elemsize, skip));
			r = nus_err((NUSERR_RD_SmallBuf, 
				     "nusdas_cut_raw %Pd < %Pd",
				     *usize, buflen));
			goto Hell;
		}
		
		((N_UI4 *)udata)[0] = HTON4(nx);
		((N_UI4 *)udata)[1] = HTON4(ny);
		memcpy((char *)udata + 8, tmpbuf + 8, skip - 8);
		ArrayCut((char *)udata + skip, tmpbuf + skip, 
			 elemsize, nx_total,
			 *ixstart - 1, nx, *iystart - 1, ny);
		r = buflen;
	}

Hell:
	NUSDAS_CLEANUP;
	nus_free(tmpbuf);
	return r;
}

/** @brief �ΰ����� DATA ��Ͽľ���ɼ�
 *
 * nusdas_read2_raw() ������������ǡ����쥳���ɤΤ����ʻ���
 * (@p ixstart , @p iystart )--(@p ixfinal , @p iyfinal )
 * ���б�������ʬ������ @p udata �˳�Ǽ����롣
 *
 * @retval �� �ɤ߽Ф��Ƴ�Ǽ�����Х��ȿ�
 * @retval 0 ���ꤷ���ǡ�����̤��Ͽ(����ե������ elementmap �ˤ�äƽ񤭹��ޤ�뤳�Ȥϵ��Ƥ���Ƥ��뤬���ޤ��ǡ������񤭹��ޤ�Ƥ��ʤ�)
 * @retval -2 ���ꤷ���ǡ����ϵ�Ͽ���뤳�Ȥ����Ƥ���Ƥ��ʤ�(elementmap �ˤ�äƶػߤ���Ƥ�����Ȼ��ꤷ����̾������̾����Ͽ����Ƥ��ʤ�����ξ����ޤ�)��
 * @retval -4 ��Ǽ������­
 *
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.1 ��Ƴ�����줿��
 * ���顼������ -4 �� NuSDaS 1.3 �ǿ��ߤ��줿��Τǡ�
 * ��������ϥ��顼�����å����ʤ���Ƥ��ʤ��ä���
 */
	N_SI4
NuSDaS_cut_raw(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		const char plane[6], /**< �� */
		const char element[6], /**< ����̾ */
		void *udata, /**< INTENT(OUT) �ǡ�����Ǽ������ */
		const N_SI4 *usize, /**< �ǡ�����Ǽ������ΥХ��ȿ� */
		const N_SI4 *ixstart, /**< $x$ �����ʻ��ֹ沼�� */
		const N_SI4 *ixfinal, /**< $x$ �����ʻ��ֹ��� */
		const N_SI4 *iystart, /**< $y$ �����ʻ��ֹ沼�� */
		const N_SI4 *iyfinal) /**< $y$ �����ʻ��ֹ��� */
{
	N_SI4 unity = 1;
	return NuSDaS_cut2_raw(type1, type2, type3, basetime, member,
			validtime, &unity, plane, plane, element,
			udata, usize,
			ixstart, ixfinal, iystart, iyfinal);
}
