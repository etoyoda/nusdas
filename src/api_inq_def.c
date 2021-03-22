/** @file
 * @brief nusdas_inq_def() の実装
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include <stddef.h>
#include <string.h>
#include "dset.h"
#include "glb.h"
#include "sys_err.h"
#include "sys_time.h"

/** @brief データセットの諸元問合せ 
 * 引数 @p type1 から @p type3で指定されるデータセットの定義ファイル
 * に書かれた内容について、引数 @p param で指定される問合せを行う。
 * <DL>
 * <DT>N_MEMBER_NUM<DD>
 * 定義ファイルに書かれたメンバーの個数が4バイト整数型変数 @p data に書かれる。
 * <DT>N_MEMBER_LIST<DD>
 * 定義ファイルに書かれたメンバー名が配列 @p data に書かれる。
 * 配列 @p data は長さ 4 文字の文字型で
 * @p N_MEMBER_NUM 要素存在しなければならない。
 * <DT>N_VALIDTIME_NUM<DD>
 * 定義ファイルに書かれたvalidtimeの個数が4バイト整数型変数 
 * @p data に書かれる。
 * <DT>N_VALIDTIME_LIST<DD>
 * 定義ファイルに書かれたvalidtimeが配列 @p data に書かれる。
 * 配列 @p data は長さ 4 byte整数型で
 * @p N_VALIDTIME_NUM 要素存在しなければならない。
 * <DT>N_VALIDTIME_LIST2<DD>
 * 定義ファイルに書かれた validtime2 が配列 @p data に書かれる。
 * <BR>配列 @p data は長さ 4 byte整数型で
 * @p N_VALIDTIME_NUM 要素存在しなければならない。
 * <DT>N_VALIDTIME_UNIT<DD>
 * 定義ファイルに書かれた validtime の単位が4文字の文字型変数 @p data 
 * に書かれる。
 * <DT>N_PLANE_NUM<DD>
 * 定義ファイルに書かれた面の個数が4バイト整数型変数 @p data に書かれる。
 * <DT>N_PLANE_LIST<DD>
 * 定義ファイルに書かれた面の名前が配列 @p data に書かれる。
 * 配列 @p data は長さ 6 文字の文字型で
 * @p N_PLANE_NUM 要素存在しなければならない。
 * <DT>N_PLANE_LIST2<DD>
 * 定義ファイルに書かれた面2の名前が配列 @p data に書かれる。
 * 配列 @p data は長さ 6 文字の文字型で
 * @p N_PLANE_NUM 要素存在しなければならない。
 * <DT>N_ELEMENT_NUM<DD>
 * 定義ファイルに書かれた要素の個数が4バイト整数型変数 @p data に書かれる。
 * <DT>N_ELEMENT_LIST<DD>
 * 定義ファイルに書かれた要素の名前が配列 @p data に書かれる。
 * 配列 @p data は長さ 6 文字の文字型で
 * @p N_ELEMENT_NUM 要素存在しなければならない。
 * <DT> N_PROJECTION <DD>
 * 定義ファイルに書かれた地図投影法の情報を4文字の文字型 @p data に格納する
 * (記号の意味は巻末の表参照)。
 * <DT> N_GRID_SIZE <DD>
 * 定義ファイルに書かれたX方向、Y方向の格子数がこの順序で4バイト整数型の
 * 配列 @p data に書かれる。配列 @p data は 2 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_GRID_BASEPOINT <DD>
 * 定義ファイルに書かれた基準点のx座標、y座標、緯度、経度が
 * この順序で4バイト単精度浮動小数点型の配列 @p data に書かれる。
 * 配列 @p data は 4 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_GRID_DISTANCE <DD>
 * 定義ファイルに書かれたX方向、Y方向の格子間隔がこの順序で
 * 4バイト単精度浮動小数点型の配列@p data に書かれる。
 * 配列 @p data は 2 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_STAND_LATLON <DD>
 * 定義ファイルに書かれた標準緯度、標準経度、第2標準緯度、第2標準経度が
 * この順序で4バイト単精度浮動小数点型の配列 @p data に書かれる。
 * 配列 @p data は 4 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_SPARE_LATLON <DD>
 * 定義ファイルに書かれた緯度1、経度1、緯度2、経度2がこの順序で
 * 4バイト単精度浮動小数点型の配列 @p data に書かれる。
 * 配列 @p data は 4 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_INDX_SIZE <DD>
 * 定義ファイルから算出されるINDX の個数が 4バイト整数型の変数 @p data 
 * に書かれる。(この問い合わせはNuSDaS1.3で追加)
 * <DT> N_ELEMENT_MAP <DD>
 * 定義ファイルでデータの格納が許容されているか否かが1 or 0 によって、
 * 1バイト整数型の配列 @p data に書かれる。
 * 配列 @p data は @p N_INDX_SIZE 要素存在
 * しなくてはならない。@p dataはメンバー、validtime, 面、要素をインデック
 * スにした配列で、それぞれの順序は @p N_MEMBER_LIST, 
 * @p N_VALIDTIME_LIST, @p N_PLANE_LIST, @p N_ELEMENT_LISTの問い合わせ
 * 結果と一致する。
 * <DT>N_SUBC_NUM<DD>
 * 定義ファイルに書かれた SUBC 記録の個数が4バイト整数型変数 @p buf に書かれる。
 * <DT>N_SUBC_LIST<DD>
 * 定義ファイルに書かれた SUBC 記録の群名が配列 @p buf に書かれる。
 * 配列 @p buf は長さ 4 文字の文字型で
 * @p N_SUBC_NUM 要素存在しなければならない。
 * <DT>N_INFO_NUM<DD>
 * 定義ファイルに書かれた INFO 記録の個数が4バイト整数型変数 @p buf に書かれる。
 * <DT>N_INFO_LIST<DD>
 * 定義ファイルに書かれた INFO 記録の群名が配列 @p buf に書かれる。
 * 配列 @p buf は長さ 4 文字の文字型で
 * @p N_INFO_NUM 要素存在しなければならない。
 * </DL>
 * @retval 正 格納要素数
 * @retval -1 格納配列が不足
 * @retval -2 格納配列が確保されていない
 * @retval -3 問い合わせが不正
 * <H3> 履歴 </H3>
 * この関数は NuSDaS1.0 より実装されていたが、NuSDaS1.3 でいくつかの問い合わせ
 * 機能が追加されている。
 */
	N_SI4
NuSDaS_inq_def(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 param, /**< 問合せ項目コード */
		void	*data, /**< INTENT(OUT) 結果格納配列 */
		const N_SI4 *datasize) /**< 結果格納配列の要素数 */
{
	N_SI4 r;
	nusdset_t *ds;
	nustype_t nustype;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		NUSPROF_MARK(NP_USER);
		r = nus_err((NUSERR_DsetNotFound, "dataset %ys not found",
					&nustype));
		NUSDAS_CLEANUP;
		return r;
	}
	r = ds_inq_def(ds, param, data, *datasize);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}
