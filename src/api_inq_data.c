#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include "sys_err.h"
#include "sys_time.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSDIMS
#include "sys_sym.h"
#include "glb.h"
#include "dset.h"

struct inqdata_dsselect_info {
	nusdims_t nusdims;
	N_SI4 item;
	void *data;
	N_SI4 nelems;
};

	static int
inqdata_dsselect(nusdset_t *ds, void *arg)
{
	struct inqdata_dsselect_info *info = arg;
	int r;
	r = ds_inq_data(ds, &(info->nusdims),
			info->item, info->data, info->nelems); 
	if (r <= 0) {
		SETERR(r);
		return 0;
	}
	return r;
}

/* N_DATA_EXIST �Ϥ���������dsscan�˼��Ԥ��������ͤ��Ǽ���� */
	static void
inqdata_rescue(N_SI4 item, void* data, const N_UI4* nelems)
{
	N_UI4 *data4;
	if (N_DATA_EXIST == item) {
 		data4 = (N_UI4 *)data;
		if(1 <= *nelems) *data4 = 0;
	}
}

/** @brief �ǡ�����Ͽ�ν�����礻 */
	N_SI4
NuSDaS_inq_data2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		const char plane1[6], /**< ��1 */
		const char plane2[6], /**< ��2 */
		const char element[6], /**< ����̾ */
		N_SI4 item, /**< ��礻���ܥ����� */
		void *data, /**< INTENT(OUT) ��̳�Ǽ���� */
		const N_SI4 *nelems) /**< ��̳�Ǽ�������ǿ� */
{
	struct inqdata_dsselect_info info;
	nustype_t type;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_inq_data"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusdims(*basetime, member, *validtime1, *validtime2,
			plane1, plane2, element, &(info.nusdims));
	info.item = item;
	info.data = data;
	info.nelems = *nelems;
	r = nusglb_dsscan_nustype(inqdata_dsselect, &type, &info);
	if (0 >= r) inqdata_rescue(item, data, nelems);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return (r > 0) ? r : NUS_ERR_CODE();
}

/** @brief �ǡ�����Ͽ�ν�����礻
 *
 * ���� @p type1 ���� @p element �ޤǤǻ��ꤵ���ǡ�����Ͽ�ˤĤ���
 * ���� @p query �ǻ��ꤵ�����礻��Ԥ���
 *
 * <DL>
 * <DT>N_DATA_QUADRUPLET<DD>
 * 16 �Х��ȤΥ����ΰ������˼�ꡢ N_GRID_SIZE ����
 * <BR>N_MISSING_VALUE �ޤǤξ����֤���롣
 * <DT>N_GRID_SIZE<DD>
 * ���� @p data ��4�Х���������Ĺ��2��������ꡢ
 * ������ X, Y �����γʻҿ����񤫤�롣
 * <DT>N_PC_PACKING<DD>
 * ���� @p data ��4�Х��Ȥ�ʸ������ꡢ
 * �����˥ѥå�����̾�Τ��񤫤�롣
 * ʸ����ϥ̥뽪ü����ʤ����Ȥ���ա�
 * <DT>N_MISSING_MODE<DD>
 * ���� @p data ��4�Х��Ȥ�ʸ������ꡢ
 * �����˷�»��ɽ������̾���񤫤�롣
 * ʸ����ϥ̥뽪ü����ʤ����Ȥ���ա�
 * <DT>N_MISSING_VALUE<DD>
 * �����ˤϾ�� N_PC_PACKING ���ܤˤ�äƷ�ޤ뷿���ѿ����ꡢ
 * �����˥ǡ�����Ͽ��η�»�ͤ��񤫤�롣
 * �����ͤ� nusdas_read() ��������������Ѥ�����
 * ��»�ͤȤϰۤʤ뤳�Ȥ���ա�
 * <DT>N_DATA_EXIST<DD>
 * ���� @p data ��4�Х����������ѿ���Ȥꡢ
 * �����˥ǡ�����¸��̵ͭ�򼨤��ͤ��񤫤�롣
 * 0�ϥǡ������Ժߡ�1��¸�ߤ򼨤���
 * <DT>N_DATA_NBYTES<DD>
 * ���� @p data ��4�Х����������ѿ���Ȥꡢ
 * �����˥ǡ�����Ͽ�ΥХ��ȿ����񤫤�롣
 * <DT>N_DATA_CONTENT<DD>
 * ���� @p data ���ؤ��Х�����˥ǡ�����Ͽ�����Τޤ޽񤫤�롣
 * <DT>N_RECORD_TIME<DD>
 * ���� @p data ��4�Х����������ѿ���Ȥꡢ
 * �����˥ǡ�����Ͽ�κ������郎�񤫤�롣
 * ������礻�ϥǡ�����Ͽ�ι�����ǧ�Ѥ��Ѱդ���Ƥ��ꡢ
 * ��̤��羮��Ӥ������Ѥ���٤���Τ��������򻻽Ф��٤��ǤϤʤ���
 * �����ͤ� time �����ƥॳ������֤��ͤβ��� 32 �ӥåȤǤ��ꡢ
 * 2038 ǯ������к��Τ��ᤤ���쵡���¸�ΰ�̣����Ĥ褦��
 * �ʤ��ΤȻפ��롣
 * </DL>
 * @retval �� ��Ǽ���ǿ�
 * @retval -1 �ǡ��������������­���Ƥ���
 * @retval -2 �ǡ��������󤬳��ݤ���Ƥ��ʤ�
 * @retval -3 �䤤��碌���ܤ����� 
 * <H3> ���� </H3>
 * ���δؿ��� pnusdas �Ǥϼ����Ϥ���Ƥ��������ɥ�����Ȳ�����Ƥ��ʤ��ä���
 * */
	N_SI4
NuSDaS_inq_data(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		const char plane[6], /**< �� */
		const char element[6], /**< ����̾ */
		N_SI4 param, /**< ��礻���ܥ����� */
		void *data, /**< INTENT(OUT) ��̳�Ǽ���� */
		const N_SI4 *nelems) /**< ��̳�Ǽ��������ǿ� */
{
	N_SI4 unity = 1;
	return NuSDaS_inq_data2(type1, type2, type3, basetime, member,
			validtime, &unity, plane, plane, element,
			param, data, nelems);
}
