/** \file
 * \brief nusdas_write �μ�����
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include "dset.h"
#include "glb.h"
# define NEED_PACK2NUSDIMS
# define NEED_PACK2NUSTYPE
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "dfile.h"
#include "sys_time.h"
#include "sys_err.h"

/** @brief �ǡ�����Ͽ�ν��
 *
 * �ǡ����쥳���ɤ���ꤵ�줿���˽񤭽Ф���
 *
 * @retval �� �ºݤ˽񤭽Ф��줿���ǿ�
 * @retval -2 ���С�̾����̾������̾���ְ�äƤ���
 * @retval -2 ���Υ쥳���ɤ� ELEMENTMAP �ˤ�äƽ񤭽Ф����ػߤ���Ƥ���
 * @retval -3 Ϳ����줿�ǡ������ǿ� @p nelems ��ɬ�פ�꾮����
 * @retval -4 ����ǡ������åȤˤϥǡ�������η� @p fmt �Ͻ񤭽Ф��ʤ�
 * @retval -5 �ǡ����쥳����Ĺ������쥳����Ĺ��Ķ����
 * @retval -6 �ǡ������åȤη�»�ͻ��������� RLEN ���̤�ʻ�ѤǤ��ʤ�
 * @retval -7 �ޥ����ӥåȤ����꤬����Ƥ��ʤ�
 * @retval -8 ���󥳡��ɲ����ǤΥ��顼 (���ͤ�����ޤ��� RLEN ���̥��顼)
 *
 * <H3>���</H3>
 * <UL>
 * <LI>�ǡ������åȤλ���Ȱۤʤ��礭���Υ쥳���ɤ�񤭽Ф��ˤϤ��餫����
 * nusdas_parameter_change() ��Ȥä�������Ѥ��Ƥ�����
 * <LI>�ʻҿ� (�ǡ������åȤλ���ޤ��� nusdas_parameter_change() ����)
 * ����礭�����ǿ� @p nelems ����ꤹ��ȥ��顼�ˤϤʤ餺��
 * ;�ä����Ǥ��񤭽Ф���ʤ���̤Ȥʤ�Τ���դ��줿����
 * </UL>
 *
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.0 ����¸�ߤ�����
 */
	N_SI4
NuSDaS_write(
		const char utype1[8],	/**< ����1 */
		const char utype2[4],	/**< ����2 */
		const char utype3[4],	/**< ����3 */
		const N_SI4 *basetime,	/**< ������(�̻�ʬ) */
		const char member[4],	/**< ���С�̾ */
		const N_SI4 *validtime,	/**< �оݻ���(�̻�ʬ) */
		const char plane[6],	/**< �̤�̾�� */
		const char element[6],	/**< ����̾ */
		const void *data,	/**< �ǡ������� */
		const char fmt[2],	/**< �ǡ�������η� */
		const N_SI4 *nelems)	/**< �ǡ�����������ǿ� */
{
	nusdset_t *ds;
	nustype_t nustype;
	nusdims_t dims;
	struct obuffer_t userdata;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(utype1, utype2, utype3, &nustype);
	pack2nusdims(*basetime, member, *validtime, 1, plane, NULL,
			element, &dims);
	nus_debug(("--- NuSDaS_write %#ys/%#ms", &nustype, &dims));
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		nus_err((NUSERR_DsetNotFound, "dataset %#ys not found",
					&nustype));
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUSERR_DsetNotFound;
	}
	userdata.ob_ptr = data;
	userdata.ob_fmt = mem2sym4(fmt);
	userdata.nelems = *nelems;
	r = ds_writedata(ds, &dims, &userdata);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief �ǡ�����Ͽ�ν��
 */
	N_SI4
NuSDaS_write2(
		const char utype1[8],	/**< ����1 */
		const char utype2[4],	/**< ����2 */
		const char utype3[4],	/**< ����3 */
		const N_SI4 *basetime,	/**< ������(�̻�ʬ) */
		const char member[4],	/**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		const char plane1[6],	/**< �̤�̾��1 */
		const char plane2[6],	/**< �̤�̾��2 */
		const char element[6],	/**< ����̾ */
		const void *data,	/**< �ǡ�����Ϳ�������� */
		const char fmt[2],	/**< data �η� */
		const N_SI4 *nelems)	/**< data �����ǿ� */
{
	nusdset_t *ds;
	nustype_t nustype;
	nusdims_t dims;
	struct obuffer_t userdata;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- NuSDaS_write2"));
	pack2nustype(utype1, utype2, utype3, &nustype);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		nus_err((NUSERR_DsetNotFound, "dataset not found"));
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUSERR_DsetNotFound;
	}
	pack2nusdims(*basetime, member, *validtime1, *validtime2,
			plane1, plane2, element, &dims);
	userdata.ob_ptr = data;
	userdata.ob_fmt = mem2sym4(fmt);
	userdata.nelems = *nelems;
	r = ds_writedata(ds, &dims, &userdata);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

