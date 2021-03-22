/** @file
 * @brief nusdas_parameter_change() の実装
 *
 * @note SIZEX, SIZEY の型が SI4 であるかどうか疑問。
 */
#include "config.h"
#include "nusdas.h"

#include <stddef.h>
#include "internal_types.h"
#include "sys_err.h"
#include "glb.h"

/** @brief 旧ヘッダとの混在防止用ダミー
 *
 * NuSDaS 1.2 までは nusdas_fort.h において
 *
 *   INTEGER:: NULL
 *   COMMON /NUSDAS/ NULL
 *
 * のように宣言しており、当時の nusdas_parameter_change() の実装では
 * 移植性に疑問がある hack によって第二引数とこのコモンブロックのポインタ
 * を比較して偽ヌルを判定していた。
 *
 * NAPS8 において NuSDaS 1.1 と NuSDaS 1.3 が併存するため、旧ヘッダファイル
 * を用いてコンパイルされたオブジェクトモジュールが新ライブラリをリンク
 * しないように以下の関数が定義されている。
 * 
 * 旧ヘッダファイルによるオブジェクトモジュールにはデータセクションに
 * NUSDAS (Fortran コンパイラによっては小文字など) というシンボルが
 * できる。コンパイラによってはデータセクションとは違うかもしれないが
 * いくらなんでもテキストセクションということはないだろう。
 * 一方、このソースコードはテキストセクションに NUSDAS 等のシンボルを
 * 定義する。少なくとも日立最適化 Fortran ではこれによってリンカが
 * 下のようなエラーを起こすことにより問題を判定することができる。
 * 他のコンパイラでは警告どまりのことが多い (少なくとも Linux ifort/g77)
 * か、あるいは何も起こらないかもしれない (それがこの機能が廃止された理由)
 * が、ともあれ NAPS8 の安定運用が重視されるのだからないよりマシだろう。

ld: 0711-224 WARNING: Duplicate symbol: NUSDAS
ld: 0711-345 Use the -bloadmap or -bnoquiet option to obtain more information.

 */

#if 1
int NUSDAS(void) { return -1; }

/** @brief 関数 NUSDAS() に同じ */
int NUSDAS_(void) { return -1; }

/** @brief 関数 NUSDAS() に同じ */
int nusdas(void) { return -1; }

/** @brief 関数 NUSDAS() に同じ */
int nusdas_(void) { return -1; }
#else
/* 将来上記の意地悪が都合が悪くなったら以下を用いよ。
 * これに移植性がある限り、Fortran API で NULL を実装しない言い訳が
 * 成り立たないのではあるが。
 * */
int NUSDAS, NUSDAS_, nusdas, nusdas_;
#endif

/** @brief オプション設定
 *
 * @p param で指定されるパラメータに値 @p value を設定する。
 * 整数値の項目については、
 * 互換性のため値ゼロのかわりに名前 N_OFF を用いることができる。
 * <DL>
 * <DT>N_PC_MISSING_UI1<DD>1バイト整数の欠損値 (既定値: N_MV_UI1)
 * <DT>N_PC_MISSING_SI2<DD>2バイト整数の欠損値 (既定値: N_MV_SI2)
 * <DT>N_PC_MISSING_SI4<DD>4バイト整数の欠損値 (既定値: N_MV_SI4)
 * <DT>N_PC_MISSING_R4<DD>4バイト実数の欠損値 (既定値: N_MV_R4)
 * <DT>N_PC_MISSING_R8<DD>8バイト実数の欠損値 (既定値: N_MV_R8)
 * <DT>N_PC_MASK_BIT<DD>ビットマスク配列へのポインタ
 * (既定値は NULL ポインタだが Fortran では直接設定できないので
 * nusdas_parameter_reset() を用いられたい)
 * <DT>N_PC_SIZEX<DD>
 *  非零値を設定すると強制的にデータレコードの @p x 方向
 *  格子数を設定する (0)
 * <DT>N_PC_SIZEY<DD>
 *  強制格子サイズ:
 *  既定値 (0) 以外を設定するとデータレコードの @p y 方向
 *  格子数を設定する
 * <DT>N_PC_PACKING<DD>
 *  4文字のパッキング名を設定すると、定義ファイルの指定にかかわらず
 *  nusdas_write() 等データ記録書き込みの際に用いられる
 *  パッキング方式が変更される。
 *  既定値に戻す (定義ファイルどおりに書かせる) には
 *  4 バイト整数値 0 を設定する。
 * <DT>N_PC_ID_SET<DD>
 *  NRD 番号制約:
 *  既定値 (-1) 以外を設定すると、その番号の NRD だけを
 *  入出力に用いるようになる
 * <DT>N_PC_WBUFFER<DD>
 *  書き込みバッファサイズ (既定値: 0)
 *  実行時オプション FWBF に同じ。
 * <DT>N_PC_RBUFFER<DD>
 *  読み取りバッファサイズ (既定値: 17)
 *  実行時オプション FRBF に同じ。
 * <DT>N_PC_KEEP_CFILE<DD>
 *  ファイルを閉じたあと CNTL/INDX などのヘッダ情報をキャッシュしておく
 *  数。負にするとキャッシュを開放しなくなる（既定値: -1）。
 *  実行時オプション GKCF に同じ。
 * <DT>N_PC_OPTIONS<DD>
 *  設定のみでリセットはできない。
 *  ヌル終端した文字列を与えると実行時オプションとして設定する。
 *  Fortran インターフェイスでもヌル終端しなければならないことに注意。
 * </DL>
 *
 * @retval 0 正常終了
 * @retval -1 サポートされていないパラメタである
 *
 * <H3>履歴</H3>
 * NuSDaS 1.0 から存在する。
 *
 * NuSDaS 1.1 ではデータセット探索のキャッシュ論理に問題があり、
 * N_PC_ID_SET で NRD 番号制約をかけて入出力を行った後で
 * NRD 番号制約を解除して同じ種別にアクセスしても探索が行われない
 * (あらかじめ NRD 制約をかけずに入出力操作をしていれば探索される)。
 * この問題は NuSDaS 1.3 では解決されている。
 * */
	N_SI4
NuSDaS_parameter_change(N_SI4 param, /**< 設定項目コード */
		const void *value) /**< 設定値 */
{
	NUSDAS_INIT;
	if (value == NULL) {
		return NuSDaS_parameter_reset(param);
	}
	/* NuSDaS 1.1 Fortran 用ヘッダの NULL を使って
	 * 出来たポインタを一応解釈する。AIX ではリンクエラーだが
	 * Linux で通ってしまうので、やはりサポートするのが吉と思い
	 * なおした。機能しないかもしれないが、そこまでは保証しない。
	 */
	if (value == (const void *)NUSDAS
			|| value == (const void *)NUSDAS_
			|| value == (const void *)nusdas
			|| value == (const void *)nusdas_) {
		nus_warn(("deprecated: use of NULL in nusdas_fort.h"));
		return NuSDaS_parameter_reset(param);
	}
	switch (param) {
		case N_PC_MISSING_UI1:
			GlobalConfig(pc_missing_ui1) = *(const N_UI1 *)value;
			break;
		case N_PC_MISSING_SI2:
			GlobalConfig(pc_missing_si2) = *(const N_SI2 *)value;
			break;
		case N_PC_MISSING_SI4:
			GlobalConfig(pc_missing_si4) = *(const N_SI4 *)value;
			break;
		case N_PC_MISSING_R4:
			GlobalConfig(pc_missing_r4) = *(const float *)value;
			break;
		case N_PC_MISSING_R8:
			GlobalConfig(pc_missing_r8) = *(const double *)value;
			break;
		case N_PC_MASK_BIT:
			GlobalDSConfig(pc_mask_bit) = value;
			break;
		case N_PC_PACKING:
			GlobalDSConfig(pc_packing) = *(const N_SI4 *)value;
			break;
		case N_PC_SIZEX:
			GlobalDSConfig(pc_sizex) = *(const N_SI4 *)value;
			break;
		case N_PC_SIZEY:
			GlobalDSConfig(pc_sizey) = *(const N_SI4 *)value;
			break;
		case N_PC_ID_SET:
			GlobalConfig(nrd_override) = *(const N_SI4 *)value;
			break;
		case N_PC_WBUFFER:
			GlobalDSConfig(pc_wbuffer) = *(const N_SI4 *)value;
			break;
		case N_PC_RBUFFER:
			GlobalDSConfig(pc_rbuffer) = *(const N_SI4 *)value;
			break;
		case N_PC_KEEP_CFILE:
			GlobalConfig(pc_keep_closed_file)
				= *(const N_SI4 *)value;
			break;
		case N_PC_OPTIONS:
			if (value) {
				nusdas_opts(NULL, value);
			}
			break;
		default:
			NUSDAS_CLEANUP;
			return -1;
	}
	NUSDAS_CLEANUP;
	return 0;
}
