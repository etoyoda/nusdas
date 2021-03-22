#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_time.h"
#include "sys_kwd.h"
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "glb.h"
#include <stddef.h>
#include "dset.h"
#include "sys_err.h"

/** @brief nusdas_inq_nrdbtime() から btlist_dsselect() に渡される情報
 */
struct btlist_dsselect_info {
	N_SI4           *buf;
	N_SI4           bufnelems;
	int             verbose;
};

/** 各データセットに対する btlist 処理.
 * @todo 成功したら 1 を返して nusglb_dsscan() を止めること
 */
	static int
btlist_dsselect(nusdset_t *ds, void *arg)
{
	struct btlist_dsselect_info *info = arg;
	int r;

	r = ds_btlist(ds, info->buf, info->bufnelems,
		      info->verbose);
	nus_debug(("ds_btlist => %d", r));
	if (r < 0) {
		/* ぬるぽの壁にあたるのでエラーコードをセーブ */
		SETERR(r);
		return 0;
	}
	return r;
}

/** @brief データセットの基準時刻リスト取得
 *
 * 種別1〜種別3で指示されるデータセットに存在する基準時刻を
 * 配列 @p btlist に書き込む。
 * 引数 @p pflag に非零値を設定すると動作過程の情報を警告メッセージとして
 * 印字するようになる。
 * @retval 非負 基準時刻の個数
 * @retval -1 ファイル I/O エラー
 * @retval -2 ファイルに管理部が存在しない
 * @retval -3 ファイルのレコード長が不正
 * @retval -4 ファイルあるいはディレクトリのオープンに失敗
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.0 から存在した。
 * <H3>注意</H3>
 * <UL>
 * <LI>
 * 配列長 @p btlistsize より多くの基準時刻が存在する場合は、
 * 配列長を越えて書き込むことはない。リターンコードと配列長を比較して、
 * リターンコードが大きかったらその数だけ配列を確保し直して
 * 本関数を呼び直すことにより、すべてのリストを得ることができる。
 * <LI>
 * NuSDaS 1.1 までは見付かったデータセットがネットワークでなければ、
 * それについてだけ探索が行われた。
 * NuSDaS 1.3 からは、
 * 指定した種別にマッチするすべてのデータセットについて探索が行われる。
 * <LI>
 * 種別に対応するデータセットが見つからない場合
 * (たとえば種別名を間違えた場合)、
 * 返却値はゼロとなる。
 * データセットが存在して空の場合と異なり、
 * このとき ``Can not find NUSDAS root directory for selected type1-3''
 * ``type1$<$...$>$ type2$<$...$>$ type3$<$...$>$ NRD=...''
 * というメッセージが標準エラー出力に表示される。
 * NRD= の後の数値が -1 でなければ、
 * NRD 番号を指定したために存在しているデータセットが見つからなくなっている
 * 可能性がある。
 * </UL>
 * */
	N_SI4
NuSDaS_inq_nrdbtime(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		N_SI4 *btlist, /**< INTENT(OUT) 基準時刻が格納される配列 */
		const N_SI4 *btlistsize, /**< 配列の要素数 */
		N_SI4 pflag) /**< 動作過程印字フラグ */
{
	nustype_t	type;
	N_SI4		r;
	struct btlist_dsselect_info info;

	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	info.buf = btlist;
	info.bufnelems = *btlistsize;
	info.verbose = pflag;
	r = nusglb_dsscan_nustype(btlist_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
	}
	NUSDAS_CLEANUP;
	return r;
}
