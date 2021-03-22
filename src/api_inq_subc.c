/** @file
 * @brief API nusdas_inq_subcinfo() の実装
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_err.h"
#include "sys_time.h"
#include "dset.h"
#include "glb.h"

struct inq_aux_info {
	nusdims_t	nusdims;
	N_SI4		query;
	sym4_t		group;
	void		*buf;
	N_UI4		bufnelems;
};

	static int
inq_aux_dsselect(nusdset_t *ds, void *arg)
{
	struct inq_aux_info *info = arg;
	int r;
	r = ds_inq_aux(ds, &info->nusdims, info->query, info->group,
			info->buf, info->bufnelems); 
	return r;
}

/** @brief SUBC/INFO の問合せ
 *
 * 引数 @p type1 から @p validtime で指定されるデータファイルに書かれた
 * SUBC または INFO 記録について、
 * 引数 @p query で指定される問合せを行う。
 * <DL>
 * <DT>N_SUBC_NUM<DD>
 * SUBC 記録の個数が4バイト整数型変数 @p buf に書かれる。
 * 引数 @p group は無視される。
 * <DT>N_SUBC_LIST<DD>
 * データファイルに定義された SUBC 記録の群名が配列 @p buf に書かれる。
 * 配列 @p buf は長さ 4 文字の文字型で
 * @p N_SUBC_NUM 要素存在しなければならない。
 * 引数 @p group は無視される。
 * <DT>N_SUBC_NBYTES<DD>
 * 群名 @p group の SUBC 記録のバイト数が4バイト整数型変数 @p buf に書かれる。
 * <DT>N_SUBC_CONTENT<DD>
 * 群名 @p group の SUBC 記録が配列 @p buf に書かれる。
 * 上述のバイト数だけの長さを確保しておかねばならない。
 * <DT>N_INFO_NUM<DD>
 * INFO 記録の個数が4バイト整数型変数 @p buf に書かれる。
 * 引数 @p group は無視される。
 * <DT>N_INFO_LIST<DD>
 * データファイルに定義された INFO 記録の群名が配列 @p buf に書かれる。
 * 配列 @p buf は長さ 4 文字の文字型で
 * @p N_INFO_NUM 要素存在しなければならない。
 * 引数 @p group は無視される。
 * <DT>N_INFO_NBYTES<DD>
 * 群名 @p group の INFO 記録のバイト数が4バイト整数型変数 @p buf に書かれる。
 * </DL>
 *
 * @retval 正 格納要素数
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で新設された。
 */
	N_SI4
NuSDaS_inq_subcinfo(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻 */
		const char member[4], /**< メンバー */
		const N_SI4 *validtime, /**< 対象時刻 */
		N_SI4 query, /**< 問合せ項目 */
		const char group[4], /**< 群名 */
		void *buf, /**< INTENT(OUT) 結果格納配列 */
		const N_SI4 bufnelems) /**< 結果格納配列の要素数 */
{
	struct inq_aux_info info;
	nustype_t	type;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_inq_subcinfo"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime, 1, &info.nusdims);
	info.query = query;
	info.group = MEM2SYM4(group);
	info.buf = buf;
	info.bufnelems = bufnelems;
	r = nusglb_dsscan_nustype(inq_aux_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	if (r == 0) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %#ys not found", &type));
	}
	return r;
}

