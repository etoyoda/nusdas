/** @file
 * @brief nusdas_grid() の実装
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

/** @brief 格子情報へのアクセス */
	N_SI4
NuSDaS_grid2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		char	proj[4], /**< 投影法3字略号 */
		N_SI4	gridsize[2], /**< 格子数 */
		float	*gridinfo, /**< 投影法緒元 */
		char	value[4], /**< 格子点値が周囲の場を代表する方法 */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
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

/** @brief 格子情報へのアクセス
 * このAPIは、CNTLレコードに格納された格子情報(つまり定義ファイルに書かれた
 * 格子情報)を返す。nusdas_parameter_change を使って、定義ファイルに書いた
 * 格子数から変更した場合には正しい情報が得られない。このような場合は 
 * nusdas_inq_data を使う。
 *
 * gridinfo には4バイト単精度浮動小数点型の配列で14要素存在するものを指定する。
 * 
 * これはCNTLレコードの項番 15 〜 21に対応する。
 * 順に基準点X座標、基準点Y座標、基準点緯度、基準点経度、
 * X方向格子間隔、Y方向格子間隔、標準緯度、標準経度、第2標準緯度、第2標準経度、
 * 緯度1、経度1、緯度2、経度2となる。
 *
 * @retval 0 正常
 * @retval -5 入出力指示が不正
 * <H3> 履歴 </H3>
 * この関数は NuSDaS 1.0 から実装されていた。
 */
	N_SI4
NuSDaS_grid(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		char	proj[4], /**< 投影法3字略号 */
		N_SI4	gridsize[2], /**< 格子数 */
		float	*gridinfo, /**< 投影法緒元 */
		char	value[4], /**< 格子点値が周囲の場を代表する方法 */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4 unity = 1;
	return NuSDaS_grid2(type1, type2, type3, basetime, member,
			validtime, &unity, proj, gridsize, gridinfo, value,
			getput);
}
