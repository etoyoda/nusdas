/** @file
 * @brief �ǡ������å�õ��
 */

#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <sys/types.h> /* for opendir */
#include <string.h>
#include "internal_types.h"
#include "sys_container.h"
# define NEED_STRING_COPY
#include "sys_string.h"
# define NEED_SI8_CMP
#include "sys_int.h"
#include "sys_err.h" /* for nus_snprintf */
#include "io_comm.h"
#include "dset.h"
#include "glb.h"

/** @brief �� NRD �ֹ���Ф���ǡ������åȤΥꥹ��
 *
 * �ºݤ˻��Ѥ����Τ�ź�� 1 ���� 99 �Τߡ�
 */
static listp_t	*(nrdDatasetList[100]);

/** @brief ���̤������õ�������ǡ������åȤΥꥹ�Ȥ�Ϳ����ϥå���
 *
 * nustype_t �򥭡��ˤ��� nustype_dstab ��Ϳ����
 */
static struct glb_typetab_t *nustypeTable = NULL;

	int
nusglb_pushdset(union nusdset_t *ds, int nrd)
{
	if (nrdDatasetList[nrd] == NULL) {
		nrdDatasetList[nrd] = listp_ini();
	}
	if (listp_push(nrdDatasetList[nrd], ds) < 0) {
		return -1;
	}
	return 0;
}

static int
allds_push_callback(nusdset_t *ds, void *arg UNUSED)
{
	struct nustype_dstab *dst;
	nustype_t nustype = ds->comm.nustype;

	if (nustypeTable == NULL) {
		if ((nustypeTable = glb_typetab_ini(127)) == NULL) {
			return -1;
		}
	}
	if ((dst = glb_typetab_get(nustypeTable, &nustype)) == NULL) {
		if ((dst = nustype_dstab_ini()) == NULL) {
			return -1;
		}
		glb_typetab_put(nustypeTable, &nustype, dst);
	}
	nustype_dstab_push(dst, ds);
	return 0;
}

int
nusglb_allds_push(void)
{
	unsigned i;
	int r;

	for (i = 1; i <= 99; i++) {
		if (nrdDatasetList[i] == NULL)
			continue;
		r = listp_each(nrdDatasetList[i],
				(int (*)(void *, void *))allds_push_callback, 
			       NULL);
		nus_debug(("allds_push_callback => %d", r));
		if (r != 0)
			return r;
	}
	return 0;
}

	int
nusglb_dsscan(int (*callback)(nusdset_t *ds, void *arg), void *arg)
{
	int	i, r;
	for (i = 1; i <= 99; i++) {
		if (nrdDatasetList[i] == NULL)
			continue;
		r = listp_each(nrdDatasetList[i],
				(int (*)(void *, void *))callback, arg);
		nus_debug(("listp_each => %d", r));
		if (r != 0)
			return r;
	}
	return 0;
}

	union nusdset_t *
nusglb_dsscan2(void)
{
	static int nrd = 0;
	static struct listp_entry *cursor = NULL;
	struct listp_entry *r;
	while (cursor == NULL) {
		nrd++;
		if (nrd >= 100) {
			cursor = NULL;
			nrd = 0;
			return NULL;
		}
		if (GlobalConfig(nrd_override) != NRD_UNFIX &&
		    nrd != GlobalConfig(nrd_override)) {
			continue;
		}
		if (nrdDatasetList[nrd]) {
			cursor = nrdDatasetList[nrd]->next;
		}
	}
	r = cursor;
	cursor = cursor->next;
	return r->obj;
}

static struct nustype_dstab *
get_dstab(nustype_t *nustype)
{
	struct nustype_dstab *dst;
	if (nustypeTable == NULL) {
		if ((nustypeTable = glb_typetab_ini(127)) == NULL) {
			return NULL;
		}
	}
	if ((dst = glb_typetab_get(nustypeTable, nustype)) == NULL) {
		if ((dst = nustype_dstab_ini()) == NULL) {
			return NULL;
		}
		glb_typetab_put(nustypeTable, nustype, dst);
	}
	return dst;
}

struct dsscan_nrdfilter_info {
	int	(*callback)(nusdset_t *ds, void *arg);
	void	*arg;
};

static int dsscan_nrdfilter(nusdset_t *ds, void *arg)
{
	struct dsscan_nrdfilter_info *finfo = arg;
	if (ds->comm.nrd == GlobalConfig(nrd_override)) {
		return finfo->callback(ds, finfo->arg);
	} else {
		return 0;
	}
}

/** @brief ������̥ǡ������å�õ�� (read ��: ��������)
 *
 * NRD �ֹ��˻��ꤵ�줿���� @p nustype �Υǡ������åȤ�õ����
 * ���줾��ˤĤ��� callback(ds, arg) ��Ƥ��֤���
 * ������Хå���������֤���õ�������Ǥ���롣
 * ������Хå������ｪλ�������ǽ�ʤ�С����ｪλ�������ͤ��ֵѤ���
 * ���Ȥ��侩����롣
 *
 * @retval NUSERR_MemShort(����) ������­
 * @retval 0 �ǡ������åȤ��ߤĤ���ʤ��ä�
 * @retval ¾ ������Хå����ֵ���
 */
int
nusglb_dsscan_nustype(int (*callback)(nusdset_t *ds, void *arg),
		nustype_t *nustype, void *arg)
{
	struct dsscan_nrdfilter_info finfo;
	struct nustype_dstab *dst;
	int r;
	nuserr_mark(MARK_FOR_DSET);
	SETERR(NUSERR_INITVALUE);
	if (nusglb_intp_type1(&nustype->type1) != 0)
		return NUS_ERR_CODE();
	dst = get_dstab(nustype);
	if (dst == NULL) {
		return nus_err((NUSERR_MemShort, "memory short"));
	}
	if (GlobalConfig(nrd_override) != NRD_UNFIX) {
		finfo.arg = arg;
		finfo.callback = callback;
		arg = &finfo;
		callback = dsscan_nrdfilter;
	}
	if ((r = nustype_dstab_each(dst, callback, arg)) != 0) {
		return r;
	}
	if (!r && NUS_ERR_CODE() == NUSERR_INITVALUE) {
		nus_err((NUSERR_DsetNotFound, 
			 "Can not find NUSDAS root directory "
			 "for selected type1-3\n"
			 "type1<%Qs> type2<%Ps> type3<%Ps> NRD=%Pd", 
			 nustype->type1, nustype->type2, nustype->type3, 
			 GlobalConfig(nrd_override)));
	}
	nus_debug(("nusglb_dsscan => %d", r));
	return r;
}

struct finddset_callback_info {
	/** ��礻����� (����) */
	nustype_t	*nustype;
	/** ���̤��Ȥ� DS ɽ (����) - ����ʹߤθ�������������å��夹�� */
	struct nustype_dstab	*dstab;
	/** ������� (����) */
	nusdset_t	*ds;
};

static int finddset_callback(nusdset_t *ds, void *ginfo)
{
	struct finddset_callback_info *info = ginfo;
	if (nustype_eq(ds->comm.nustype, *info->nustype)) {
		info->ds = ds;
		return 1;
	} else {
		return 0;
	}
}

static int finddset_callback_fixnrd(nusdset_t *ds, void *ginfo)
{
	struct finddset_callback_info *info = ginfo;
	if (ds->comm.nrd == GlobalConfig(nrd_override)
	&& nustype_eq(ds->comm.nustype, *info->nustype)) {
		info->ds = ds;
		return 1;
	} else {
		return 0;
	}
}

/** @brief ������̥ǡ������å�õ�� (write ��: �ҤȤĤ���)
 *
 * ���ꤵ�줿���� @p nustype �Υǡ������åȤΤ����ǽ�Τ�Τ��֤���
 */
nusdset_t *
nusglb_find_dset(nustype_t *nustype)
{
	struct finddset_callback_info info;
	struct nustype_dstab *dst;
	nusdset_t *ds;
	if (nusglb_intp_type1(&nustype->type1) != 0)
		return NULL;
	dst = get_dstab(nustype);
	if (dst == NULL) {
		nus_err((NUSERR_MemShort, "memory short"));
		return NULL;
	}
	if ((ds = nustype_dstab_first(dst)) != NULL) {
		return ds;
	}
	info.nustype = nustype;
	info.ds = NULL;
	info.dstab = dst;
	nusglb_dsscan((GlobalConfig(nrd_override) == NRD_UNFIX
			? finddset_callback
			: finddset_callback_fixnrd), &info);
	return info.ds;
}
