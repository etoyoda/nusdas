/** @file
 * @brief nusdas_parameter_change() の実装
 *
 * @note SIZEX, SIZEY の型が SI4 であるかどうか疑問。
 */
#include "config.h"
#include "nusdas.h"

#include <string.h>
#include <stddef.h>
#include "internal_types.h"
#include "sys_err.h"
#include "glb.h"

/** @brief オプション取得
 *
 * nusdas_parameter_change() の項目 @p param で設定される
 * パラメータの値を @p value の指す領域 (型は以下を参照) に書き込む。
 * <DL>
 * <DT>N_PC_MISSING_UI1<DD>1バイト整数の欠損値
 * <DT>N_PC_MISSING_SI2<DD>2バイト整数の欠損値
 * <DT>N_PC_MISSING_SI4<DD>4バイト整数の欠損値
 * <DT>N_PC_MISSING_R4<DD>4バイト実数の欠損値
 * <DT>N_PC_MISSING_R8<DD>8バイト実数の欠損値
 * <DT>N_PC_SIZEX<DD>4バイト整数に x 方向強制格子サイズを与える
 * <DT>N_PC_SIZEY<DD>4バイト整数に y 方向強制格子サイズを与える
 * <DT>N_PC_MASK_BIT<DD>
 * ビットマスク配列を返す。
 * この問合せは設定値が nusdas_make_mask() で作られた場合にしか機能しない。
 * <DT>N_PC_PACKING<DD>
 *  4バイトの文字列に強制パック方式名を与える。
 *  設定されていない場合は 4 文字のスペースが書き込まれる。
 * <DT>N_PC_ID_SET<DD>
 *  NRD 番号制約がかかっている場合その値、かかっていない場合 -1 を与える。
 * <DT>N_PC_WBUFFER<DD>
 *  4バイト整数に書き込みバッファサイズ (実行時オプション FWBF) を与える。
 * <DT>N_PC_RBUFFER<DD>
 *  4バイト整数に読み取りバッファサイズ (実行時オプション FRBF) を与える。
 * </DL>
 *
 * @retval 0 正常終了
 * @retval -1 サポートされていないパラメタである
 * @retval -2 ビットマスク配列は設定されていない
 * @retval -3 ビットマスク配列は設定されているが長さがわからない
 *
 * <H3>履歴</H3>
 * NuSDaS 1.3 で導入された。
 * */
	N_SI4
NuSDaS_inq_parameter(N_SI4 param, /**< 設定項目コード */
		void *value) /**< INTENT(OUT) 設定値 */
{
	NUSDAS_INIT;
	switch (param) {
		case N_PC_MISSING_UI1:
			*(N_UI1 *)value = GlobalConfig(pc_missing_ui1);
			break;
		case N_PC_MISSING_SI2:
			*(N_SI2 *)value = GlobalConfig(pc_missing_si2);
			break;
		case N_PC_MISSING_SI4:
			*(N_SI4 *)value = GlobalConfig(pc_missing_si4);
			break;
		case N_PC_MISSING_R4:
			*(float *)value = GlobalConfig(pc_missing_r4);
			break;
		case N_PC_MISSING_R8:
			*(double *)value = GlobalConfig(pc_missing_r8);
			break;
		case N_PC_MASK_BIT:
			if (GlobalDSConfig(pc_mask_bit) == NULL) {
				NUSDAS_CLEANUP;
				return -2;
			} else if (GlobalDSConfig(pc_mask_bit)
					== GlobalConfig(saved_mask)) {
				memcpy(value, GlobalConfig(saved_mask),
					GlobalConfig(saved_mask_size));
			} else {
				NUSDAS_CLEANUP;
				return -3;
			}
			break;
		case N_PC_PACKING:
			*(N_SI4 *)value = GlobalDSConfig(pc_packing);
			break;
		case N_PC_SIZEX:
			*(N_SI4 *)value = GlobalDSConfig(pc_sizex);
			break;
		case N_PC_SIZEY:
			*(N_SI4 *)value = GlobalDSConfig(pc_sizey);
			break;
		case N_PC_ID_SET:
			*(N_SI4 *)value = GlobalConfig(nrd_override);
			break;
		case N_PC_WBUFFER:
			*(N_SI4 *)value = GlobalDSConfig(pc_wbuffer);
			break;
		case N_PC_RBUFFER:
			*(N_SI4 *)value = GlobalDSConfig(pc_rbuffer);
			break;
		default:
			NUSDAS_CLEANUP;
			return -1;
	}
	NUSDAS_CLEANUP;
	return 0;
}
