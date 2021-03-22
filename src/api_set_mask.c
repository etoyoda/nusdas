/** @file
 * @brief �������줿�ӥåȥޥ�������ؿ�
 */

#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
#include "dset.h"
#include "glb.h"
#include "sys_time.h"
#include "sys_err.h"
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "sys_mem.h"

/** @brief �������ӥåȥޥ�������ؿ�
 *
 * ���� @p udata �����Ƥ˽��ä� nusdas_make_mask() ��Ʊ�ͤ�
 * �ޥ����ӥå���������
 * ���ꤷ�����̤Υǡ������åȤ��Ф������ꤹ�롣
 *
 * @retval 0 ���ｪλ
 * @retval -5 ̤�Τη�̾ @p utype ��Ϳ����줿
 *
 * <H3>���</H3>
 * �ܴؿ��ˤ��ޥ����ӥåȤ������ nusdas_parameter_change() ��
 * ͥ�褹�뤬��¾�Υǡ������åȤˤϸ��̤�⤿�ʤ���
 *
 * <H3>����</H3>
 * �ܴؿ��� NuSDaS 1.3 �ǿ��ߤ��줿��
 */
	N_SI4
NuSDaS_set_mask(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const void *udata, /**< �ǡ������� */
		const char utype[2], /**< �ǡ�������η� */
		N_SI4 usize /**< ��������ǿ� */)
{
	nusdset_t *ds;
	nustype_t nustype;
	N_UI4 buflen;
	unsigned char *buf;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	nus_debug(("--- NuSDaS_set_mask %#ys", &nustype));
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		nus_err((NUSERR_DsetNotFound, "dataset %#ys not found",
					&nustype));
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUS_ERR_CODE();
	}
	buflen = ((N_UI4)usize - 1) / 8 + 1; 
	if ((buf = nus_malloc(buflen)) == NULL) {
		r = NUSERR_MemShort;
		goto finish;
	}
	r = NuSDaS_make_mask(udata, utype, &usize, buf, (N_SI4 *)&buflen);
	if (r == 0) {
		if (ds->comm.param.dsp_pc_mask_bit) {
			nus_free((void *)ds->comm.param.dsp_pc_mask_bit);
		}
		ds->comm.param.dsp_pc_mask_bit = buf;
	} else {
		nus_free(buf);
	}
finish:
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}
