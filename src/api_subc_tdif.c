/** @file
 * @brief nusdas_subc_tdif の実装
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
#include "sys_endian.h"
#include "sys_kwd.h"
# define NEED_MAKE_UI8
#include "sys_int.h"

struct subctdif_param {
	nusdims_t dims;
	N_SI4 diff_time;
	N_SI4 total_sec;
};

	INLINE int
subctdif_recsize(nusdset_t *ds, N_UI4 siz,
		struct subctdif_param *param,
		N_UI4 *ofs, N_UI4 *arynelems)
{
	N_UI4 nm, nv, iv, im;
	N_UI8 vt;
	vt = make_ui8(param->dims.validtime1, param->dims.validtime2);
	ds_inq_cntl(ds, &param->dims, SYM4_MIDX, &im, &param->dims.member, 1);
	ds_inq_cntl(ds, &param->dims, SYM4_VIDX, &iv, &vt, 1);
	ds_inq_cntl(ds, &param->dims, N_MEMBER_NUM, &nm, NULL, 1);
	ds_inq_cntl(ds, &param->dims, N_VALIDTIME_NUM, &nv, NULL, 1);
	if ((N_SI4)im < 0) {
		return nus_err((NUSERR_SC_BadArg, "member \"%Ps\" not found in %#ys",
			param->dims.member, &ds->comm.nustype));
	}
	if ((N_SI4)iv < 0) {
		return nus_err((NUSERR_SC_BadArg, "validtime \"%T/%T\" not found in %#ys",
			param->dims.validtime1, param->dims.validtime2, &ds->comm.nustype));
	}
	*ofs = nv * im + iv;
	*arynelems = nm * nv;
	/* check size */
	if (siz < *arynelems * 8) {
		return nus_err((NUSERR_SC_SizeMismatch,
				"record size %Pu < needed %Pu",
				siz, *arynelems * 8));
	}
	return 0;
}

	static int
subctdif_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds, N_SI4 ofs_flag)
{
	const N_UI4 *rec = vrec;
	struct subctdif_param *param = vparam;
	N_UI4 ofs, arynelems;
	int r;
	if (ofs_flag == 1){
		if ((r = subctdif_recsize(ds, siz, param, &ofs, &arynelems)) != 0) {
			return r;
		}
	} else{
		ofs = 0;
		arynelems = 1;
	}
	/* decode */
	param->diff_time = NTOH4(rec[ofs]);
	param->total_sec = NTOH4(rec[ofs + arynelems]);
	return 0;
}

	static int
subctdif_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct subctdif_param *param = vparam;
	int r;
	r = ds_read_aux(ds, &param->dims, SYM4_SUBC, SYM4_TDIF,
			subctdif_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		return 0;
	}
	return 1;
}

	static int
subctdif_get(nustype_t *type, struct subctdif_param *param)
{
	int r;
	r = nusglb_dsscan_nustype(subctdif_get_dsselect, type, param);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

	static int
subctdif_put_encode(void *vrec, N_UI4 siz, void *vparam, union nusdset_t *ds)
{
	N_UI4 *rec = vrec;
	struct subctdif_param *param = vparam;
	N_UI4 ofs, arynelems;
	int r;
	if ((r = subctdif_recsize(ds, siz, param, &ofs, &arynelems)) != 0) {
		return r;
	}
	rec[ofs] = HTON4(param->diff_time);
	rec[ofs + arynelems] = HTON4(param->total_sec);
	return 0;
}

	static int
subctdif_put(nustype_t *type, struct subctdif_param *param)
{
	nusdset_t *ds;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, &param->dims, SYM4_SUBC, SYM4_TDIF,
			~(size_t)0,
			subctdif_put_encode, param);
}

/** @brief SUBC TDIF へのアクセス */
	N_SI4
NuSDaS_subc_tdif2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		N_SI4	*diff_time, /**< 対象時刻からのずれ(秒) */
		N_SI4	*total_sec, /**< 総予報時間(秒) */
		const char getput[3])
		/**< 入出力指示 (@p "GET" または @p "PUT") */
{
	struct subctdif_param param;
	nustype_t type;
	sym4_t op;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	op = str3sym4upcase(getput);
	nus_debug(("--- subc_tdif %Ps", op));
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &param.dims);
	if (op == SYM4_GET) {
		r = subctdif_get(&type, &param);
		*diff_time = param.diff_time;
		*total_sec = param.total_sec;
	} else if (op == SYM4_PUT) {
		param.diff_time = *diff_time;
		param.total_sec = *total_sec;
		r = subctdif_put(&type, &param);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief SUBC TDIF へのアクセス
 * 格納した値の時刻の対象時間とのずれ、積算時間を格納する補助管理部 TDIF 
 * へのアクセスを提供する。
 * @retval 0 正常終了
 * @retval -2 要求されたレコードが存在しない、または書き込まれていない。
 * @retval -3 レコードサイズが不正
 * @retval -5 入出力指示が不正
 *
 * <H3>補足</H3>
 * <UL>
 * <LI> diff_time = 時間範囲始点 - 対象時刻 [秒単位]
 * <LI> total_sec = 時間範囲終点 - 時間範囲始点 [秒単位]
 * </UL>
 *
 * <H3>履歴</H3>
 * この関数は NuSDaS1.0 から存在した。
 */
	N_SI4
NuSDaS_subc_tdif(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		N_SI4	*diff_time, /**< 対象時刻からのずれ(秒) */
		N_SI4	*total_sec, /**< 総予報時間(秒) */
		const char getput[3])
		/**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4 unity = 1;
	return NuSDaS_subc_tdif2(type1, type2, type3, basetime, member,
			validtime, &unity, diff_time, total_sec, getput);
}
