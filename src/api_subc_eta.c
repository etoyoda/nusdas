/** @file
 * @brief 数値予報系 nusdas_subc_* の実装
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_kwd.h"
#include "sys_time.h"
#include <stddef.h>
#include "glb.h"
#include "dset.h"
#include "sys_err.h"
#include <string.h>
#include <ctype.h>
# define NEED_STR3SYM4UPCASE
# define NEED_PACK2NUSBMV
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
# define NEED_POKE_FLOAT
# define NEED_MEMCPY_NTOH4
#include "sys_endian.h"

struct sigmeta_get_param {
	sym4_t	group;
	const nusdims_t *dims;
	N_SI4	*n_levels;
	float	*a;
	float	*b;
	float	*c;
};

/** @brief ETA/SIGM 型 SUBC 読取 (対データファイルコールバック)
 * @note rec, siz は定義ファイル subcntl 文に書く長さに相当する部分
 */
	static int
sigmeta_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char	*rec = vrec;
	struct sigmeta_get_param *param = vparam;
	N_UI4	nlev;
	memcpy_ntoh4(&nlev, rec, 1);
	if (~nlev == 0) {
		/* 記録長と記録内の鉛直総数が不整合
		 * 0xFF で memset された SUBC を読んだ時を想定
		 */
		memset(param->a, 0xFF, sizeof(float) * (nlev + 1));
		memset(param->b, 0xFF, sizeof(float) * (nlev + 1));
		memset(param->c, 0xFF, sizeof(float));
		return nus_err((NUSERR_SC_Uninitialized,
			"Uninitialized SUBC %Ps",
			param->group));
	}
	if (siz != ((nlev + 1) * 8 + 8)) {
		return nus_err((NUSERR_SC_SizeMismatch,
			"Broken SUBC siz %Pu != expected %Pu",
			siz, (nlev + 1) * 8 + 8));
	}
	if (nlev > (N_UI4)*param->n_levels) {
		return nus_err((NUSERR_SC_ShortBuf,
			"SUBC levels %Pd > your buffer %Pd",
			nlev, *param->n_levels));
	}
	*param->n_levels = nlev;
	memcpy_ntoh4(param->a, rec + 4, nlev + 1);
	memcpy_ntoh4(param->b, rec + 4 + (nlev  + 1) * 4, nlev + 1);
	memcpy_ntoh4(param->c, rec + 4 + (nlev  + 1) * 8, 1);
	return 0;
}

/** @brief ETA/SIGM 型 SUBC 読取 (対データセットコールバック) */
	static int
sigmeta_get_dsselect(nusdset_t *ds, void *vparam)
{
	int r;
	struct sigmeta_get_param *param = vparam;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, param->group,
			sigmeta_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* 見付からなければ探索続行 */
		return 0;
	}
	/* 見付かればこのデータセットで終わり */
	return 1;
}

/** @brief ETA/SIGM 型 SUBC 読取の実働部隊 */
	static int
sigmeta_get(nustype_t *type, const nusdims_t *dims, sym4_t group, 
		N_SI4 *n_levels, float *a, float *b, float *c)
{
	struct sigmeta_get_param param;
	int	r;
	param.group = group;
	param.dims = dims;
	param.n_levels = n_levels;
	param.a = a;
	param.b = b;
	param.c = c;
	r = nusglb_dsscan_nustype(sigmeta_get_dsselect, type, &param);
	nus_debug(("nusglb_dsscan_nustype => %d", r));
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		switch (r) {
			case 0:
			case NUSERR_OpenRFailed:
				r = NUSERR_NoDfileToRead;
			default:
				/* do nothing */;
		}
		return r;
	}
}

struct sigmeta_put_param {
	N_UI4	n_levels;
	const float *a;
	const float *b;
	float c;
};

	static int
sigmeta_put_encode(void *rec, N_UI4 siz UNUSED, void *arg,
		union nusdset_t *ds UNUSED)
{
	struct sigmeta_put_param *param = arg;
	N_UI1 *buf = rec;
	unsigned i;
	float *xa, *xb;
	xa = (float *)(buf + 4);
	xb = xa + (param->n_levels + 1);
	POKE_N_UI4(buf, param->n_levels);
	for (i = 0; i < param->n_levels + 1; i++) {
		POKE_float(&xa[i], param->a[i]);
		POKE_float(&xb[i], param->b[i]);
	}
	POKE_float(&xb[param->n_levels + 1], param->c);
	return 0;
}

/** @brief ETA/SIGM 型 SUBC 書き込みの実働部隊 */
	static int
sigmeta_put(nustype_t *type, nusdims_t *dims, sym4_t group,
		N_UI4 n_levels, const float *a, const float *b, float c)
{
	union nusdset_t *ds;
	struct sigmeta_put_param param;
	param.n_levels = n_levels;
	param.a = a;
	param.b = b;
	param.c = c;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, dims, SYM4_SUBC, group, (n_levels + 2) * 8,
		sigmeta_put_encode, &param);
}

/** @brief SUBC ETA へのアクセス */
	N_SI4
NuSDaS_subc_eta2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		N_SI4 *n_levels, /**< 鉛直層数 */
		float a[], /**< 係数 a */
		float b[], /**< 係数 b */
		float *c, /**< 係数 c */
		const char getput[3])
		/**< 入出力指示 (@p "GET" または @p "PUT") */
{
	nustype_t	type;
	nusdims_t	dims;
	sym4_t		op;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_subc_eta2"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	op = str3sym4upcase(getput);
	if (op == SYM4_GET) {
		r = sigmeta_get(&type, &dims, SYM4_ETA,
				n_levels, a, b, c);
	} else if (op == SYM4_PUT) {
		r = sigmeta_put(&type, &dims, SYM4_ETA,
				*n_levels, a, b, *c);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC ETA へのアクセス
 * 鉛直座標に ETA 座標系を用いるときに、鉛直座標を定めるパラメータへの
 * アクセスを提供する。 
 * パラメータは4バイト単精度浮動小数点型の配列@p a, @p b, @p c で構成され、
 * @p a, @p b, は鉛直層数 @p n_levels に対して、@p n_levels+1 要素の配列、
 * @p c は1要素の配列(変数)を確保する必要がある。
 * n_levels は nusdas_subc_inq_nz で問い合わせることができる。
 * @retval 0 正常終了
 * @retval -2 レコードが存在しない、またはレコードの書き込みがされていない。
 * @retval -3 レコードサイズが不正
 * @retval -4 ユーザーの鉛直層数がファイルの中の鉛直層数より小さい
 * @retval -5 入出力指示が不正。
 * <H3> 履歴 </H3>
 * この関数は NuSDaS1.0 から存在した。
 * NuSDaS1.1までは、レコードが書き込まれたかの情報を持ち合わせていなかった
 * ために無記録のレコードをファイルから読んで正常終了していた。NuSDaS1.3では
 * ファイルの初期化時にレコードを初期化し、未記録を判定できるようにした。
 * その場合のエラーは-2としている。
 * <H3> 注意 </H3>
 * SUBC ETA に使われている鉛直層数 @p n_levels は実際のモデルの鉛直層数と
 * 異なっている場合があるので、配列確保の際にはnusdas_subc_inq_nzで問い
 * 合わせた結果を用いること。
 *
*/
	N_SI4
NuSDaS_subc_eta(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		N_SI4 *n_levels, /**< 鉛直層数 */
		float a[], /**< 係数 a */
		float b[], /**< 係数 b */
		float *c, /**< 係数 c */
		const char getput[3])
		/**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4	unity = 1;
	return NuSDaS_subc_eta2(type1, type2, type3, basetime,
			member, validtime, &unity, n_levels, a, b, c,
			getput);
}

/** @brief SUBC ETA/SIGM のデフォルト値設定 
 * ファイルが新たに生成される際にETA, SIGMに書き込む値を設定する。
 * SIGM や引数については nusdas_subc_eta を参照。
 * 引数の「群名」には、"ETA " または "SIGM" を指定する。
 * @retval 0 正常終了
 * @retval -1 定義ファイルに指定した群名が登録されていない
 * @retval -2 メモリの確保に失敗した
 * @retval -3 レコードのサイズが不正
 *
 * <H3> 互換性 </H3>
 * NuSDaS1.1 では、一つのNuSDaSデータセットに設定できる補助管理部の数は最大
 * 10 に制限されており、それを超えると-2が返された。一方、NuSDaS1.3 では
 * メモリが確保できる限り数に制限はなく、-2 をメモリ確保失敗のエラーコードに
 * 読み替えている。
*/
	N_SI4
NuSDaS_subc_preset1(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const char group[4], /**< 群名 */
		const N_SI4 *n_levels, /**< 鉛直層数 */
		float a[], /**< 係数 a */
		float b[], /**< 係数 b */
		float *c) /**< 係数 c */
{
	struct sigmeta_put_param param;
	nustype_t nustype;
	nusdset_t *ds;
	sym4_t grp;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	param.n_levels = *n_levels;
	param.a = a;
	param.b = b;
	param.c = *c;
	pack2nustype(type1, type2, type3, &nustype);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		r = nus_err((NUSERR_DsetNotFound, 
			     "missing dataset %Qs.%Ps.%Ps",
			     nustype.type1, nustype.type2, nustype.type3));
		NUSDAS_CLEANUP;
		return r;
	}
	grp = str2sym4(group);
	r = ds_subcpreset(ds, grp,
			*n_levels * 8 + 16,
			sigmeta_put_encode, &param);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC SIGM へのアクセス
 * 鉛直座標に ETA 座標系を用いるときに、鉛直座標を定めるパラメータへの
 * アクセスを提供する。 
 * 関数の仕様は、nusdas_subc_eta と同じである。
 */
	N_SI4
NuSDaS_subc_sigm(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		N_SI4 *n_levels, /**< 鉛直層数 */
		float a[], /**< 係数 a */
		float b[], /**< 係数 b */
		float *c, /**< 係数 c */
		const char getput[3])
		/**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4	unity = 1;
	return NuSDaS_subc_sigm2(type1, type2, type3, basetime,
			member, validtime, &unity, n_levels, a, b, c,
			getput);
}

/** @brief SUBC SIGM へのアクセス */
	N_SI4
NuSDaS_subc_sigm2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		N_SI4 *n_levels, /**< 鉛直層数 */
		float a[], /**< 係数 a */
		float b[], /**< 係数 b */
		float *c, /**< 係数 c */
		const char getput[3])
		/**< 入出力指示 (@p "GET" または @p "PUT") */
{
	nustype_t	type;
	nusdims_t	dims;
	sym4_t		op;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_subc_sigm2"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	op = str3sym4upcase(getput);
	if (op == SYM4_GET) {
		r = sigmeta_get(&type, &dims, SYM4_SIGM,
				n_levels, a, b, c);
	} else if (op == SYM4_PUT) {
		r = sigmeta_put(&type, &dims, SYM4_SIGM,
				*n_levels, a, b, *c);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}
