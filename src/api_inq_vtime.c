#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include "sys_err.h"
#include "sys_time.h"
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "dset.h"
#include "glb.h"

/** @brief nusdas_inq_nrdvtime() から vtlist_dsselect() に渡される情報
 */
struct vtlist_dsselect_info {
	N_SI4           *buf;
	N_SI4           bufnelems;
	N_SI4           basetime;
	int             verbose;
};

/** 各データセットに対する vtlist 処理.
 * @todo 成功したら 1 を返して nusglb_dsscan() を止めること
 */
	static int
vtlist_dsselect(nusdset_t *ds, void *arg)
{
	struct vtlist_dsselect_info *info = arg;
	int r;

	r = ds_vtlist(ds, info->buf, info->bufnelems, info->basetime, 
		      info->verbose);
	nus_debug(("ds_vtlist => %d", r));
	if (r < 0) {
		/* ぬるぽの壁にあたるのでエラーコードをセーブ */
		SETERR(r);
		return 0;
	}
	return r;
}

/** @brief データセットの対象時刻リスト取得
 *
 * 種別1〜種別3で指示されるデータセットに基準時刻 @p basetime のもとで
 * 存在する対象時刻を配列 @p vtlist に書き込む。
 * 引数 @p pflag に非零値を設定すると動作過程の情報を警告メッセージとして
 * 印字するようになる。
 * @retval 非負 対象時刻の個数
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.0 から存在したがドキュメントされていなかった。
 * <H3>注意</H3>
 * <UL>
 * <LI>配列長 @p vtlistsize より多くの対象時刻が存在する場合は、
 * 配列長を越えて書き込むことはない。リターンコードと配列長を比較して、
 * リターンコードが大きかったらその数だけ配列を確保し直して
 * 本関数を呼び直すことにより、すべてのリストを得ることができる。
 * <LI>対象時刻の探索はファイルの有無または CNTL レコードによる。
 * リスト中の対象時刻についてデータレコードが書かれていない場合もありうる。
 * <LI>基準時刻 @p basetime に -1 を指定すると、
 * 基準時刻を問わない検索になる。
 * <LI>検索にあたってメンバー名は問わない。
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
 */
	N_SI4
NuSDaS_inq_nrdvtime(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		N_SI4 *vtlist, /**< INTENT(OUT) 対象時刻が書かれる配列 */
		const N_SI4 *vtlistsize, /**< 配列の要素数 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		N_SI4 pflag) /**< 動作詳細印字フラグ */
{
	nustype_t	type;
	struct vtlist_dsselect_info info;	
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	info.buf = vtlist;
	info.bufnelems = *vtlistsize;
	info.basetime = *basetime;
	info.verbose = pflag;
	r = nusglb_dsscan_nustype(vtlist_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	return r;
}

