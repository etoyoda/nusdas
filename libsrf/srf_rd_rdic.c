/* ************************************************************************** */
/* subroutine for read radar dictionary                                       */
/* ************************************************************************** */
#include <nusdas.h>
#include <nwpl_capi.h>
#include <stdio.h>

#ifdef TESTHDR
#include "libsrf.h"
#endif

/** @brief 改行まで読み飛ばす
 *
 * ストリーム @p fp から改行文字が読み取られるまで1バイトづつ読み飛ばす。
 * 
 */

char
srf_new_line(FILE * fp)
{
    /* C 言語教室: fgetc の結果を決して char で受けてはならない。
     * char が符号付きなら文字コード 0xFF (DEL) と EOF の区別がつかなく
     * なるし、char が符号なしなら EOF を検知できなくなって無限ループに
     * なるからである。
     */
    int c;
    while ((c = fgetc(fp)) != EOF && c != '\n')
	continue;
    return c;
}

/** @brief 度分秒から度への換算
 *
 * 度分秒形式で表現された整数 (一十の桁が秒、百千の桁が分、万の桁が度)
 * から度単位の実数を与える。
 */

float
srf_i2f_latlon(int ll)
{
    int i, j, k;
    i = ll / 10000;
    j = (ll - i * 10000) / 100;
    k = (ll - i * 10000 - j * 100);
    return i + j / 60.0f + k / 3600.0f;
}

/** @brief レーダーサイト情報の問い合わせ
 *
 * ファイル名 RADAR_DIC のレーダー地点辞書から
 * 日時 @p iseq (通算時であって通算分でないことに注意)
 * における地点番号 @p stnum のレーダーサイトの情報を読出す。
 * 
 * @retval 1 正常終了
 * @retval 0 指定されたレーダーサイトがみつからなかった
 * @retval -1 レーダー地点辞書が開けなかった
 *
 * <H3>注</H3>
 * <UL>
 * <LI>レーダー地点辞書は NAPS8 では
 * /grpK/nwp/Open/Const/Vsrf/Dcd/rdrdic.txt に置かれている。
 * <LI>NAPS8 初期版 libsrf.a には経度のかわりに誤って緯度を返すバグがある。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NAPS7 時代からルーチン環境には存在したが、
 * pnusdas から NuSDaS 1.1 に至る CVS 版ソースには含まれていなかった。
 * NuSDaS 1.3 で両者が統合された。
 */

int
srf_rd_rdic(int stnum, /**< 地点番号 */
		int iseq, /**< 日時(通算時) */
		float *lat, /**< INTENT(OUT) 緯度 */
		float *lon, /**< INTENT(OUT) 経度 */
		float *hh, /**< INTENT(OUT) 高度 */
		N_SI4 *offx, /**< INTENT(OUT) 中心のオフセット */
		N_SI4 *offy, /**< INTENT(OUT) 中心のオフセット */
		N_SI4 *type1, /**< INTENT(OUT) デジタル化タイプ */
		N_SI4 *type2 /**< INTENT(OUT) デジタル化タイプ */)
{
    FILE *fp;
    char line[256], stime[16], etime[16];
    char name[80];
    int num, iseqs, iseqe;
    int iy, im, id, ih, ilat, ilon, ihh;
    int set_flag = 0;

    if ((fp = fopen("RADAR_DIC", "r")) == NULL) {
	fprintf(stderr,
		"srf_rd_rddic: "
		"Can not open radar dictionary file <RADAR_DIC>\n");
	return (-1);
    }
    /* レーダー地点辞書の書式

....v....1....v....2....v....3....v....4....v....5....v....6....v....7....v
#### stnum      start         end       lat     lon  height offx offy type
SAPP 47415 1999.03.01.00 2999.12.31.23 d30820 1410035  7490   40  140 1 3
%s-> %d--> %s----------> %s----------> %d---> %d---->  %d->   %d  %d> %d%d

     */
    while (fgets(line, sizeof line, fp) != NULL) {
        int nfields, ox, oy, t1, t2;
	nfields = sscanf(line, "%79s%d%15s%15s%d%d%d%d%d%d%d",
				name, &num, stime, etime,
				&ilat, &ilon, &ihh, &ox, &oy, &t1, &t2);
	if (nfields != 11 || name[0] == '#') {
	    continue;
	}
	if (stnum != num) {
	    continue;
	}
	if (sscanf(stime, "%d.%d.%d.%d", &iy, &im, &id, &ih) != 4)
	    break;
	iseqs = nwp_ymdh2seq(iy, im, id, ih);
	if (sscanf(etime, "%d.%d.%d.%d", &iy, &im, &id, &ih) != 4)
	    break;
	iseqe = nwp_ymdh2seq(iy, im, id, ih);
	if (iseq < iseqs || iseq > iseqe) {
	    continue;
	}
	*lat = srf_i2f_latlon(ilat);
	*lon = srf_i2f_latlon(ilon);
	*hh = ihh / 10.0f;
	*offx = ox;
	*offy = oy;
	*type1 = t1;
	*type2 = t2;
	set_flag = 1;
	break;
    }
    fclose(fp);
    return set_flag;
}
