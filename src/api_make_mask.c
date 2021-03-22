/** @file
 * @brief nusdas_make_mask() の実装
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "sys_kwd.h"
#include <stddef.h>
#include "sys_err.h"
#include "glb.h"

/** @brief マスクビット配列の作成
 *
 * 配列 @p udata の内容をチェックしてマスクビット列を作成し
 * @p maskbit に書き込む。
 * 引数 @p utype と欠損値は配列の型に応じて次のように指定する。
 * <DL>
 * <DT>1バイト整数型<DD>
 * 引数 @p utype に N_I1 を指定する。
 * 配列中の欠損扱いしたい要素に N_MV_UI1 を設定しておく。
 * <DT>2バイト整数型<DD>
 * 引数 @p utype に N_I2 を指定する。
 * 配列中の欠損扱いしたい要素に N_MV_SI2 を設定しておく。
 * <DT>4バイト整数型<DD>
 * 引数 @p utype に N_I4 を指定する。
 * 配列中の欠損扱いしたい要素に N_MV_SI4 を設定しておく。
 * <DT>4バイト実数型<DD>
 * 引数 @p utype に N_R4 を指定する。
 * 配列中の欠損扱いしたい要素に N_MV_R4 を設定しておく。
 * <DT>8バイト実数型<DD>
 * 引数 @p utype に N_R8 を指定する。
 * 配列中の欠損扱いしたい要素に N_MV_R8 を設定しておく。
 * </DL>
 *
 * @retval 0 正常終了
 * @retval -1 配列長 @p mb_bytes が不足している
 * @retval -5 未知の型名 @p utype が与えられた
 *
 * <H3>サイズ要件</H3>
 * @p mb_bytes は少なくとも (@p usize + 7) / 8 バイト以上必要である。
 *
 * <H3>履歴</H3>
 * nusdas_make_mask() は NuSDaS 1.0 から存在する。
 *
 */
	N_SI4
NuSDaS_make_mask(const void *udata, /**< 格子データ */
		const char utype[2], /**< 格子データの型 */
		const N_SI4 *usize, /**< 格子データの要素数 */
		void *output, /**< INTENT(OUT) マスクビット配列 */
		const N_SI4 *mb_bytes) /**< マスクビット配列のバイト数 */
{
	sym4_t	utype_s = mem2sym4(utype);
	const double	*ddata;
	const float	*fdata;
	const N_SI4	*idata;
	const N_SI2	*hdata;
	const N_UI1	*cdata;
	N_UI1 *maskbits = output;
	size_t	i, round;
	N_SI4 r;

	NUSDAS_INIT;
	if (*mb_bytes < (*usize - 1) / 8 + 1) {
		r = nus_err((NUSERR_MM_SmallBuf,
				"mask bit array (%Pd bytes given) "
				"is too short (%Pd required)",
				*mb_bytes,
				(*usize - 1) / 8 + 1));
		NUSDAS_CLEANUP;
		return r;
	}
	round = *usize / 8;
	switch (utype_s) {
	case SYM4_R4:
#define EXPAND(xdata, miss) \
		xdata = udata; \
		for (i = 0; i < round; i++) { \
			maskbits[i] = (\
			  ((xdata[i * 8    ] != miss) << 7) \
			| ((xdata[i * 8 + 1] != miss) << 6) \
			| ((xdata[i * 8 + 2] != miss) << 5) \
			| ((xdata[i * 8 + 3] != miss) << 4) \
			| ((xdata[i * 8 + 4] != miss) << 3) \
			| ((xdata[i * 8 + 5] != miss) << 2) \
			| ((xdata[i * 8 + 6] != miss) << 1) \
			|  (xdata[i * 8 + 7] != miss)); \
		} \
		if ((*usize % 8) != 0) { \
			unsigned char c; \
			for (c = 0, i = round * 8; i < (size_t)*usize; i++) { \
				c |= ((xdata[i] != miss) << (7 - (i % 8))); \
			} \
			maskbits[round] = c; \
		}
		EXPAND(fdata, N_MV_R4)
		break;
	case SYM4_R8:
		EXPAND(ddata, N_MV_R8)
		break;
	case SYM4_I4:
		EXPAND(idata, N_MV_SI4)
		break;
	case SYM4_I2:
		EXPAND(hdata, N_MV_SI2)
		break;
	case SYM4_I1:
		EXPAND(cdata, N_MV_UI1)
		break;
	default:
		r = nus_err((NUSERR_MM_BadType,
				"unrecognized user data type <%.2s>", utype));
		NUSDAS_CLEANUP;
		return r;
	}
	GlobalConfig(saved_mask) = maskbits;
	GlobalConfig(saved_mask_size) = *mb_bytes;
	NUSDAS_CLEANUP;
	return 0;
}
