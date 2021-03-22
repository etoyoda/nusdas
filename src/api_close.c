/** @file
 * @brief nusdas_onefile_close(), nusdas_allfile_close() の実装
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include "dset.h"
#include "glb.h"
#include "sys_time.h"
# define NEED_PACK2NUSBMV
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "sys_err.h" /* for NUS_ERR_CODE() */

/** @brief データセットを閉じる際のオプションと結果情報
 */
struct All_File_Close_Info {
	N_SI4	param;		/**< @brief 閉じるデータファイルの種類
				  @todo 今のところ無視されている */
	int	ok;		/**< @brief 閉じたデータファイル数 */
	int	ng;		/**< @brief 閉じる際にエラーになった
				  データファイル数 */
};

/** @brief ひとつのデータセットを閉じる
 */
static int
All_File_Close_Callback(union nusdset_t *ds, void *arg)
{
	struct All_File_Close_Info *info = arg;
	int	r;
	r = ds_close(ds, info->param);
	if (r > 0) {
		info->ok += r;
	} else if (r < 0) {
		info->ng += r;
	}
	return 0;
}

/** @brief 全てのデータファイルを閉じる
 *
 * 今までに NuSDaS インターフェイスで開いた全てのファイルを閉じる.
 * 引数 param は次のいずれかを用いる:
 * <DL>
 * <DT>N_FOPEN_READ<DD>読み込み用に開いたファイルだけを閉じる
 * <DT>N_FOPEN_WRITE<DD>書き込み可で開いたファイルだけを閉じる
 * <DT>N_FOPEN_ALL<DD>すべてのファイルを閉じる
 * </DL>
 *
 * @retval 正 正常に閉じられたファイルの数
 * @retval 0 閉じるべきファイルがなかった
 * @retval 負 閉じる際にエラーが起こったファイルの数
 *
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.0 から存在した.
 */
	N_SI4
NuSDaS_allfile_close(N_SI4 param /**< 閉じるファイルの種類 */)
{
	struct All_File_Close_Info info;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	info.param = param;
	info.ok = info.ng = 0;
	nus_debug(("--- NuSDaS_allfile_close %Pd", param));
	nusglb_dsscan(All_File_Close_Callback, &info);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return info.ng ? info.ng : info.ok;
}

/** @brief 指定データファイルを閉じる
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.0 から存在した.
 */
	N_SI4
NuSDaS_onefile_close(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime) /**< 対象時刻 */
{
	nustype_t nustype;
	nusdims_t dims;
	union nusdset_t *ds;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime, 1, &dims);
	nus_debug(("--- NuSDaS_onefile_close %#ys/%#ms", &nustype, &dims));
	SETERR(NUSERR_DsetNotFound);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUS_ERR_CODE();
	}
	r = ds_close_file(ds, &dims);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief ひとつのファイルを閉じる
 */
	N_SI4
NuSDaS_onefile_close2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2) /**< 対象時刻2(通算分) */
{
	nustype_t nustype;
	nusdims_t dims;
	union nusdset_t *ds;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	nus_debug(("--- NuSDaS_onefile_close2 %#ys/%#ms", &nustype, &dims));
	SETERR(NUSERR_DsetNotFound);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUS_ERR_CODE();
	}
	r = ds_close_file(ds, &dims);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}
