/** \file
 * \brief nusdas_read の実装。
 */

#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "internal_types.h"
# define NEED_PACK2NUSDIMS
# define NEED_PACK2NUSTYPE
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_time.h"
#include "dset.h"
#include "dfile.h"
#include "glb.h"
#include "sys_err.h"

/** @brief nusdas_read() から read_dsselect() に渡される情報
 */
struct read_dsselect_info {
	nusdims_t	nusdims;
	struct ibuffer_t buf;
	int		readsize;
};

/** 各データセットに対する read 処理.
 * @todo 成功したら 1 を返して nusglb_dsscan() を止めること
 */
	static int
read_dsselect(nusdset_t *ds, void *arg)
{
	struct read_dsselect_info *info = arg;
	info->readsize = ds_readdata(ds, &(info->nusdims), &(info->buf));
	nus_debug(("ds_readdata => %d", info->readsize));
	if (info->readsize < 0) {
		/* ぬるぽの壁にあたるのでエラーコードをセーブ */
		SETERR(info->readsize);
		return 0;
	}
	SETERR(0);
	return 1;
}

/** @brief データ記録の読取
 * 引数で指定したTYPE, 基準時刻、メンバー、対象時刻、面、要素のデータを
 * 読み出す。 
 * @retval 正 読み出して格納した格子数
 * @retval 0 指定したデータは未記録(定義ファイルの elementmap によって書き込まれることは許容されているが、まだデータが書き込まれていない)
 * @retval -2 指定したデータは記録することが許容されていない(elementmap によって禁止されている場合と指定した面名、要素名が登録されていない場合の両方を含む)。
 * @retval -4 格納配列が不足
 * @retval -5 格納配列の型とレコードの記録形式が不整合
 *
 * <H3> 注意 </H3>
 * nusdas_read では、返却値 0 はエラーであることに注意が必要。
 * nusdas_read のエラーチェックは返却値が求めている格子数と一致していること
 * を確認するのが望ましい。
 * <H3> 互換性 </H3>
 * NuSDaS1.1 では「ランレングス圧縮で、データが指定最大値を超えている」
 * (返却値-6)が定義されていたが、はデータの最初だけを
 * 見ているだけで意味がないと思われるので、NuSDaS1.3 ではこのエラーは
 * 返さない。また、「ユーザーオープンファイルの管理部又はアドレス部が不正
 * である」(返却値-7)は、共通部分の-54〜-57に対応するので、このエラーは返さない
*/
	N_SI4
NuSDaS_read(
		const char utype1[8], /**< 種別1 */
		const char utype2[4], /**< 種別2 */
		const char utype3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		const char plane[6], /**< 面の名前 */
		const char element[6], /**< 要素名 */
		void *data, /**< INTENT(OUT) 結果格納配列 */
		const char fmt[2], /**< 結果格納配列の型 */
		const N_SI4 *size) /**< 結果格納配列の要素数 */
{
	struct read_dsselect_info info;
	nustype_t	type;
	int		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(utype1, utype2, utype3, &type);
	pack2nusdims(*basetime, member, *validtime, 1, plane, NULL,
			element, &info.nusdims);
	nus_debug(("--- NuSDaS_read %#ys/%#ms", &type, &info.nusdims));
	info.buf.ib_ptr = data;
	info.buf.ib_fmt = mem2sym4(fmt);
	info.buf.nelems = *size;
	cut_rectangle_disable(&info.buf.ib_cut);
	r = nusglb_dsscan_nustype(read_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		NUSDAS_CLEANUP;
		return info.readsize;
	} else {
		NUSDAS_CLEANUP;
		return NUS_ERR_CODE();
	}
}

/** @brief データ記録の読取
 */
	N_SI4
NuSDaS_read2(
		const char utype1[8], /**< 種別1 */
		const char utype2[4], /**< 種別2 */
		const char utype3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		const char plane1[6], /**< 面の名前1 */
		const char plane2[6], /**< 面の名前2 */
		const char element[6], /**< 要素名 */
		void *data, /**< INTENT(OUT) 結果格納配列 */
		const char fmt[2], /**< 結果格納配列の型 */
		const N_SI4 *size) /**< 結果格納配列の要素数 */
{
	struct read_dsselect_info info;
	nustype_t	type;
	int		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- NuSDaS_read2"));
	pack2nustype(utype1, utype2, utype3, &type);
	pack2nusdims(*basetime, member, *validtime1, *validtime2,
			plane1, plane2,
			element, &(info.nusdims));
	info.buf.ib_ptr = data;
	info.buf.ib_fmt = mem2sym4(fmt);
	info.buf.nelems = *size;
	cut_rectangle_disable(&info.buf.ib_cut);
	r = nusglb_dsscan_nustype(read_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	if (r > 0) {
		return info.readsize;
	} else {
		return NUS_ERR_CODE();
	}
}
