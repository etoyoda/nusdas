/** @file
 * @brief nusdas_subc_delt() の実装
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
#include <stddef.h>
# define NEED_MEMCPY_NTOH4
# define NEED_MEMCPY_HTON4
# define NEED_POKE_FLOAT
#include "sys_endian.h"
#include <string.h>
#include "sys_time.h"
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

/** @brief ZHYB 型 SUBC を読み取るために受け渡す情報 */
struct subcdelt_get_param {
	const nusdims_t *dims;
	float delt;
};

	static int
subcdelt_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct subcdelt_get_param *param = vparam;
	N_UI4	dummy;
	memcpy_ntoh4(&dummy, rec, 1);
	memcpy_ntoh4(&param->delt, rec, 1);
	if (~dummy == 0) {
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	if (siz != 4) {
		return nus_err((NUSERR_SC_SizeMismatch, "Broken SUBC/DELT size"));
	}
	return 0;
}

	static int
subcdelt_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct subcdelt_get_param *param = vparam;
	int r;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, SYM4_DELT,
			subcdelt_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* みつからないので探索続行 */
		return 0;
	}
	return 1;
}

	static N_SI4
subcdelt_get(nustype_t *type,
		struct subcdelt_get_param *param)
{
	int r;
	r = nusglb_dsscan_nustype(subcdelt_get_dsselect, type, param);
	if (r > 0) {
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

/** @brief ZHYB 型 SUBC を書き出すために受け渡す情報 */
struct subcdelt_put_param {
	float	delt;
};

	static int
subcdelt_put_encode(void *rec, N_UI4 siz UNUSED, void *arg,
		union nusdset_t *ds UNUSED)
{
	struct subcdelt_put_param *param = arg;
	N_UI1 *buf = rec;
	POKE_float(buf, param->delt);
	return 0;
}


/** @brief ZHYB 型 SUBC 書き込みの実働 */
	static int
subcdelt_put(nustype_t *type,
		nusdims_t *dims,
		struct subcdelt_put_param *param)
{
	union nusdset_t *ds;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, dims, SYM4_SUBC, SYM4_DELT, 4,
			subcdelt_put_encode, param);
}

/** @brief SUBC DELT へのアクセス */
	N_SI4
NuSDaS_subc_delt2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		float *delt, /**< DELT 数値へのポインタ */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
{
	nustype_t	type;
	nusdims_t	dims;
	sym4_t		op;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	op = str3sym4upcase(getput);
	if (op == SYM4_GET) {
		struct subcdelt_get_param param;
		param.dims = &dims;
		r = subcdelt_get(&type, &param);
		if (r == 0) nuserr_cancel(MARK_FOR_DSET);
		*delt = param.delt;
	} else if (op == SYM4_PUT) {
		struct subcdelt_put_param param;
		param.delt = *delt;
		r = subcdelt_put(&type, &dims, &param);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC DELT へのアクセス 
 * モデルの時間積分間隔を補助管理情報に記録しておくものである。
 * @retval 0 正常終了
 * @retval -2 レコードが存在しない、または書き込まれていない。
 * @retval -3 レコードサイズが不正
 * @retval -5 入出力指示が不正
 * <H3> 履歴 </H3>
 * この関数は NuSDaS1.2で導入された。
*/
	N_SI4
NuSDaS_subc_delt(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		float *delt, /**< DELT 数値へのポインタ */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4	unity = 1;
	return nusdas_subc_delt2(type1, type2, type3,
			basetime, member, validtime, &unity,
			delt, getput);
}

/** @brief SUBC DELT のデフォルト設定
 * ファイルが新たに生成される際にDELTレコードに書き込む値を設定する。
 * DELT レコードや引数についてはnusdas_subc_delt を参照。
 * @retval 0 正常終了
 * @retval -1 定義ファイルに "DELT" が登録されていない
 * @retval -2 メモリの確保に失敗した
 * <H3> 互換性 </H3>
 * NuSDaS1.1 では、一つのNuSDaSデータセットに設定できる補助管理部の数は最大
 * 10 に制限されており、それを超えると-2が返された。一方、NuSDaS1.3 では
 * メモリが確保できる限り数に制限はなく、-2 をメモリ確保失敗のエラーコードに
 * 読み替えている。
 */
	N_SI4
NuSDaS_subc_delt_preset1(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const float *delt) /**< DELT 数値へのポインタ */
{
	struct subcdelt_put_param param;
	nustype_t nustype;
	nusdset_t *ds;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	param.delt = *delt;
	pack2nustype(type1, type2, type3, &nustype);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		r = nus_err((NUSERR_DsetNotFound, 
			     "missing dataset %Qs.%Ps.%Ps",
			     nustype.type1, nustype.type2,
			     nustype.type3));
		NUSDAS_CLEANUP;
		return r;
	}
	r = ds_subcpreset(ds, SYM4_DELT, 4,
			subcdelt_put_encode, &param);
	NUSDAS_CLEANUP;
	return r;
}

