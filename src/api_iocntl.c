/** @file
 * @brief nusdas_iocntl() の実装
 */
#include "config.h"
#include "nusdas.h"

#include <stddef.h>
#include "internal_types.h"
#include "sys_time.h"
#include "glb.h"
#include "sys_err.h"

/** @brief 入出力フラグ設定
 *
 * 入出力にかかわるフラグを設定する.
 * <DL>
 * <DT>N_IO_MARK_END<DD>既定値1.
 * 零にすると nusdas_write() などの出力関数を呼び出すたびに
 * データファイルへの出力を完結させ END 記録を書き出すのをやめる.
 * <DT>N_IO_W_FCLOSE<DD>既定値1.
 * 零にすると nusdas_write() などの出力関数を呼び出すたびに
 * 書き込み用に開いたファイルを閉じるのをやめる.
 * 速度上有利だが、データファイルの操作が終了した後で
 * ファイルを閉じる関数 nusdas_allfile_close() または
 * nusdas_onefile_close() を適切に呼んでファイルを閉じないと
 * 出力ファイルが不完全となり、後で読むことができない.
 * なお、このフラグを変更すると N_IO_MARK_END も連動する.
 * <DT>N_IO_R_FCLOSE<DD>既定値1.
 * 零にすると nusdas_read() などの入力関数を呼び出すたびに
 * 読み込み用に開いたファイルを閉じるのをやめる.
 * 速度上有利だが、多数のファイルから入力するプログラムでは
 * ファイルハンドルが枯渇する懸念があるので
 * ファイルを明示的に閉じることが推奨される.
 * <DT>N_IO_WARNING_OUT<DD>既定値1.
 * 0 にするとエラーメッセージだけが出力される.
 * 1 にすると、それに加えて警告メッセージも出力されるようになる.
 * 2 にすると、それに加えてデバッグメッセージも出力されるようになる.
 * <DT>N_IO_BADGRID<DD>既定値0.
 * 1 にすると投影法パラメタの検査で不適切な値が検出されても
 * データファイルが作成できるようになる。
 * </DL>
 *
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.0 から存在した.
 * <B>N_IO_WARNING_OUT</B> の値 2 は NuSDaS 1.3 の拡張である.
 * <B>N_IO_BADGRID</B> も NuSDaS 1.3 の拡張である.
 * */
	N_SI4
NuSDaS_iocntl(N_SI4 param, /**< 設定項目コード */
		N_SI4 value) /**< 設定値 */
{
	/** @note この関数はライブラリの初期化をしない.
	 * */
	switch (param) {
		case N_IO_MARK_END:
			GlobalConfig(io_mark_end) = (value ? 1 : 0);
			break;
		case N_IO_W_FCLOSE:
			GlobalConfig(io_mark_end) =
			GlobalConfig(io_w_fclose) = (value ? 1 : 0);
			break;
		case N_IO_R_FCLOSE:
			GlobalConfig(io_r_fclose) = (value ? 1 : 0);
			break;
		case N_IO_WARNING_OUT:
			nus_wrn_enabled = !!value;
			nus_dbg_enabled = (value > 1);
			break;
		case N_IO_BADGRID:
			GlobalConfig(io_badgrid) = (value ? 1 : 0);
			break;
		default:
			return -1;
	}
	return 0;
}
