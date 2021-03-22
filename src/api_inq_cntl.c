/** @file
 * @brief nusdas_inq_cntl() の実装
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
#include "sys_sym.h"
#include "sys_time.h"
#include <fcntl.h>
#include <stddef.h>
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

struct inq_info {
	nusdims_t	nusdims;
	N_SI4		param;
	void		*data;
	N_SI4		datasize;
};

	static int
inqcntl_dsselect(nusdset_t *ds, void *arg)
{
	struct inq_info *info = arg;
	return ds_inq_cntl(ds, &info->nusdims, info->param,
			info->data, &info->datasize, 0);
}

/** @brief データファイルの諸元問合せ 
 * 引数 @p type1 から @p validtime で指定されるデータファイルに書かれた
 * CNTL 記録について、
 * 引数 @p param で指定される問合せを行う。
 * <DL>
 * <DT>N_MEMBER_NUM<DD>
 * メンバーの個数が4バイト整数型変数 @p data に書かれる。
 * <DT>N_MEMBER_LIST<DD>
 * データファイルに定義されたメンバー名が配列 @p data に書かれる。
 * 配列 @p data は長さ 4 文字の文字型で
 * @p N_MEMBER_NUM 要素存在しなければならない。
 * <DT>N_VALIDTIME_NUM<DD>
 * validtimeの個数が4バイト整数型変数 @p data に書かれる。
 * <DT>N_VALIDTIME_LIST<DD>
 * データファイルに定義されたvalidtimeが配列 @p data に書かれる。
 * 配列 @p data は長さ 4 byte整数型で
 * @p N_VALIDTIME_NUM 要素存在しなければならない。
 * <DT>N_VALIDTIME_LIST2<DD>
 * データファイルに定義されたvalidtime2が配列 @p data に書かれる。
 * 配列 @p data は長さ 4 byte整数型で
 * @p N_VALIDTIME_NUM 要素存在しなければならない。
 * <DT>N_PLANE_NUM<DD>
 * 面の個数が4バイト整数型変数 @p data に書かれる。
 * <DT>N_PLANE_LIST<DD>
 * データファイルに定義された面の名前が配列 @p data に書かれる。
 * 配列 @p data は長さ 6 文字の文字型で
 * @p N_PLANE_NUM 要素存在しなければならない。
 * <DT>N_PLANE_LIST2<DD>
 * N_PLANE_LIST と全く同じ動作である。
 * <DT>N_ELEMENT_NUM<DD>
 * 要素の個数が4バイト整数型変数 @p data に書かれる。
 * <DT>N_ELEMENT_LIST<DD>
 * データファイルに定義された要素の名前が配列 @p data に書かれる。
 * 配列 @p data は長さ 6 文字の文字型で
 * @p N_ELEMENT_NUM 要素存在しなければならない。
 * <DT> N_NUSD_NBYTES <DD>
 * NUSD レコードのサイズ(単位バイト)が4バイト整数型変数 @p data に書か
 * れる。(先頭・末尾に付加されるレコード長の大きさ(4*2バイト)を含む)
 * <DT> N_NUSD_CONTENT <DD>
 * NUSD レコードの内容を配列 @p data に格納する。配列 @p data は
 * <BR>N_NUSD_NBYTES バイト存在しなくてはならない。
 * (先頭・末尾に付加されるレコード長を含む)
 * <DT> N_CNTL_NBYTES <DD>
 * CNTL レコードのサイズ(単位バイト)が4バイト整数型変数 @p data に書か
 * れる。(先頭・末尾に付加されるレコード長の大きさ(4*2バイト)を含む)
 * <DT> N_CNTL_CONTENT <DD>
 * CNTL レコードの内容を配列 @p data に格納する。配列 @p data は
 * <BR>N_CNTL_NBYTES バイト存在しなくてはならない。
 * (先頭・末尾に付加されるレコード長を含む)
 * <DT> N_PROJECTION <DD>
 * 地図投影法の情報を4文字の文字型 @p data に格納する
 * (記号の意味は巻末の表参照)。
 * <DT> N_GRID_SIZE <DD>
 * X方向、Y方向の格子数がこの順序で4バイト整数型の配列 @p data に
 * 書かれる。配列 @p data は 2 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_GRID_BASEPOINT <DD>
 * 基準点のx座標、y座標、緯度、経度がこの順序で4バイト単精度浮動小数点型の配
 * 列 @p data に書かれる。配列 @p data は 4 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_GRID_DISTANCE <DD>
 * X方向、Y方向の格子間隔がこの順序で4バイト単精度浮動小数点型の配列
 * @p data に書かれる。配列 @p data は 2 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_STAND_LATLON <DD>
 * 標準緯度、標準経度、第2標準緯度、第2標準経度がこの順序で
 * 4バイト単精度浮動小数点型の配列 @p data に書かれる。
 * 配列 @p data は 4 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_SPARE_LATLON <DD>
 * 緯度1、経度1、緯度2、経度2がこの順序で
 * 4バイト単精度浮動小数点型の配列 @p data に書かれる。
 * 配列 @p data は 4 要素存在しなくてはならない。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_INDX_SIZE <DD>
 * INDX の個数が 4バイト整数型の変数 @p data に書かれる。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_ELEMENT_MAP <DD>
 * データの格納が許容されているか否かが1 or 0 によって、1バイト整数型
 * の配列 @p data に書かれる。配列 @p data は @p N_INDX_SIZE 要素存在
 * しなくてはならない。@p dataはメンバー、validtime, 面、要素をインデック
 * スにした配列で、それぞれの順序は @p N_MEMBER_LIST, 
 * @p N_VALIDTIME_LIST, @p N_PLANE_LIST, @p N_ELEMENT_LISTの問い合わせ
 * 結果と一致する。
 * (この問い合わせはNuSDaS1.3で追加)
 * <DT> N_DATA_MAP <DD>
 * データが書き込まれているか否かが1 or 0 によって、1バイト整数型
 * の配列 @p data に書かれる。配列 @p data は @p N_INDX_SIZE 要素存在
 * しなくてはならない。@p dataはメンバー、validtime, 面、要素をインデック
 * スにした配列で、それぞれの順序は @p N_MEMBER_LIST, 
 * @p N_VALIDTIME_LIST, @p N_PLANE_LIST, @p N_ELEMENT_LISTの問い合わせ
 * 結果と一致する。
 * (この問い合わせはNuSDaS1.3で追加)
 * </DL>
 *
 * @retval 正 格納要素数
 * @retval -1 データの配列数が不足している。
 * @retval -2 データの配列が確保されていない。
 * @retval -3 問い合わせ項目が不正
 * <H3> 注意 </H3>
 * NuSDaS1.1以前では、同じ構造のデータセットでも
 * N_VALIDTIME_NUM, N_VALIDTIME_LIST の問い合わせ結果が
 * 1つの basetime に複数の validtime を格納するか否かによって異なっていた。
 * これは、validtime でファイルを分ける
 * (異なる validtime のファイルが異なる) 設定ならば
 * データファイルには 1 つの validtime だけが書かれていたからである。
 * しかし NuSDaS1.3では定義ファイルに指定されたすべての validtime が
 * 各データファイルの validtime に格納されているので、問い合わせ結果は
 * 格納形態を問わず一定である。
 */
	N_SI4
NuSDaS_inq_cntl(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		N_SI4 param, /**< 問合せ項目コード */
		void *data, /**< INTENT(OUT) 問合せ結果配列 */
		const N_SI4 *datasize) /**< 問合せ結果配列の要素数 */
{
	nustype_t nustype;
	struct inq_info info;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime, 1, &info.nusdims);
	info.param = param;
	info.data = data;
	info.datasize = *datasize;
	r = nusglb_dsscan_nustype(inqcntl_dsselect, &nustype, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	return r <= 0 ? NUS_ERR_CODE() : r;
}

/** @brief データファイルの諸元問合せ */
	N_SI4
NuSDaS_inq_cntl2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		N_SI4 param, /**< 問合せ項目コード */
		void *data, /**< INTENT(OUT) 問合せ結果配列 */
		const N_SI4 *datasize) /**< 問合せ結果配列の要素数 */
{
	nustype_t nustype;
	struct inq_info info;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &info.nusdims);
	info.param = param;
	info.data = data;
	info.datasize = *datasize;
	r = nusglb_dsscan_nustype(inqcntl_dsselect, &nustype, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	return r <= 0 ? NUS_ERR_CODE() : r;
}
