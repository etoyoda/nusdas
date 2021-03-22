/** @file
 * @brief nusdas_grid() �μ���
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <ctype.h>
#include "sys_kwd.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
# define NEED_STR3SYM4UPCASE
#include "sys_sym.h"
#include "sys_time.h"
#include <stddef.h>
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

	static int
grid_dsselect(nusdset_t *ds, void *arg)
{
	struct inq_grid_info *info = arg;
	int r;
	r = ds_inq_grid(ds, info);
	return r < 0 ? 0 : r;
}

/** @brief �ʻҾ���ؤΥ������� */
	N_SI4
NuSDaS_grid2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		char	proj[4], /**< ���ˡ3��ά�� */
		N_SI4	gridsize[2], /**< �ʻҿ� */
		float	*gridinfo, /**< ���ˡ�︵ */
		char	value[4], /**< �ʻ����ͤ����Ϥξ����ɽ������ˡ */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	nustype_t nustype;
	struct inq_grid_info info;
	sym4_t op;
	int r;

	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2,
			&info.nusdims);
	op = str3sym4upcase(getput);
	nus_debug(("--- nusdas_grid(%Ps)", op));
	if (op == SYM4_GET) {
		info.proj = proj;
		info.gridsize = gridsize;
		info.gridinfo = gridinfo;
		info.value = value;
		r = nusglb_dsscan_nustype(grid_dsselect, &nustype, &info);
		r = r <= 0 ? NUS_ERR_CODE() : 0;
		if (r == 0) {
			nuserr_cancel(MARK_FOR_DSET);
		}
	} else if (op == SYM4_PUT) {
		union nusdset_t *ds;
		ds = nusglb_find_dset(&nustype);
		r = ds_write_grid(ds, &info.nusdims,
				proj, gridsize, gridinfo, value);
	} else {
		r = nus_err((NUSERR_GD_BadParam, "Bad parameter GET/PUT"));
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief �ʻҾ���ؤΥ�������
 * ����API�ϡ�CNTL�쥳���ɤ˳�Ǽ���줿�ʻҾ���(�Ĥޤ�����ե�����˽񤫤줿
 * �ʻҾ���)���֤���nusdas_parameter_change ��Ȥäơ�����ե�����˽񤤤�
 * �ʻҿ������ѹ��������ˤ����������������ʤ������Τ褦�ʾ��� 
 * nusdas_inq_data ��Ȥ���
 *
 * gridinfo �ˤ�4�Х���ñ������ư���������������14����¸�ߤ����Τ���ꤹ�롣
 * 
 * �����CNTL�쥳���ɤι��� 15 �� 21���б����롣
 * ��˴����X��ɸ�������Y��ɸ����������١���������١�
 * X�����ʻҴֳ֡�Y�����ʻҴֳ֡�ɸ����١�ɸ����١���2ɸ����١���2ɸ����١�
 * ����1������1������2������2�Ȥʤ롣
 *
 * @retval 0 ����
 * @retval -5 �����ϻؼ�������
 * <H3> ���� </H3>
 * ���δؿ��� NuSDaS 1.0 �����������Ƥ�����
 */
	N_SI4
NuSDaS_grid(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		char	proj[4], /**< ���ˡ3��ά�� */
		N_SI4	gridsize[2], /**< �ʻҿ� */
		float	*gridinfo, /**< ���ˡ�︵ */
		char	value[4], /**< �ʻ����ͤ����Ϥξ����ɽ������ˡ */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4 unity = 1;
	return NuSDaS_grid2(type1, type2, type3, basetime, member,
			validtime, &unity, proj, gridsize, gridinfo, value,
			getput);
}
