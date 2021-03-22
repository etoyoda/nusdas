#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sys_kwd.h"
# define NEED_MEM2SYM4
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSDIMS
#include "sys_sym.h"
#include "sys_err.h"
#include "sys_time.h"
#include "dfile.h"
#include "dset.h"
#include "glb.h"

struct cut_dsselect_info {
	nusdims_t	nusdims;
	struct ibuffer_t buf;
	int		readsize;
};

	static int
cut_dsselect(nusdset_t *ds, void *arg)
{
	struct cut_dsselect_info *info = arg;
	info->readsize = ds_readdata(ds, &info->nusdims, &info->buf);
	nus_debug(("ds_readdata => %d", info->readsize));
	if (info->readsize < 0) {
		/* エラーコードを伝えられないのでセーブしておく */
		SETERR(info->readsize);
		return 0;
	}
	/* 読めた */
	return 1;
}

/** @brief 領域限定のデータ読取 */
	N_SI4
NuSDaS_cut2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1 */
		const N_SI4 *validtime2, /**< 対象時刻2 */
		const char plane1[6], /**< 面1 */
		const char plane2[6], /**< 面2 */
		const char element[6], /**< 要素名 */
		void *udata, /**< INTENT(OUT) データ格納配列 */
		const char utype[2], /**< データ格納配列の型 */
		const N_SI4 *usize, /**< データ格納配列の要素数 */
		const N_SI4 *ixstart, /**< $x$ 方向格子番号下限 */
		const N_SI4 *ixfinal, /**< $x$ 方向格子番号上限 */
		const N_SI4 *iystart, /**< $y$ 方向格子番号下限 */
		const N_SI4 *iyfinal) /**< $y$ 方向格子番号上限 */
{
	struct cut_dsselect_info info;
	nustype_t	type;
	int		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusdims(*basetime, member, *validtime1, *validtime2,
			plane1, plane2,
			element, &info.nusdims);
	nus_debug(("--- nusdas_cut %#ys/%#ms", &type, &info.nusdims));
	info.buf.ib_ptr = udata;
	info.buf.ib_fmt = mem2sym4(utype);
	info.buf.nelems = *usize;
	if (*ixstart <= 0 || *iystart <= 0
			|| *ixfinal < *ixstart || *iyfinal < *iystart) {
		cut_rectangle_disable(&info.buf.ib_cut);
	} else {
		info.buf.ib_cut.cr_xofs = *ixstart - 1;
		info.buf.ib_cut.cr_yofs = *iystart - 1;
		info.buf.ib_cut.cr_xnelems = *ixfinal - *ixstart + 1;
		info.buf.ib_cut.cr_ynelems = *iyfinal - *iystart + 1;
		if (info.buf.nelems < (unsigned)cut_rectangle_size(&info.buf.ib_cut)) {
			r =  nus_err((NUSERR_RD_SmallBuf,
				      "buffer %Pu < %u elements required",
				      info.buf.nelems,
				      cut_rectangle_size(&info.buf.ib_cut)));
			NUSPROF_MARK(NP_USER);
			NUSDAS_CLEANUP;
			return r;
		}
	}
	r = nusglb_dsscan_nustype(cut_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0)
		nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	if (r > 0) {
		return info.readsize;
	} else {
		return NUS_ERR_CODE();
	}
}

/** @brief 領域限定のデータ読取
 *
 * nusdas_read() * と同様だが、データレコードのうち格子点
 * (@p ixstart , @p iystart )--(@p ixfinal , @p iyfinal )
 * だけが @p udata に格納される。
 *
 * 格子番号は 1 から始まるものとするため、
 * @p ixstart や @p iystart は正でなければならず、また
 * @p ixfinal や @p iyfinal はそれぞれ
 * @p ixstart や @p iystart 以上でなければならない。
 * この規則に反する指定を行った場合は、返却値-8のエラーとなる。
 * なお、@p iyfinal, @p jyfinal の上限が格子数を超えていることの
 * チェックはしていないので注意が必要。
 *
 * @retval 正 読み出して格納した格子数
 * @retval 0 指定したデータは未記録(定義ファイルの elementmap によって書き込まれることは許容されているが、まだデータが書き込まれていない)
 * @retval -2 指定したデータは記録することが許容されていない(elementmap によって禁止されている場合と指定した面名、要素名が登録されていない場合の両方を含む)。
 * @retval -4 格納配列が不足
 * @retval -5 格納配列の型とレコードの記録形式が不整合
 * @retval -8 領域指定パラメータが不正
 *
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.1 で導入され、NuSDaS 1.3 で初めてドキュメントされた。
 * <H3>互換性</H3>
 * NuSDaS 1.1 では、ローカルのデータファイルに対しては、
 * @p ixstart ≦ 0 の場合は @p ixstart = 1 に(@p jystart も同様), 
 * @p ixfinal がX方向の格子数を超える場合には、@p ixfinal はX方向の格子数に
 * (@p jyfinal も同様)に読み替えられていたが、NuSDaS1.3 では返却値-8のエラー
 * とする。また、pandora データについては、@p ixstart, @p ixfinal, 
 * @p jystart, @p jyfinal が非負であることだけがチェックされていた。
 * NuSDaS1.3 ではデータファイル、pandora とも上述の通りとなる。
 * */
	N_SI4
NuSDaS_cut(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		const char plane[6], /**< 面 */
		const char element[6], /**< 要素名 */
		void *udata, /**< INTENT(OUT) データ格納先配列 */
		const char utype[2], /**< データ格納先配列の型 */
		const N_SI4 *usize, /**< データ格納先配列の要素数 */
		const N_SI4 *ixstart, /**< $x$ 方向格子番号下限 */
		const N_SI4 *ixfinal, /**< $x$ 方向格子番号上限 */
		const N_SI4 *iystart, /**< $y$ 方向格子番号下限 */
		const N_SI4 *iyfinal) /**< $y$ 方向格子番号上限 */
{
	struct cut_dsselect_info info;
	nustype_t	type;
	int		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusdims(*basetime, member, *validtime, 1, plane, NULL,
			element, &info.nusdims);
	nus_debug(("--- nusdas_cut %#ys/%#ms", &type, &info.nusdims));
	info.buf.ib_ptr = udata;
	info.buf.ib_fmt = mem2sym4(utype);
	info.buf.nelems = *usize;
	if (*ixstart <= 0 || *iystart <= 0
			|| *ixfinal < *ixstart || *iyfinal < *iystart) {
		r = nus_err((NUSERR_RD_BadCutRegion, 
			     "Invalid cut region"));
		NUSDAS_CLEANUP;
		return r;
	} else {
		info.buf.ib_cut.cr_xofs = *ixstart - 1;
		info.buf.ib_cut.cr_yofs = *iystart - 1;
		info.buf.ib_cut.cr_xnelems = *ixfinal - *ixstart + 1;
		info.buf.ib_cut.cr_ynelems = *iyfinal - *iystart + 1;
		if (info.buf.nelems < (unsigned)cut_rectangle_size(&info.buf.ib_cut)) {
			r = nus_err((NUSERR_RD_SmallBuf,
				     "buffer %Pu < %u elements required",
				     info.buf.nelems,
				     cut_rectangle_size(&info.buf.ib_cut)));
			NUSPROF_MARK(NP_USER);
			NUSDAS_CLEANUP;
			return r;
		}
	}
	r = nusglb_dsscan_nustype(cut_dsselect, &type, &info);
	NUSPROF_MARK(NP_USER);
	if (r > 0)
		nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	if (r > 0) {
		return info.readsize;
	} else {
		return NUS_ERR_CODE();
	}
}
