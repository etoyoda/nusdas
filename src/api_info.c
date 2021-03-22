/** @file
 * @brief nusdas_info() �μ���
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_time.h"
#include "sys_kwd.h"
#include <string.h>
#include <ctype.h>
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
# define NEED_STR3SYM4UPCASE
#include "sys_sym.h"
#include <stddef.h>
#include "sys_err.h"
#include "glb.h"
#include "dset.h"

struct info_get_param {
	nusdims_t *dims;
	sym4_t grp;
	N_UI4 bytesize;
	char *info;
};

/** @brief INFO �ɼ� (�Хǡ����ե����륳����Хå�)
 * @note rec, siz ������ե����� information ʸ�˽�Ĺ��������������ʬ
 */
	static int
info_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	struct info_get_param *param = vparam;
	if (siz > param->bytesize) {
		return nus_err((NUSERR_SC_ShortBuf,
				"INFO %Pu > your buffer %Pd",
				siz, param->bytesize));
	}
	memcpy(param->info, vrec, siz);
	return siz;
}

/** @brief INFO �ɼ� (�Хǡ������åȥ�����Хå�) */
	static int
info_get_dsselect(nusdset_t *ds, void *vparam)
{
	int r;
	struct info_get_param *param = vparam;
	r = ds_read_aux(ds, param->dims, SYM4_INFO, param->grp,
			info_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_PeekFailed) {
		/* ���顼�����ɤĤ����� */
		r = SETERR(NUSERR_IN_PeekFailed);
	}
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* ���դ���ʤ����õ��³�� */
		return 0;
	}
	return r;
}

/** @brief INFO �ɼ�μ�Ư���� */
	static int
info_get(nustype_t *type, nusdims_t *dims, sym4_t grp, N_SI4 bytesize,
		char *info)
{
	struct info_get_param param;
	int	r;
	param.grp = grp;
	param.dims = dims;
	param.bytesize = bytesize;
	param.info = info;
	r = nusglb_dsscan_nustype(info_get_dsselect, type, &param);
	nus_debug(("nusglb_dsscan_nustype => %d", r));
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return r;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

struct info_put_param {
	const char *info;
	N_SI4 written_size;
};

	static int
info_put_encode(void *rec, N_UI4 siz, void *arg, union nusdset_t *ds UNUSED)
{
	struct info_put_param *param = arg;
	memcpy(rec, param->info, siz);
	param->written_size = siz;
	return 0;
}

/** @brief ETA/SIGM �� SUBC �񤭹��ߤμ�Ư���� */
	static int
info_put(nustype_t *type, nusdims_t *dims, sym4_t group,
		N_UI4 bytesize, char *info)
{
	union nusdset_t *ds;
	struct info_put_param param;
	int r;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	param.info = info;
	r = ds_write_aux(ds, dims, SYM4_INFO, group, bytesize,
		info_put_encode, &param);
	if (r == 0) {
		r = param.written_size;
	}
	return r;
}

/** @brief INFO ��Ͽ�ؤΥ������� */
	N_SI4
NuSDaS_info2(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1(�̻�ʬ) */
		const N_SI4 *validtime2, /**< �оݻ���2(�̻�ʬ) */
		const char group[4], /**< ��̾ */
		char	info[], /**< INFO ��Ͽ���� */
		const N_SI4 *bytesize, /**< INFO ��Ͽ�ΥХ��ȿ� */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	nustype_t	type;
	nusdims_t	dims;
	sym4_t		op, grp;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_info"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	grp = MEM2SYM4(group);
	op = str3sym4upcase(getput);
	if (op == SYM4_GET) {
		r = info_get(&type, &dims, grp, *bytesize, info);
	} else if (op == SYM4_PUT) {
		r = info_put(&type, &dims, grp, *bytesize, info);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief INFO ��Ͽ�ؤΥ������� 
 * @retval ���� �񤭽Ф���INFO�ΥХ��ȿ�
 * @retval -3 �Хåե�����­���Ƥ���
 * @retval -5 �����ϻؼ�������
 *
 * <H3> ��� </H3>
 * NuSDaS1.1�Ǥϡ��Хåե�����­���Ƥ�����Ǥ�Хåե����礭����ʬ������
 * �񤭹��ߡ����Υ��������֤��Ƥ�������NuSDaS1.3�ǤϤ��Τ褦�ʾ���-3���֤롣
 * �ޤ���INFO �Υ������� NuSDaS1.3 �ǿ��ߤ��줿 nusdas_inq_subcinfo ��
 * �䤤��碌���ܤ� N_INFO_NUM �ˤ�������뤳�Ȥ��Ǥ��롣
*/
	N_SI4
NuSDaS_info(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
		const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime, /**< �оݻ���(�̻�ʬ) */
		const char group[4], /**< ��̾ */
		char	info[], /**< INFO ��Ͽ���� */
		const N_SI4 *bytesize, /**< INFO ��Ͽ�ΥХ��ȿ� */
		const char getput[3]) /**< �����ϻؼ� (@p "GET" �ޤ��� @p "PUT") */
{
	N_SI4 unity = 1;
	return NuSDaS_info2(type1, type2, type3, basetime, member,
			validtime, &unity, group, info, bytesize, getput);
}
