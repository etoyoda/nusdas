/** @file
 * @brief nusdas_subc_rgau_inq_jn() の実装
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_time.h"
#include "sys_kwd.h"
#include <ctype.h>
#include <stddef.h>
#include "sys_err.h"
# define NEED_MEMCPY_NTOH4
#include "sys_endian.h"
# define NEED_PACK2NUSTYPE
# define NEED_PACK2NUSBMV
#include "sys_sym.h"
#include "dset.h"
#include "glb.h"

/** @brief RGAU 型 SUBC を読み取るために受け渡す情報 */
struct inq_jn_param {
	const nusdims_t *dims;
	N_SI4	j_n;
};

	static int
sigmrgau_get_decode(const void *vrec, N_UI4 siz, void *vparam,
		union nusdset_t *ds UNUSED, N_SI4 ofs_flg UNUSED)
{
	const char *rec = vrec;
	struct inq_jn_param *param = vparam;
	N_UI4	nj;
	memcpy_ntoh4(&nj, rec + 8, 1);
	if (~nj == 0) {
		param->j_n = -1;
		return nus_err((NUSERR_SC_Uninitialized, "Uninitialized SUBC"));
	}
	if (siz != (nj * 16 + 12)) {
		return nus_err((NUSERR_SC_SizeMismatch, "Broken SUBC/RGAU"));
	}
	param->j_n = nj;
	return 0;
}

	static int
sigmrgau_get_dsselect(nusdset_t *ds, void *vparam)
{
	struct inq_jn_param *param = vparam;
	int r;
	r = ds_read_aux(ds, param->dims, SYM4_SUBC, SYM4_RGAU,
			sigmrgau_get_decode, vparam);
	nus_debug(("ds_read_aux => %d", r));
	if (r == NUSERR_SC_Uninitialized) {
		return -1;
	} else if (r < 0) {
		/* みつからないので探索続行 */
		return 0;
	}
	return 1;
}

/** @brief SUBC RGAU 記録の大きさを問合せ */
	N_SI4
NuSDaS_subc_rgau_inq_jn2(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		N_SI4 *j_n) /**< INTENT(OUT) 南北格子数 */
{
	struct inq_jn_param param;
	nusdims_t	dims;
	nustype_t	type;
	N_SI4		r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &type);
	pack2nusbmv(*basetime, member, *validtime1, *validtime2, &dims);
	param.dims = &dims;
	r = nusglb_dsscan_nustype(sigmrgau_get_dsselect, &type, &param);
	*j_n = param.j_n;
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

/** @brief SUBC RGAU 記録の大きさを問合せ
 * RGAU に記録されている j_n (南北格子数) を問い合わせる。
 * @retval 正 正常終了
 * @retval -2 要求されたレコードが存在しない、または書き込まれていない。
 * @retval -3 レコードのサイズが不正
 * <H3> 履歴 </H3>
 * この関数は NuSDaS1.2で導入された。
*/
	N_SI4
NuSDaS_subc_rgau_inq_jn(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime, /**< 対象時刻(通算分) */
		N_SI4 *j_n) /**< INTENT(OUT) 南北格子数 */
{
	N_SI4 unity = 1;
	return NuSDaS_subc_rgau_inq_jn2(type1, type2, type3,
			basetime, member, validtime, &unity, j_n);
}
