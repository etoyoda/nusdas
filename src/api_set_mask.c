/** @file
 * @brief 改善されたビットマスク設定関数
 */

#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
#include "dset.h"
#include "glb.h"
#include "sys_time.h"
#include "sys_err.h"
# define NEED_PACK2NUSTYPE
#include "sys_sym.h"
#include "sys_mem.h"

/** @brief 改善型ビットマスク設定関数
 *
 * 配列 @p udata の内容に従って nusdas_make_mask() と同様に
 * マスクビット列を作成し
 * 指定した種別のデータセットに対して設定する。
 *
 * @retval 0 正常終了
 * @retval -5 未知の型名 @p utype が与えられた
 *
 * <H3>注意</H3>
 * 本関数によるマスクビットの設定は nusdas_parameter_change() に
 * 優先するが、他のデータセットには効果をもたない。
 *
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.3 で新設された。
 */
	N_SI4
NuSDaS_set_mask(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
		const void *udata, /**< データ配列 */
		const char utype[2], /**< データ配列の型 */
		N_SI4 usize /**< 配列の要素数 */)
{
	nusdset_t *ds;
	nustype_t nustype;
	N_UI4 buflen;
	unsigned char *buf;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(type1, type2, type3, &nustype);
	nus_debug(("--- NuSDaS_set_mask %#ys", &nustype));
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		nus_err((NUSERR_DsetNotFound, "dataset %#ys not found",
					&nustype));
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUS_ERR_CODE();
	}
	buflen = ((N_UI4)usize - 1) / 8 + 1; 
	if ((buf = nus_malloc(buflen)) == NULL) {
		r = NUSERR_MemShort;
		goto finish;
	}
	r = NuSDaS_make_mask(udata, utype, &usize, buf, (N_SI4 *)&buflen);
	if (r == 0) {
		if (ds->comm.param.dsp_pc_mask_bit) {
			nus_free((void *)ds->comm.param.dsp_pc_mask_bit);
		}
		ds->comm.param.dsp_pc_mask_bit = buf;
	} else {
		nus_free(buf);
	}
finish:
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}
