/** @file
 * @brief nusdas_subc_eta_inq_nz() の実装
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_time.h"
#include <ctype.h>
#include <stddef.h>
#include "sys_err.h"
# define NEED_MEMCPY_NTOH4
#include "sys_endian.h"
#include "sys_kwd.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
#include "sys_sym.h"
#include "dset.h"
#include "glb.h"

/** @brief SUBC を読み取るために受け渡す情報 */
struct inq_nz_param {
	const nusdims_t *dims;
	sym4_t	group;
	N_SI4	nz;
};

	static int
sigmvertical_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct inq_nz_param *param = vparam;
	N_UI4	nz, siz2;
	memcpy_ntoh4(&nz, rec, 1);
	if (~nz == 0) {
		param->nz = -1;
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	switch (param->group) {
		case SYM4_ZHYB:
			siz2 = nz * 24 + 12;
			break;
		case SYM4_ETA:
		case SYM4_SIGM:
			siz2 = nz * 8 + 16;
			break;
		default:
			siz2 = 0;
	}
	if (siz != siz2) {
		return nus_err((NUSERR_SC_SizeMismatch,
				"SUBC/%Ps size %Pu != expected %Pu",
				param->group, siz, siz2));
	}
	param->nz = nz;
	return 0;
}

	static int
sigmrgau_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct inq_nz_param *param = vparam;
	int r;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, param->group,
			sigmvertical_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* みつからないので探索続行 */
		return 0;
	}
	return 1;
}

/** @brief SUBC 記録の鉛直層数問合せ */
	N_SI4
NuSDaS_subc_eta_inq_nz2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
	        const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		const char group[4], /**< 群名 */
		N_SI4 *n_levels) /**< INTENT(OUT) 鉛直層数 */
{
	struct inq_nz_param param;
	nusdims_t	dims;
	nustype_t	type;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	param.dims = &dims;
	param.group = str2sym4(group);
	r = nusglb_dsscan_nustype(sigmrgau_get_dsselect, &type, &param);
	*n_levels = param.nz;
	NUSPROF_MARK(NP_USER);
	if (r > 0) nuserr_cancel(MARK_FOR_DSET);
	NUSDAS_CLEANUP;
	if (r > 0) {
		return r;
	} else {
		r = NUS_ERR_CODE();
		return r ? r : NUSERR_NoDfileToRead;
	}
}

/** @brief SUBC 記録の鉛直層数問合せ
 * SUBC レコードの ETA, SIGM, ZHYB に記録された鉛直層数を問い合わせる。
 * 群名には "ETA ", "SIGM", "ZHYB" のいずれかを指定する。
 * @retval 正 正常終了
 * <H3> 履歴 </H3>
 * この関数は NuSDaS1.2 で導入された。
 */
	N_SI4
NuSDaS_subc_eta_inq_nz(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
	        const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		const char group[4], /**< 群名 */
		N_SI4 *n_levels) /**< INTENT(OUT) 鉛直層数 */
{
	N_SI4	unity = 1;
	return NuSDaS_subc_eta_inq_nz2(type1, type2, type3,
			basetime, member, validtime, &unity,
			group, n_levels);
}
