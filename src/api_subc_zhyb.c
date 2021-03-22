/** @file
 * @brief nusdas_subc_zhyb() の実装
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <string.h>
#include <stddef.h>
# define NEED_MEMCPY_NTOH4
# define NEED_MEMCPY_HTON4
# define NEED_POKE_FLOAT
#include "sys_endian.h"
#include <ctype.h>
#include "sys_kwd.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
# define NEED_STR3SYM4UPCASE
#include "sys_sym.h"
#include "sys_time.h"
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

/** @brief ZHYB 型 SUBC を読み取るために受け渡す情報 */
struct subczhyb_get_param {
	const nusdims_t *dims;
	N_SI4	nz;
	float	ptrf;
	float	presrf;
	float	*zrp;
	float	*zrw;
	float	*vctrans_p;
	float	*vctrans_w;
	float	*dvtrans_p;
	float	*dvtrans_w;
};

/** @brief ZHYB 型 SUBC 読取 (対データファイルコールバック)
 * @note rec, siz は定義ファイル subcntl 文に書く長さに対応
 */
	static int
subczhyb_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct subczhyb_get_param *param = vparam;
	N_UI4	nlev;
	memcpy_ntoh4(&nlev, rec, 1);
	if (~nlev == 0) {
		/* 初期化されたまま書かれていない記録を読んだ場合 */
		nlev = param->nz;
		memset(&param->ptrf, 0xFF, sizeof(float));
		memset(&param->presrf, 0xFF, sizeof(float));
		memset(param->zrp, 0xFF, sizeof(float) * nlev);
		memset(param->zrw, 0xFF, sizeof(float) * nlev);
		memset(param->vctrans_p, 0xFF, sizeof(float) * nlev);
		memset(param->vctrans_w, 0xFF, sizeof(float) * nlev);
		memset(param->dvtrans_p, 0xFF, sizeof(float) * nlev);
		memset(param->dvtrans_w, 0xFF, sizeof(float) * nlev);
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	if (siz != (nlev * 24 + 12)) {
		return nus_err((NUSERR_SC_SizeMismatch, "Broken SUBC/ZHYB"));
	}
	if (nlev > (N_UI4)param->nz) {
		return nus_err((NUSERR_SC_BadArg,
					"SUBC levels %Pd > given buffer %Pd",
					nlev, param->nz));
	}
	param->nz = nlev;
	memcpy_ntoh4(&param->ptrf, rec + 4, 1);
	memcpy_ntoh4(&param->presrf, rec + 8, 1);
	memcpy_ntoh4(param->zrp, rec + 12, nlev);
	memcpy_ntoh4(param->zrw, rec + 12 + nlev * 4, nlev);
	memcpy_ntoh4(param->vctrans_p, rec + 12 + nlev * 4 * 2, nlev);
	memcpy_ntoh4(param->vctrans_w, rec + 12 + nlev * 4 * 3, nlev);
	memcpy_ntoh4(param->dvtrans_p, rec + 12 + nlev * 4 * 4, nlev);
	memcpy_ntoh4(param->dvtrans_w, rec + 12 + nlev * 4 * 5, nlev);
	return 0;
}

/** @brief ZHYB 型 SUBC 読取 (対データセットコールバック) */
	static int
subczhyb_get_dsselect(nusdset_t *ds, void *vparam)
{
	int	r;
	struct subczhyb_get_param *param = vparam;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, SYM4_ZHYB,
			subczhyb_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* 見付からなければ探索続行 */
		return 0;
	}
	return 1;
}

/** @brief ZHYB 型 SUBC 読取の実働 */
	static N_SI4
subczhyb_get(nustype_t *type,
		nusdims_t *dims,
		N_SI4 *nz,
		float *ptrf,
		float *presrf,
		float zrp[],
		float zrw[],
		float vctrans_p[],
		float vctrans_w[],
		float dvtrans_p[],
		float dvtrans_w[])
{
	struct subczhyb_get_param param;
	int	r;
	param.dims = dims;
	param.nz = *nz;
	param.zrp = zrp;
	param.zrw = zrw;
	param.vctrans_p = vctrans_p;
	param.vctrans_w = vctrans_w;
	param.dvtrans_p = dvtrans_p;
	param.dvtrans_w = dvtrans_w;
	r = nusglb_dsscan_nustype(subczhyb_get_dsselect, type, &param);
	if (r > 0) {
		*nz = param.nz;
		*ptrf = param.ptrf;
		*presrf = param.presrf;
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

/** @brief ZHYB 型 SUBC を書き出すために受け渡す情報 */
struct subczhyb_put_param {
	const nusdims_t *dims;
	N_SI4	nz;
	float	ptrf;
	float	presrf;
	const float	*zrp;
	const float	*zrw;
	const float	*vctrans_p;
	const float	*vctrans_w;
	const float	*dvtrans_p;
	const float	*dvtrans_w;
};

	static int
subczhyb_put_encode(void *rec, N_UI4 siz UNUSED, void *arg,
		union nusdset_t *ds UNUSED)
{
	struct subczhyb_put_param *param = arg;
	N_UI1 *buf = rec;
	POKE_N_UI4(buf, param->nz);
	POKE_float(buf + 4, param->ptrf);
	POKE_float(buf + 8, param->presrf);
	memcpy_hton4(buf + 12, param->zrp, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 4, param->zrw, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 8, param->vctrans_p, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 12, param->vctrans_w, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 16, param->dvtrans_p, param->nz);
	memcpy_hton4(buf + 12 + param->nz * 20, param->dvtrans_w, param->nz);
	return 0;
}

static int
subczhyb_put_arg_check(struct subczhyb_put_param *param)
{
	/* --- Checking input data --- */
	if ( param->nz < 1 ){
		return nus_err((NUSERR_SC_BadInput, 
				" Parameter nz must be larger than 1. "
				"Your input nz = %d",
				param->nz));
	} else if ( param->presrf <= 0.e0 ){
		return nus_err((NUSERR_SC_BadInput, 
				" Parameter presrf must be larger than 0. "
				"Your input presrf = %f",
				param->presrf));
	}
	return 0;
}


/** @brief ZHYB 型 SUBC 書き込みの実働 */
	static int
subczhyb_put(nustype_t *type,
		nusdims_t *dims,
		N_SI4 nz,
		float ptrf,
		float presrf,
		float zrp[],
		float zrw[],
		float vctrans_p[],
		float vctrans_w[],
		float dvtrans_p[],
		float dvtrans_w[])
{
	struct subczhyb_put_param param;
	union nusdset_t *ds;
	int r;
	param.nz = nz;
	param.ptrf = ptrf;
	param.presrf = presrf;
	param.zrp = zrp;
	param.zrw = zrw;
	param.vctrans_p = vctrans_p;
	param.vctrans_w = vctrans_w;
	param.dvtrans_p = dvtrans_p;
	param.dvtrans_w = dvtrans_w;
	if ((r = subczhyb_put_arg_check(&param)) != 0) {
		return r;
	}
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, dims, SYM4_SUBC, SYM4_ZHYB,
			nz * 24 + 12,
			subczhyb_put_encode, &param);
}

/** @brief SUBC ZHYB へのアクセス */
	N_SI4
NuSDaS_subc_zhyb2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		N_SI4 *nz, /**< 鉛直層数 */
		float *ptrf, /**< 温位の参照値 */
		float *presrf, /**< 気圧の参照値 */
		float zrp[], /**< モデル面高度 (フルレベル) */
		float zrw[], /**< モデル面高度 (ハーフレベル) */
		float vctrans_p[], /**< 座標変換関数 (フルレベル) */
		float vctrans_w[], /**< 座標変換関数 (ハーフレベル) */
		float dvtrans_p[], /**< 座標変換関数の鉛直微分 (フルレベル) */
		float dvtrans_w[], /**< 座標変換関数の鉛直微分 (ハーフレベル) */
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
		r = subczhyb_get(&type, &dims, nz, ptrf, presrf, zrp, zrw,
				vctrans_p, vctrans_w, dvtrans_p, dvtrans_w);
	} else if (op == SYM4_PUT) {
		r = subczhyb_put(&type, &dims, *nz, *ptrf, *presrf, zrp, zrw,
				vctrans_p, vctrans_w, dvtrans_p, dvtrans_w);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC ZHYB へのアクセス 
 * 鉛直座標に鉛直ハイブリッド座標をを使う場合の補助管理情報ZHYB
 * へのアクセスを提供する。
 * 入出力指示が @p GET の場合においても、nz の値はセットする。この nz の値は
 * nusdas_subc_eta_inq_nz を使って問い合わせできる。
 * zrp, zrw, vctrans_p, vctrans_w, dvtrans_p, dvtrans_w は 
 * nz 要素をもった配列を用意する。
 * @retval 0 正常終了
 * @retval -2 レコードが存在しない、または書き込まれていない。
 * @retval -3 サイズの情報が引数と定義ファイルで不一致
 * @retval -4 指定した入力値(ptrf, presrf)が不正(PUTのときのみ)
 * @retval -5 入出力指示が不正
 * @retval -6 指定した入力値(nz)が不正(GETのときのみ)
 * <H3> 注意 </H3>
 * SUBC のサイズは 24 * nz + 12 を計算した値を定義ファイルに書く。
 * <H3> 履歴 </H3>
 * この関数はNuSDaS1.2で実装された
*/
	N_SI4
NuSDaS_subc_zhyb(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		N_SI4 *nz, /**< 鉛直層数 */
		float *ptrf, /**< 温位の参照値 */
		float *presrf, /**< 気圧の参照値 */
		float zrp[], /**< モデル面高度 (フルレベル) */
		float zrw[], /**< モデル面高度 (ハーフレベル) */
		float vctrans_p[], /**< 座標変換関数 (フルレベル) */
		float vctrans_w[], /**< 座標変換関数 (ハーフレベル) */
		float dvtrans_p[], /**< 座標変換関数の鉛直微分 (フルレベル) */
		float dvtrans_w[],
		/**< 座標変換関数の鉛直微分 (ハーフレベル) */
		const char getput[3])
		/**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4	unity = 1;
	return nusdas_subc_zhyb2(type1, type2, type3,
			basetime, member, validtime, &unity,
			nz, ptrf, presrf, zrp, zrw,
			vctrans_p, vctrans_w,
			dvtrans_p, dvtrans_w,
			getput);
}

/** @brief SUBC ZHYB のデフォルト値を設定
 * ファイルが新たに生成される際にZHYBレコードに書き込む値を設定する。
 * ZHYB レコードや引数についてはnusdas_subc_zhyb を参照。
 * @retval 0 正常終了
 * @retval -1 定義ファイルに "ZHYB" が登録されていない
 * @retval -2 メモリの確保に失敗した
 * <H3> 互換性 </H3>
 * NuSDaS1.1 では、一つのNuSDaSデータセットに設定できる補助管理部の数は最大
 * 10 に制限されており、それを超えると-2が返された。一方、NuSDaS1.3 では
 * メモリが確保できる限り数に制限はなく、-2 をメモリ確保失敗のエラーコードに
 * 読み替えている。
*/
	N_SI4
NuSDaS_subc_zhyb_preset1(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *nz, /**< 鉛直層数 */
		const float *ptrf, /**< 温位の参照値 */
		const float *presrf, /**< 気圧の参照値 */
		const float zrp[], /**< モデル面高度 (フルレベル) */
		const float zrw[], /**< モデル面高度 (ハーフレベル) */
		const float vctrans_p[], /**< 座標変換関数 (フルレベル) */
		const float vctrans_w[], /**< 座標変換関数 (ハーフレベル) */
		const float dvtrans_p[],
		/**< 座標変換関数の鉛直微分 (フルレベル) */
		const float dvtrans_w[])
		/**< 座標変換関数の鉛直微分 (ハーフレベル) */
{
	struct subczhyb_put_param param;
	nustype_t nustype;
	nusdset_t *ds;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	param.nz = *nz;
	param.ptrf = *ptrf;
	param.presrf = *presrf;
	param.zrp = zrp;
	param.zrw = zrw;
	param.vctrans_p = vctrans_p;
	param.vctrans_w = vctrans_w;
	param.dvtrans_p = dvtrans_p;
	param.dvtrans_w = dvtrans_w;
	if ((r = subczhyb_put_arg_check(&param)) != 0) {
		NUSDAS_CLEANUP;
		/* for compatibility with NuSDaS1.2 */
		if (r == NUSERR_SC_BadInput) r = -3; 
		return r;
	}
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
	r = ds_subcpreset(ds, SYM4_ZHYB,
			*nz * 24 + 12,
			subczhyb_put_encode, &param);
	NUSDAS_CLEANUP;
	return r;
}
