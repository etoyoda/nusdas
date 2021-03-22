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

/* N_DATA_EXIST はその性質上dsscanに失敗した場合も値を格納する */
	static void
inqdata_rescue(N_SI4 item, void* data, const N_UI4* nelems)
{
	N_UI4 *data4;
	if (N_DATA_EXIST == item) {
 		data4 = (N_UI4 *)data;
		if(1 <= *nelems) *data4 = 0;
	}
}

/** @brief データ記録の諸元問合せ */
	N_SI4
NuSDaS_inq_data2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		const char plane1[6], /**< 面1 */
		const char plane2[6], /**< 面2 */
		const char element[6], /**< 要素名 */
		N_SI4 item, /**< 問合せ項目コード */
		void *data, /**< INTENT(OUT) 結果格納配列 */
		const N_SI4 *nelems) /**< 結果格納配列要素数 */
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

/** @brief データ記録の諸元問合せ
 *
 * 引数 @p type1 から @p element までで指定されるデータ記録について
 * 引数 @p query で指定される問合せを行う。
 *
 * <DL>
 * <DT>N_DATA_QUADRUPLET<DD>
 * 16 バイトのメモリ領域を引数に取り、 N_GRID_SIZE から
 * <BR>N_MISSING_VALUE までの情報が返される。
 * <DT>N_GRID_SIZE<DD>
 * 引数 @p data に4バイト整数の長さ2の配列を取り、
 * そこに X, Y 方向の格子数が書かれる。
 * <DT>N_PC_PACKING<DD>
 * 引数 @p data に4バイトの文字列を取り、
 * そこにパック方式名称が書かれる。
 * 文字列はヌル終端されないことに注意。
 * <DT>N_MISSING_MODE<DD>
 * 引数 @p data に4バイトの文字列を取り、
 * そこに欠損値表現方式名が書かれる。
 * 文字列はヌル終端されないことに注意。
 * <DT>N_MISSING_VALUE<DD>
 * 引数には上述 N_PC_PACKING 項目によって決まる型の変数を取り、
 * そこにデータ記録上の欠損値が書かれる。
 * この値は nusdas_read() で得られる配列で用いられる
 * 欠損値とは異なることに注意。
 * <DT>N_DATA_EXIST<DD>
 * 引数 @p data に4バイト整数型変数をとり、
 * そこにデータの存在有無を示す値が書かれる。
 * 0はデータの不在、1は存在を示す。
 * <DT>N_DATA_NBYTES<DD>
 * 引数 @p data に4バイト整数型変数をとり、
 * そこにデータ記録のバイト数が書かれる。
 * <DT>N_DATA_CONTENT<DD>
 * 引数 @p data が指すバイト列にデータ記録がそのまま書かれる。
 * <DT>N_RECORD_TIME<DD>
 * 引数 @p data に4バイト整数型変数をとり、
 * そこにデータ記録の作成時刻が書かれる。
 * この問合せはデータ記録の更新確認用に用意されており、
 * 結果は大小比較だけに用いるべきもので日時等を算出すべきではない。
 * この値は time システムコールの返す値の下位 32 ビットであり、
 * 2038 年問題の対策のためいずれ機種依存の意味を持つように
 * なるものと思われる。
 * </DL>
 * @retval 正 格納要素数
 * @retval -1 データの配列数が不足している
 * @retval -2 データの配列が確保されていない
 * @retval -3 問い合わせ項目が不正 
 * <H3> 履歴 </H3>
 * この関数は pnusdas では実装はされていたが、ドキュメント化されていなかった。
 * */
	N_SI4
NuSDaS_inq_data(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		const char plane[6], /**< 面 */
		const char element[6], /**< 要素名 */
		N_SI4 param, /**< 問合せ項目コード */
		void *data, /**< INTENT(OUT) 結果格納配列 */
		const N_SI4 *nelems) /**< 結果格納配列の要素数 */
{
	N_SI4 unity = 1;
	return NuSDaS_inq_data2(type1, type2, type3, basetime, member,
			validtime, &unity, plane, plane, element,
			param, data, nelems);
}
