#include <nusdas.h>
#include <string.h>
#include <stdio.h>

#ifdef TESTHDR
#include "libsrf.h"
#endif

/** @brief AQCのパックを展開
 *
 * アメダス デコード データセットに含まれる AQC から
 * 要素名 @p param で指定される各ビットフィールドを取り出す。
 * <DL>
 * <DT>UNYOU△<DD>入電・休止・運用情報 (-1:休止, 0:入電無し, 正:入電回数)
 * <DT>RRfr0△<DD>降水量の情報
 * (0:入電無し, 1:ハードエラー・欠測・休止, 2:AQC該当値, 3:正常値; 以下同じ)
 * <DT>SSfr0△<DD>日照時間の情報
 * <DT>T△△△△△<DD>気温の情報
 * <DT>WindD△<DD>風向の情報
 * <DT>WindS△<DD>風速の情報
 * <DT>SnowD△<DD>積雪深の情報
 * </DL>
 * 
 * <H3>注意</H3>
 * 要素名が不正な場合は警告後なにもせず終了する。
 * (NuSDaS 1.3 より前は不定動作)
 * <H3>履歴</H3>
 * この関数は NAPS7 時代から存在した。
 */

void
srf_amd_aqc(const N_UI2 aqc_in[], /**< AQC 配列 */
		int num, /**< 配列要素数 */
		N_SI2 aqc_out[], /**< 結果配列 */
		const char *param /**< 要素名 */)
{
    int i, shift_bit;

    if (strncmp(param, "UNYOU ", 6) == 0) {
	for (i = 0; i < num; i++) {
	    if (aqc_in[i] & 0x0008) {
	        aqc_out[i] = -1;
	    } else {
	        aqc_out[i] = (aqc_in[i] & 0x0007);
	    }
	}
	return;
    } else if (strncmp(param, "RRfr0 ", 6) == 0) {
	shift_bit = 14;
    } else if (strncmp(param, "SSfr0 ", 6) == 0) {
	shift_bit = 12;
    } else if (strncmp(param, "T     ", 6) == 0) {
	shift_bit = 10;
    } else if (strncmp(param, "WindD ", 6) == 0) {
	shift_bit = 8;
    } else if (strncmp(param, "WindS ", 6) == 0) {
	shift_bit = 6;
    } else if (strncmp(param, "SnowD ", 6) == 0) {
	shift_bit = 4;
    } else {
	fprintf(stderr, "srf_amd_aqc: invalid element name %.6s\n", param);
	return;
    }
    for (i = 0; i < num; i++) {
        aqc_out[i] = (short) ((aqc_in[i] >> shift_bit) & 0x0003);
    }
}
