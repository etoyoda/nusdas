#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sys_kwd.h"
# define NEED_MEM2SYM4
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSDIMS
#include "sys_sym.h"
#include "sys_err.h"
#include "sys_time.h"
#include "dfile.h"
#include "dset.h"
#include "glb.h"

struct cut_dsselect_info {
	nusdims_t	nusdims;
	struct ibuffer_t buf;
	int		readsize;
};

	static int
cut_dsselect(nusdset_t *ds, void *arg)
{
	struct cut_dsselect_info *info = arg;
	info->readsize = ds_readdata(ds, &info->nusdims, &info->buf);
	nus_debug(("ds_readdata => %d", info->readsize));
	if (info->readsize < 0) {
		/* ���顼�����ɤ��������ʤ��Τǥ����֤��Ƥ��� */
		SETERR(info->readsize);
		return 0;
	}
	/* �ɤ᤿ */
	return 1;
}

/** @brief �ΰ����Υǡ����ɼ� */
	N_SI4
NuSDaS_cut2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1 */
		const N_SI4 *validtime2, /**< �оݻ���2 */
		const char plane1[6], /**< ��1 */
		const char plane2[6], /**< ��2 */
		const char element[6], /**< ����̾ */
		void *udata, /**< INTENT(OUT) �ǡ�����Ǽ���� */
		const char utype[2], /**< �ǡ�����Ǽ����η� */
		const N_SI4 *usize, /**< �ǡ�����Ǽ��������ǿ� */
		const N_SI4 *ixstart, /**< $x$ �����ʻ��ֹ沼�� */
		const N_SI4 *ixfinal, /**< $x$ �����ʻ��ֹ��� */
		const N_SI4 *iystart, /**< $y$ �����ʻ��ֹ沼�� */
		const N_SI4 *iyfinal) /**< $y$ �����ʻ��ֹ��� */
{
	struct cut_dsselect_info info;
	nustype_t	type;
	int		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusdims(*basetime, member, *validtime1, *validtime2,
			plane1, plane2,
			element, &info.nusdims);
	nus_debug(("--- nusdas_cut %#ys/%#ms", &type, &info.nusdims));
	info.buf.ib_ptr = udata;
	info.buf.ib_fmt = mem2sym4(utype);
	info.buf.nelems = *usize;
	if (*ixstart <= 0 || *iystart <= 0
			|| *ixfinal < *ixstart || *iyfinal < *iystart) {
		cut_rectangle_disable(&info.buf.ib_cut);
	} else {
		info.buf.ib_cut.cr_xofs = *ixstart - 1;
		info.buf.ib_cut.cr_yofs = *iystart - 1;
		info.buf.ib_cut.cr_xnelems = *ixfinal - *ixstart + 1;
		info.buf.ib_cut.cr_ynelems = *iyfinal - *iystart + 1;
		if (info.buf.nelems < (unsigned)cut_rectangle_size(&info.buf.ib_cut)) {
			r =  nus_err((NUSERR_RD_SmallBuf,
				      "buffer %Pu < %u elements required",
				      info.buf.nelems,
				      cut_rectangle_size(&info.buf.ib_cut)));
			NUSPROF_MARK(NP_USER);
			NUSDAS_CLEANUP;
			return r;
		}
	}
	r = nusglb_dsscan_nustype(cut_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0)
		nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	if (r > 0) {
		return info.readsize;
	} else {
		return NUS_ERR_CODE();
	}
}

/** @brief �ΰ����Υǡ����ɼ�
 *
 * nusdas_read() * ��Ʊ�ͤ������ǡ����쥳���ɤΤ����ʻ���
 * (@p ixstart , @p iystart )--(@p ixfinal , @p iyfinal )
 * ������ @p udata �˳�Ǽ����롣
 *
 * �ʻ��ֹ�� 1 ����Ϥޤ��ΤȤ��뤿�ᡢ
 * @p ixstart �� @p iystart �����Ǥʤ���Фʤ餺���ޤ�
 * @p ixfinal �� @p iyfinal �Ϥ��줾��
 * @p ixstart �� @p iystart �ʾ�Ǥʤ���Фʤ�ʤ���
 * ���ε�§��ȿ��������Ԥä����ϡ��ֵ���-8�Υ��顼�Ȥʤ롣
 * �ʤ���@p iyfinal, @p jyfinal �ξ�¤��ʻҿ���Ķ���Ƥ��뤳�Ȥ�
 * �����å��Ϥ��Ƥ��ʤ��Τ���դ�ɬ�ס�
 *
 * @retval �� �ɤ߽Ф��Ƴ�Ǽ�����ʻҿ�
 * @retval 0 ���ꤷ���ǡ�����̤��Ͽ(����ե������ elementmap �ˤ�äƽ񤭹��ޤ�뤳�Ȥϵ��Ƥ���Ƥ��뤬���ޤ��ǡ������񤭹��ޤ�Ƥ��ʤ�)
 * @retval -2 ���ꤷ���ǡ����ϵ�Ͽ���뤳�Ȥ����Ƥ���Ƥ��ʤ�(elementmap �ˤ�äƶػߤ���Ƥ�����Ȼ��ꤷ����̾������̾����Ͽ����Ƥ��ʤ�����ξ����ޤ�)��
 * @retval -4 ��Ǽ������­
 * @retval -5 ��Ǽ����η��ȥ쥳���ɤε�Ͽ������������
 * @retval -8 �ΰ����ѥ�᡼��������
 *
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.1 ��Ƴ�����졢NuSDaS 1.3 �ǽ��ƥɥ�����Ȥ��줿��
 * <H3>�ߴ���</H3>
 * NuSDaS 1.1 �Ǥϡ�������Υǡ����ե�������Ф��Ƥϡ�
 * @p ixstart �� 0 �ξ��� @p ixstart = 1 ��(@p jystart ��Ʊ��), 
 * @p ixfinal ��X�����γʻҿ���Ķ������ˤϡ�@p ixfinal ��X�����γʻҿ���
 * (@p jyfinal ��Ʊ��)���ɤ��ؤ����Ƥ�������NuSDaS1.3 �Ǥ��ֵ���-8�Υ��顼
 * �Ȥ��롣�ޤ���pandora �ǡ����ˤĤ��Ƥϡ�@p ixstart, @p ixfinal, 
 * @p jystart, @p jyfinal ������Ǥ��뤳�Ȥ����������å�����Ƥ�����
 * NuSDaS1.3 �Ǥϥǡ����ե����롢pandora �Ȥ��Ҥ��̤�Ȥʤ롣
 * */
	N_SI4
NuSDaS_cut(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		const char plane[6], /**< �� */
		const char element[6], /**< ����̾ */
		void *udata, /**< INTENT(OUT) �ǡ�����Ǽ������ */
		const char utype[2], /**< �ǡ�����Ǽ������η� */
		const N_SI4 *usize, /**< �ǡ�����Ǽ����������ǿ� */
		const N_SI4 *ixstart, /**< $x$ �����ʻ��ֹ沼�� */
		const N_SI4 *ixfinal, /**< $x$ �����ʻ��ֹ��� */
		const N_SI4 *iystart, /**< $y$ �����ʻ��ֹ沼�� */
		const N_SI4 *iyfinal) /**< $y$ �����ʻ��ֹ��� */
{
	struct cut_dsselect_info info;
	nustype_t	type;
	int		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusdims(*basetime, member, *validtime, 1, plane, NULL,
			element, &info.nusdims);
	nus_debug(("--- nusdas_cut %#ys/%#ms", &type, &info.nusdims));
	info.buf.ib_ptr = udata;
	info.buf.ib_fmt = mem2sym4(utype);
	info.buf.nelems = *usize;
	if (*ixstart <= 0 || *iystart <= 0
			|| *ixfinal < *ixstart || *iyfinal < *iystart) {
		r = nus_err((NUSERR_RD_BadCutRegion, 
			     "Invalid cut region"));
		NUSDAS_CLEANUP;
		return r;
	} else {
		info.buf.ib_cut.cr_xofs = *ixstart - 1;
		info.buf.ib_cut.cr_yofs = *iystart - 1;
		info.buf.ib_cut.cr_xnelems = *ixfinal - *ixstart + 1;
		info.buf.ib_cut.cr_ynelems = *iyfinal - *iystart + 1;
		if (info.buf.nelems < (unsigned)cut_rectangle_size(&info.buf.ib_cut)) {
			r = nus_err((NUSERR_RD_SmallBuf,
				     "buffer %Pu < %u elements required",
				     info.buf.nelems,
				     cut_rectangle_size(&info.buf.ib_cut)));
			NUSPROF_MARK(NP_USER);
			NUSDAS_CLEANUP;
			return r;
		}
	}
	r = nusglb_dsscan_nustype(cut_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0)
		nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	if (r > 0) {
		return info.readsize;
	} else {
		return NUS_ERR_CODE();
	}
}
