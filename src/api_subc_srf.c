/** @file
 * @brief 数値予報系 nusdas_subc_* の実装
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include <ctype.h>
#include "sys_err.h"
#include "sys_time.h"
#include "sys_kwd.h"
# define NEED_STR3SYM4UPCASE
# define NEED_STR2SYM4
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSDIMS
#include "sys_sym.h"
#include "sys_endian.h"
# define NEED_MAKE_UI8
#include "sys_int.h"
#include "dset.h"
#include "glb.h"

struct subcsrf_param {
	nusdims_t	dims;
	sym4_t		group;
	N_UI4		*data;
};

	INLINE N_SI4
subcsrf_chunksize(sym4_t group)
{
	switch (group) {
		case SYM4_RADR:
		case SYM4_THUN:
			return 1;
			break;
		case SYM4_RADS:
			return 6;
			break;
		case SYM4_DPRD:
			return 8;
			break;
		case SYM4_ISPC:
			return 128;
			break;
		default:
			return nus_err((NUSERR_SC_BadGroup,
						"bad group %Ps", group));
	}
}

	INLINE int
subcsrf_recsize(nusdset_t *ds, N_UI4 siz,
		struct subcsrf_param *param,
		N_UI4 *ofs, N_UI4 *nelems)
{
	N_UI4 nm, nv, nz, ne, iv, im, iz, ie, arynelems;
	N_UI8 vt;
	N_SI4 size, r;
	vt = make_ui8(param->dims.validtime1, param->dims.validtime2);
	size = 1;
	r = ds_inq_cntl(ds, &param->dims, SYM4_MIDX, &im, 
			&param->dims.member, 1);
	if(r >= 0){
		r = ds_inq_cntl(ds, &param->dims, SYM4_VIDX, &iv, 
				&vt, 1);
	}
	if(r >= 0){
		r = ds_inq_cntl(ds, &param->dims, SYM4_ZIDX, &iz, 
				&param->dims.plane1, 1);
	}
	if(r >= 0){
		r = ds_inq_cntl(ds, &param->dims, SYM4_EIDX, &ie, 
				&param->dims.element, 1);
	}
	if(r >= 0){
		r = ds_inq_cntl(ds, &param->dims, N_MEMBER_NUM, &nm, 
			    &size, 1);
	}
	if(r >= 0){
		r = ds_inq_cntl(ds, &param->dims, N_VALIDTIME_NUM, &nv, 
				&size, 1);
	}
	if(r >= 0){
		r = ds_inq_cntl(ds, &param->dims, N_PLANE_NUM, &nz, 
				&size, 1);
	}
	if(r >= 0){
		ds_inq_cntl(ds, &param->dims, N_ELEMENT_NUM, &ne, 
			    &size, 1);
	}
	if(r >= 0){
		if ((N_SI4)im < 0) {
			return nus_err((NUSERR_SC_BadArg, "member \"%Ps\" not found in %#ys",
				param->dims.member, &ds->comm.nustype));
		}
		if ((N_SI4)iv < 0) {
			return nus_err((NUSERR_SC_BadArg, "validtime \"%T/%T\" not found in %#ys",
				param->dims.validtime1, param->dims.validtime2, &ds->comm.nustype));
		}
		if ((N_SI4)iz < 0) {
			return nus_err((NUSERR_SC_BadArg, "plane \"%Qs\" not found in %#ys",
				param->dims.plane1, &ds->comm.nustype));
		}
		if ((N_SI4)ie < 0) {
			return nus_err((NUSERR_SC_BadArg, "element \"%Qs\" not found in %#ys",
				param->dims.element, &ds->comm.nustype));
		}
		arynelems = nm * nv * nz * ne;
		*nelems = subcsrf_chunksize(param->group);
		*ofs = (((im * nv + iv) * nz + iz) * ne + ie) * *nelems;
		/* check size */
		if (siz < arynelems * *nelems * sizeof(N_SI4)) {
			return nus_err((NUSERR_SC_SizeMismatch,
					"record size %Pu < needed %Pu",
					siz, arynelems * *nelems * sizeof(N_SI4)));
		}
	}
	return  r < 0 ? r : 0;
}

	static int
subcsrf_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds, N_SI4 ofs_flg)
{
	const N_UI4 *rec = vrec;
	struct subcsrf_param *param = vparam;
	N_UI4 ofs, nelems, i, check;
	int r;
	if (ofs_flg == 1) {
		r = subcsrf_recsize(ds, siz, param, &ofs, &nelems);
		if (r) {
			return r;
		}
	} else{
		ofs = 0;
		nelems = subcsrf_chunksize(param->group);
	}
	check = 0;
	for (i = 0; i < nelems; i++) {
		param->data[i] = NTOH4(rec[ofs + i]);
		check = (check || ~rec[ofs + i]);
	}
	if (check == 0) {
		for (i = 0; i < nelems; i++) {
			nus_debug(("%Pu: %Px", i, param->data[i]));
		}
		return nus_err((NUSERR_SC_Uninitialized,
			"nusdas_subc_srf: uninitialized part"));
	}
	return 0;
}

	static int
subcsrf_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct subcsrf_param *param = vparam;
	int r;
	r = ds_read_aux(ds, &param->dims, SYM4_SUBC, param->group,
			subcsrf_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		return 0;
	}
	return 1;
}

	static int
subcsrf_get(nustype_t *type, struct subcsrf_param *param)
{
	int r;
	r = nusglb_dsscan_nustype(subcsrf_get_dsselect, type, param);
	if (r > 0) {
		nuserr_cancel(MARK_FOR_DSET);
		return 0;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

	static int
subcsrf_put_encode(void *vrec, N_UI4 siz, void *vparam, union nusdset_t *ds)
{
	N_UI4 *rec = vrec;
	struct subcsrf_param *param = vparam;
	N_UI4 ofs, nelems, i;
	int r;
	r = subcsrf_recsize(ds, siz, param, &ofs, &nelems);
	if (r) {
		return r;
	}
	for (i = 0; i < nelems; i++) {
		rec[ofs + i] = HTON4(param->data[i]);
	}
	return 0;
}

	static int
subcsrf_put(nustype_t *type, struct subcsrf_param *param)
{
	nusdset_t *ds;
	ds = nusglb_find_dset(type);
	if (ds == NULL) {
		return nus_err((NUSERR_DsetNotFound,
					"dataset %ys not found", type));
	}
	return ds_write_aux(ds, &param->dims, SYM4_SUBC, param->group,
			~(size_t)0,
			subcsrf_put_encode, param);
}

/** @brief 降短系 SUBC へのアクセス */
	N_SI4
NuSDaS_subc_srf2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		const char plane1[6], /**< 面1 */
		const char plane2[6], /**< 面2 */
		const char element[6], /**< 要素名 */
		const char group[4], /**< 群名 */
		N_SI4 *data, /**< データ配列 */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
{
	struct subcsrf_param param;
	nustype_t type;
	sym4_t op;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	op = str3sym4upcase(getput);
	param.group = str2sym4(group);
	nus_debug(("--- subc_srf %Ps %Ps", param.group, op));
	pack2nustype(type1, type2, type3, &type);
	pack2nusdims(*basetime, member, *validtime1, *validtime2,
			plane1, plane2, element,
			&param.dims);
	param.data = (N_UI4 *)data;
	if (subcsrf_chunksize(param.group) < 0) {
		NUSDAS_CLEANUP;
		return NUS_ERRNO;
	}
	if (op == SYM4_GET) {
		r = subcsrf_get(&type, &param);
	} else if (op == SYM4_PUT) {
		r = subcsrf_put(&type, &param);
	} else {
		r = -5;
	}
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief 降短系 SUBC へのアクセス
 * 降水短時間予報系のデータの補助管理部へのアクセスを提供する。
 * 群名には次のもののいずれかを指定する。
 * <DL>
 * <DT>ISPC<DD>
 * レーダーや雨量計の運用情報、レベル値変換テーブルが格納される。
 * data には 128要素の4バイト整数型配列を用意する。内部のフォーマットは
 * 4バイト整数型であることは関係ないが、バイトオーダーの変換はされるので
 * 注意が必要。
 * <DT>THUN<DD>
 * 詳細未詳。
 * data には 4バイト整数型変数を用意する。
 * <DT>RADR<DD>
 * レーダー観測に関する情報。data には 4バイト整数型変数を用意する。
 * <DT>RADS<DD>
 * レーダー観測に関する情報。data には 6要素の4バイト整数型配列を用意する。
 * <DT>DPRD<DD>
 * ドップラーレーダー観測に関する情報。
 * data には 8要素の4バイト整数型配列を用意する。
 * </DL>
 * @retval 0 正常終了
 * @retval -2 要求されたレコードが存在しない、または書かれていない。
 * @retval -3 レコードサイズが不正
 * @retval -4 群名が不正
 * @retval -5 入出力指示が不正
 * <H3> 履歴 </H3>
 * この関数は NuSDaS1.0 から存在した。
 */
	N_SI4
NuSDaS_subc_srf(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		const char plane[6], /**< 面 */
		const char element[6], /**< 要素名 */
		const char group[4], /**< 群名 */
		N_SI4 *data, /**< データ配列 */
		const char getput[3]) /**< 入出力指示 (@p "GET" または @p "PUT") */
{
	N_SI4 unity = 1;
	return NuSDaS_subc_srf2(type1, type2, type3, basetime, member,
			validtime, &unity, plane, plane, element,
			group, data, getput);
}
