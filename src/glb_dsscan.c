/** @file
 * @brief データセット探索
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

/** @brief 各 NRD 番号に対するデータセットのリスト
 *
 * 実際に使用されるのは添字 1 から 99 のみ。
 */
static listp_t	*(nrdDatasetList[100]);

/** @brief 種別から既に探索したデータセットのリストを与えるハッシュ
 *
 * nustype_t をキーにして nustype_dstab を与える
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

/** @brief 指定種別データセット探索 (read 用: 該当全部)
 *
 * NRD 番号順に指定された種別 @p nustype のデータセットを探索し
 * それぞれについて callback(ds, arg) を呼び返す。
 * コールバックが非零を返すと探索は中断される。
 * コールバックに正常終了が定義可能ならば、正常終了時に正値を返却する
 * ことが推奨される。
 *
 * @retval NUSERR_MemShort(負値) メモリ不足
 * @retval 0 データセットがみつからなかった
 * @retval 他 コールバックの返却値
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
	/** 問合せる種別 (入力) */
	nustype_t	*nustype;
	/** 種別ごとの DS 表 (入力) - 次回以降の検索に備えキャッシュする */
	struct nustype_dstab	*dstab;
	/** 検索結果 (出力) */
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

/** @brief 指定種別データセット探索 (write 用: ひとつだけ)
 *
 * 指定された種別 @p nustype のデータセットのうち最初のものを返す。
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
