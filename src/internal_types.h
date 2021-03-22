/** \file
 * \brief 内部的データ構造の定義
 *
 * このヘッダを利用する応用プログラムは他のバージョンへの移植性はない。
 */

#ifndef NUSDAS_H
#include "nusdas.h"
#endif

#define INTERNAL_TYPES_H

/** @brief 4文字を保持するための整数型
 *
 * 文字列はマシンバイトオーダーで格納される。ファイルへの書出禁止。
 */
typedef N_UI4	sym4_t;

/** @brief 8文字を保持するための整数型
 *
 * 文字列はマシンバイトオーダーで格納される。ファイルへの書出禁止。
 */
typedef N_UI8	sym8_t;

/** @brief NuSDaS 種別
 */
typedef struct {
	sym8_t	type1; /**< 種別1 */
	sym4_t	type2; /**< 種別2 */
	sym4_t	type3; /**< 種別3 */
} nustype_t;

#define nustype_eq(a, b) \
	(  ui8_eq((a).type1, (b).type1) \
	&& ((a).type2 == (b).type2) \
	&& ((a).type3 == (b).type3) )

#define nustype_p_eq(a, b) \
	(  ui8_eq((a)->type1, (b)->type1) \
	&& ((a)->type2 == (b)->type2) \
	&& ((a)->type3 == (b)->type3) )

/** @brief 種別以外の NuSDaS 次元 (basetime..element)
 */
typedef struct nusdims_t {
	N_SI4	basetime;
	sym4_t	member;
	N_SI4	validtime1;
	N_SI4	validtime2;
	sym8_t	plane1;
	sym8_t	plane2;
	sym8_t	element;
} nusdims_t;

/** @brief データセット設定
 *
 * データセット毎に設定されうる内容. データセット毎にコピーが作られる.
 */
struct nusxds_param_t {
	const N_UI1	*dsp_pc_mask_bit;
	sym4_t	dsp_pc_packing;
	N_SI4	dsp_pc_sizex;
	N_SI4	dsp_pc_sizey;
	/** @brief 書き込みバッファ長 (kbyte) */
	N_UI4	dsp_pc_wbuffer;
	/** @brief 読み込みバッファ長 (バイト数を得るために 1 をシフトする数) */
	N_UI4	dsp_pc_rbuffer;
	/** @brief ファイルを開く
	 *
	 * デフォルトは pio_open() である。
	 * オプション設定により sio_open(), pio_open(), eio_open() 等に
	 * 切替えられる。
	 * @note 本当は関数ポインタなのだが io_comm.h 依存性を排除するため
	 * 隠蔽している。
	 */
	union nusio_t *(*dsp_io_open)(const char *filename, int flags);
};

#define DynParam(param, name)	((param)->dsp_ ## name)
