/** @file
 * @brief nusdas_info() の実装
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_time.h"
#include "sys_kwd.h"
#include <string.h>
#include <ctype.h>
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
# define NEED_STR3SYM4UPCASE
#include "sys_sym.h"
#include <stddef.h>
#include "sys_err.h"
#include "glb.h"
#include "dset.h"

struct info_get_param {
	nusdims_t *dims;
	sym4_t grp;
	N_UI4 bytesize;
	char *info;
};

/** @brief INFO 読取 (対データファイルコールバック)
 * @note rec, siz は定義ファイル information 文に書く長さに相当する部分
 */
	static int
info_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	struct info_get_param *param = vparam;
	if (siz > param->bytesize) {
		return nus_err((NUSERR_SC_ShortBuf,
				"INFO %Pu > your buffer %Pd",
				siz, param->bytesize));
	}
	memcpy(param->info, vrec, siz);
	return siz;
}

/** @brief INFO 読取 (対データセットコールバック) */
	static int
info_get_dsselect(nusdset_t *ds, void *vparam)
{
	int r;
	struct info_get_param *param = vparam;
	r = ds_read_aux(ds, param->dims, SYM4_INFO, param->grp,
			info_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_PeekFailed) {
		/* エラーコードつけかえ */
		r = SETERR(NUSERR_IN_PeekFailed);
	}
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* 見付からなければ探索続行 */
		return 0;
	}
	return r;
}

/** @brief INFO 読取の実働部隊 */
	static int
info_get(nustype_t *type, nusdims_t *dims, sym4_t grp, N_SI4 bytesize,
		char *info)
{
	struct info_get_param param;
	int	r;
	param.grp = grp;
	param.dims = dims;
	param.bytesize = bytesize;
	param.info = info;
	r = nusglb_dsscan_nustype(info_get_dsselect, type, &param);
	nus_debug(("nusglb_dsscan_nustype => %d", r));
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return r;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

struct info_put_param {
	const char *info;
	N_SI4 written_size;
};

	static int
info_put_encode(void *rec, N_UI4 siz, void *arg, union nusdset_t *ds UNUSED)
{
	struct info_put_param *param = arg;
	memcpy(rec, param->info, siz);
	param->written_size = siz;
	return 0;
}

/** @brief ETA/SIGM 型 SUBC 書き込みの実働部隊 */
	static int
info_put(nustype_t *type, nusdims_t *dims, sym4_t group,
		N_UI4 bytesize, char *info)
{
	union nusdset_t *ds;
	struct info_put_param param;
	int r;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	param.info = info;
	r = ds_write_aux(ds, dims, SYM4_INFO, group, bytesize,
		info_put_encode, &param);
	if (r == 0) {
		r = param.written_size;
	}
	return r;
}

/** @brief INFO 記録へのアクセス */
	N_SI4
NuSDaS_info2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		const char group[4], /**< 群名 */
		char	info[], /**< INFO 記録内容 */
		const N_SI4 *bytesize, /**< INFO 記録のバイト数 */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
{
	nustype_t	type;
	nusdims_t	dims;
	sym4_t		op, grp;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- nusdas_info"));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	grp = MEM2SYM4(group);
	op = str3sym4upcase(getput);
	if (op == SYM4_GET) {
		r = info_get(&type, &dims, grp, *bytesize, info);
	} else if (op == SYM4_PUT) {
		r = info_put(&type, &dims, grp, *bytesize, info);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief INFO 記録へのアクセス 
 * @retval 非負 書き出したINFOのバイト数
 * @retval -3 バッファが不足している
 * @retval -5 入出力指示が不正
 *
 * <H3> 注意 </H3>
 * NuSDaS1.1では、バッファが不足している場合でもバッファの大きさの分だけを
 * 書き込み、そのサイズを返していたが、NuSDaS1.3ではこのような場合は-3が返る。
 * また、INFO のサイズは NuSDaS1.3 で新設された nusdas_inq_subcinfo で
 * 問い合わせ項目を N_INFO_NUM にすれば得ることができる。
*/
	N_SI4
NuSDaS_info(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		const char group[4], /**< 群名 */
		char	info[], /**< INFO 記録内容 */
		const N_SI4 *bytesize, /**< INFO 記録のバイト数 */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4 unity = 1;
	return NuSDaS_info2(type1, type2, type3, basetime, member,
			validtime, &unity, group, info, bytesize, getput);
}
