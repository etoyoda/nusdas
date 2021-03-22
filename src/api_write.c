/** \file
 * \brief nusdas_write の実装。
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include "dset.h"
#include "glb.h"
# define NEED_PACK2NUSDIMS
# define NEED_PACK2NUSTYPE
# define NEED_MEM2SYM4
#include "sys_sym.h"
#include "dfile.h"
#include "sys_time.h"
#include "sys_err.h"

/** @brief データ記録の書出
 *
 * データレコードを指定された場所に書き出す。
 *
 * @retval 正 実際に書き出された要素数
 * @retval -2 メンバー名、面名、要素名が間違っている
 * @retval -2 このレコードは ELEMENTMAP によって書き出しが禁止されている
 * @retval -3 与えられたデータ要素数 @p nelems が必要より小さい
 * @retval -4 指定データセットにはデータ配列の型 @p fmt は書き出せない
 * @retval -5 データレコード長が固定レコード長を超える
 * @retval -6 データセットの欠損値指定方式と RLEN 圧縮は併用できない
 * @retval -7 マスクビットの設定がされていない
 * @retval -8 エンコード過程でのエラー (数値が過大または RLEN 圧縮エラー)
 *
 * <H3>注意</H3>
 * <UL>
 * <LI>データセットの指定と異なる大きさのレコードを書き出すにはあらかじめ
 * nusdas_parameter_change() を使って設定を変えておく。
 * <LI>格子数 (データセットの指定または nusdas_parameter_change() 設定)
 * より大きい要素数 @p nelems を指定するとエラーにはならず、
 * 余った要素が書き出されない結果となるので注意されたい。
 * </UL>
 *
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.0 から存在した。
 */
	N_SI4
NuSDaS_write(
		const char utype1[8],	/**< 種別1 */
		const char utype2[4],	/**< 種別2 */
		const char utype3[4],	/**< 種別3 */
		const N_SI4 *basetime,	/**< 基準時刻(通算分) */
		const char member[4],	/**< メンバー名 */
		const N_SI4 *validtime,	/**< 対象時刻(通算分) */
		const char plane[6],	/**< 面の名前 */
		const char element[6],	/**< 要素名 */
		const void *data,	/**< データ配列 */
		const char fmt[2],	/**< データ配列の型 */
		const N_SI4 *nelems)	/**< データ配列の要素数 */
{
	nusdset_t *ds;
	nustype_t nustype;
	nusdims_t dims;
	struct obuffer_t userdata;
	int r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	pack2nustype(utype1, utype2, utype3, &nustype);
	pack2nusdims(*basetime, member, *validtime, 1, plane, NULL,
			element, &dims);
	nus_debug(("--- NuSDaS_write %#ys/%#ms", &nustype, &dims));
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		nus_err((NUSERR_DsetNotFound, "dataset %#ys not found",
					&nustype));
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUSERR_DsetNotFound;
	}
	userdata.ob_ptr = data;
	userdata.ob_fmt = mem2sym4(fmt);
	userdata.nelems = *nelems;
	r = ds_writedata(ds, &dims, &userdata);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

/** @brief データ記録の書出
 */
	N_SI4
NuSDaS_write2(
		const char utype1[8],	/**< 種別1 */
		const char utype2[4],	/**< 種別2 */
		const char utype3[4],	/**< 種別3 */
		const N_SI4 *basetime,	/**< 基準時刻(通算分) */
		const char member[4],	/**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1(通算分) */
		const N_SI4 *validtime2, /**< 対象時刻2(通算分) */
		const char plane1[6],	/**< 面の名前1 */
		const char plane2[6],	/**< 面の名前2 */
		const char element[6],	/**< 要素名 */
		const void *data,	/**< データを与える配列 */
		const char fmt[2],	/**< data の型 */
		const N_SI4 *nelems)	/**< data の要素数 */
{
	nusdset_t *ds;
	nustype_t nustype;
	nusdims_t dims;
	struct obuffer_t userdata;
	N_SI4 r;
	NUSDAS_INIT;
	NUSPROF_MARK(NP_API);
	nus_debug(("--- NuSDaS_write2"));
	pack2nustype(utype1, utype2, utype3, &nustype);
	ds = nusglb_find_dset(&nustype);
	if (ds == NULL) {
		nus_err((NUSERR_DsetNotFound, "dataset not found"));
		NUSPROF_MARK(NP_USER);
		NUSDAS_CLEANUP;
		return NUSERR_DsetNotFound;
	}
	pack2nusdims(*basetime, member, *validtime1, *validtime2,
			plane1, plane2, element, &dims);
	userdata.ob_ptr = data;
	userdata.ob_fmt = mem2sym4(fmt);
	userdata.nelems = *nelems;
	r = ds_writedata(ds, &dims, &userdata);
	NUSPROF_MARK(NP_USER);
	NUSDAS_CLEANUP;
	return r;
}

