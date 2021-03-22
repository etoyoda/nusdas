/** @file
 * @brief nusdas_subc_rgau() の実装
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
#include "sys_endian.h"
#include <string.h>
#include "sys_time.h"
#include "sys_err.h"
#include "dset.h"
#include "glb.h"

/** @brief RGAU 型 SUBC を読み取るために受け渡す情報 */
struct subcrgau_get_param {
	const nusdims_t *dims;
	N_SI4	*j;
	N_SI4	*j_start;
	N_SI4	*j_n;
	N_SI4	*i;
	N_SI4	*i_start;
	N_SI4	*i_n;
	float	*lat;
};

	static int
subcrgau_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct subcrgau_get_param *param = vparam;
	N_UI4	nj;
	memcpy_ntoh4(&nj, rec + 8, 1);
	if (~nj == 0) {
		nj = *param->j_n;
		memset(param->j, 0xFF, sizeof(float));
		memset(param->j_start, 0xFF, sizeof(float));
		memset(param->i, 0xFF, sizeof(float) * nj);
		memset(param->i_start, 0xFF, sizeof(float) * nj);
		memset(param->i_n, 0xFF, sizeof(float) * nj);
		memset(param->lat, 0xFF, sizeof(float) * nj);
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	if (siz != (nj * 16 + 12)) {
		return nus_err((NUSERR_SC_SizeMismatch, "Broken SUBC/RGAU"));
	}
	if (nj > (N_UI4)(*param->j_n)) {
		return nus_err((NUSERR_SC_BadArg,
					"SUBC nj %Pd > given buffer %Pd",
					nj, param->j_n));
	}
	memcpy_ntoh4(param->j, rec + 0, 1);
	memcpy_ntoh4(param->j_start, rec + 4, 1);
	*param->j_n = nj;
	memcpy_ntoh4(param->i, rec + 12, nj);
	memcpy_ntoh4(param->i_start, rec + 12 + nj * 4, nj);
	memcpy_ntoh4(param->i_n, rec + 12 + nj * 8, nj);
	memcpy_ntoh4(param->lat, rec + 12 + nj * 12, nj);
	return 0;
}

	static int
subcrgau_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct subcrgau_get_param *param = vparam;
	int r;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, SYM4_RGAU,
			subcrgau_get_decode, vparam);
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
subcrgau_get(nustype_t *type,
		struct subcrgau_get_param *param)
{
	int r;
	r = nusglb_dsscan_nustype(subcrgau_get_dsselect, type, param);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

/** @brief RGAU 型 SUBC を書き出すために受け渡す情報 */
struct subcrgau_put_param {
	const nusdims_t *dims;
	N_SI4	j;
	N_SI4	j_start;
	N_SI4	j_n;
	const N_SI4	*i;
	const N_SI4	*i_start;
	const N_SI4	*i_n;
	const float	*lat;
};

	static int
subcrgau_put_encode(void *rec, N_UI4 siz UNUSED, void *arg,
		union nusdset_t *ds UNUSED)
{
	struct subcrgau_put_param *param = arg;
	N_UI1 *buf = rec;
	POKE_N_UI4(buf, param->j);
	POKE_N_UI4(buf + 4, param->j_start);
	POKE_N_UI4(buf + 8, param->j_n);
	memcpy_hton4(buf + 12, param->i, param->j_n);
	memcpy_hton4(buf + 12 + param->j_n * 4, param->i_start, param->j_n);
	memcpy_hton4(buf + 12 + param->j_n * 8, param->i_n, param->j_n);
	memcpy_hton4(buf + 12 + param->j_n * 12, param->lat, param->j_n);
	return 0;
}

static int
subcrgau_put_arg_check(struct subcrgau_put_param *param)
{
	int counter;

	/* --- Checking input data --- */
	if ( param->j < 1 ) {
		return nus_err((NUSERR_SC_BadInput, 
				" Parameter j must be larger than 1. "
				"Your input j = %d", param->j));
	} else if ((param->j_start > param->j) || param->j_start < 1 ) {
		return  nus_err((NUSERR_SC_BadInput, 
				 " Parameter j_start must be smaller than j "
				 "and larger than 1. "
				 "Your input j_start = %d, j = %d",
				 param->j_start, param->j));
	} else if ((param->j_n > ( param->j - (param->j_start) + 1 )) 
		   || param->j_n < 1) {
		return  nus_err((NUSERR_SC_BadInput, 
				 " Parameter j_n must be smaller than "
				 "j - j_start and larger than 1. "
				 "Your input j_n = %d, j = %d, j_start = %d.",
				 param->j_n, param->j, param->j_start));
	} else {
		for(counter = 0; counter < param->j_n ; counter++){
			if ( param->i[counter] < 1 ) {
				return nus_err((NUSERR_SC_BadInput, 
						" Parameter i must be larger "
						"than 1. "
						"Your input i[%d] = %d",
						counter, param->i[counter]));
			}
			if ((param->i_start[counter] > param->i[counter]) 
			    || (param->i_start[counter] < 1)) {
				return nus_err((NUSERR_SC_BadInput, 
						" Parameter i_start must be "
						"smaller than i and larger "
						"than 1. Your input "
						"i_start[%d] = %d, i[%d] = %d",
						counter, 
						param->i_start[counter], 
						counter, 
						param->i[counter]));
			}
			if ((param->i_n[counter] > 
			     (param->i[counter] - param->i_start[counter] + 1)) 
			    || (param->i_n[counter] < 1)) {
				return nus_err((NUSERR_SC_BadInput, 
						"Parameter i_n must be "
						"smaller than i - i_start "
						"and larger than 1. "
						"Your input i_n[%d] = %d, "
						"i[%d] = %d, i_start[%d] = %d.",
						counter, 
						param->i_n[counter], 
						counter, 
						param->i[counter], 
						counter, 
						param->i_start[counter]));
			}
		}
	}
	return 0;
}


/** @brief RGAU 型 SUBC 書き込みの実働 */
	static int
subcrgau_put(nustype_t *type,
		nusdims_t *dims,
		struct subcrgau_put_param *param)
{
	union nusdset_t *ds;
	int r;

	if ((r = subcrgau_put_arg_check(param)) != 0) {
		return r;
	}
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, dims, SYM4_SUBC, SYM4_RGAU,
			param->j_n * 16 + 12,
			subcrgau_put_encode, param);
}

/** @brief SUBC RGAU へのアクセス */
	N_SI4
NuSDaS_subc_rgau2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		N_SI4 *j, /**< 全球の南北分割数 */
		N_SI4 *j_start, /**< データの最北格子の番号(1始まり) */
		N_SI4 *j_n, /**< データの南北格子数 */
		N_SI4 i[], /**< 全球の東西格子数 */
		N_SI4 i_start[], /**< データの最西格子の番号(1始まり) */
		N_SI4 i_n[], /**< データの東西格子数 */
		float lat[], /**< 緯度 */
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
		struct subcrgau_get_param param;
		param.dims = &dims;
		param.j = j;
		param.j_start = j_start;
		param.j_n = j_n;
		param.i = i;
		param.i_start = i_start;
		param.i_n = i_n;
		param.lat = lat;
		r = subcrgau_get(&type, &param);
	} else if (op == SYM4_PUT) {
		struct subcrgau_put_param param;
		param.j = *j;
		param.j_start = *j_start;
		param.j_n = *j_n;
		param.i = i;
		param.i_start = i_start;
		param.i_n = i_n;
		param.lat = lat;
		r = subcrgau_put(&type, &dims, &param);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC RGAU へのアクセス 
 * Reduced Gauss 格子を使う場合の補助管理情報へのアクセスを提供する。
 * 入出力指示が @p GET の場合においても、j_n の値はセットする。この j_n の値は
 * nusdas_subc_rgau_inq_jn を使って問い合わせできる。
 * i, i_start, i_n, lat は j_n 要素をもった配列を用意する。
 * @retval 0 正常終了
 * @retval -2 レコードが存在しない、または書き込まれていない。
 * @retval -3 サイズの情報が引数と定義ファイルで不一致
 * @retval -4 指定した入力値(j_n, j_start, j_n, i, i_start, i_n)が不正(PUTのときのみ)
 * @retval -5 入出力指示が不正
 * @retval -6 指定した入力値(j_n)が不正(GETのときのみ)
 * <H3> 注意 </H3>
 * Reduced Gauss 格子を使う場合は1次元でデータを格納するので、定義ファイルの
 * size(格子数)には (実際の格子数) 1 と指定する。また、SUBC のサイズは 
 * 16 * j_n + 12 を計算した値を定義ファイルに書く。
 * <H3> 履歴 </H3>
 * この関数はNuSDaS1.2で実装された
*/
	N_SI4
NuSDaS_subc_rgau(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		N_SI4 *j, /**< 全球の南北分割数 */
		N_SI4 *j_start, /**< データの最北格子の番号(1始まり) */
		N_SI4 *j_n, /**< データの南北格子数 */
		N_SI4 i[], /**< 全球の東西格子数 */
		N_SI4 i_start[], /**< データの最西格子の番号(1始まり) */
		N_SI4 i_n[], /**< データの東西格子数 */
		float lat[], /**< 緯度 */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4	unity = 1;
	return nusdas_subc_rgau2(type1, type2, type3,
			basetime, member, validtime, &unity,
			j, j_start, j_n, i, i_start, i_n, lat, getput);
}

/** @brief SUBC RGAU のデフォルト値を設定
 * ファイルが新たに生成される際にRGAUレコードに書き込む値を設定する。
 * RGAU レコードや引数については nusdas_subc_rgau を参照。
 * @retval 0 正常終了
 * @retval -1 定義ファイルに "RGAU" が登録されていない
 * @retval -2 メモリの確保に失敗した
 * <H3> 互換性 </H3>
 * NuSDaS1.1 では、一つのNuSDaSデータセットに設定できる補助管理部の数は最大
 * 10 に制限されており、それを超えると-2が返された。一方、NuSDaS1.3 では
 * メモリが確保できる限り数に制限はなく、-2 をメモリ確保失敗のエラーコードに
 * 読み替えている。
 */
	N_SI4
NuSDaS_subc_rgau_preset1(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *j, /**< 全球の南北分割数 */
		const N_SI4 *j_start, /**< データの最北格子の番号(1始まり) */
		const N_SI4 *j_n, /**< データの南北格子数 */
		const N_SI4 i[], /**< 全球の東西格子数 */
		const N_SI4 i_start[], /**< データの最西格子の番号(1始まり) */
		const N_SI4 i_n[], /**< データの東西格子数 */
		const float lat[]) /**< 緯度 */
{
	struct subcrgau_put_param param;
	nustype_t nustype;
	nusdset_t *ds;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_USER);
	param.j = *j;
	param.j_start = *j_start;
	param.j_n = *j_n;
	param.i = i;
	param.i_start = i_start;
	param.i_n = i_n;
	param.lat = lat;
	if ((r = subcrgau_put_arg_check(&param)) != 0) {
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
	r = ds_subcpreset(ds, SYM4_RGAU,
			*j_n * 16 + 12,
			subcrgau_put_encode, &param);
	NUSDAS_CLEANUP;
	return r;
}

