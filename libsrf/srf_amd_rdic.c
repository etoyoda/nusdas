/* -------------------------------------------------------------------------- */
/* アメダスＢＵＦＲ電報をデコードするプログラム                               */
/* -------------------------------------------------------------------------- */
										/* サブルーチン集                                 Ver.1 1999.09.30 N.Fuji     *//* -------------------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include "nusdas.h"
#include "srf_amedas.h"
#include "nwpl_capi.h"

#ifdef TESTHDR
#include "libsrf.h"
#endif

/** @brief 地点番号の辞書内通番を探す
 *
 * SRF_AMD_SINFO 構造型の配列 @p amd (地点数 @p ac 個) から
 * 地点番号 @p stn の地点情報を収めた添字 (1始まり) を返す。
 *
 * @retval 正 地点の辞書内格納順位 (1始まり)
 * @retval -1 地点がみつからない
 *
 * <H3>注意</H3>
 * <UL>
 * <LI>地点種別 @p amd_type は無視される。
 * <LI>配列が地点番号順にソートされていることを前提に二分探索を使っている。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NAPS7 時代から存在したようであるが
 * ドキュメントされていなかった。
 * NuSDaS 1.3 リリースに際してドキュメントされるようになった。
 */

int
srf_search_amdstn(const SRF_AMD_SINFO * amd, /**< 地点辞書配列 */
		int ac, /**< 地点辞書配列の長さ */
		int stn, /**< 地点番号 */
		int amd_type UNUSED /**< 地点種別 */)
{
    int i, j, k;
    i = ac / 2;
    j = (i + 1) / 2;
    k = 0;
    while (1) {
	if (stn == (amd + i)->snum)
	    break;
	else if (stn > (amd + i)->snum)
	    i += j;
	else if (stn < (amd + i)->snum)
	    i -= j;
	if (k > 3)
	    return (-1);
	j = (j + 1) / 2;
	if (j == 1)
	    k++;
	if (i > ac - 1)
	    i = ac - 1;
	else if (i < 0)
	    i = 0;
    }
    return (i + 1);
}

/** @brief アメダス地点辞書の読み込み
 *
 * 環境変数 NWP_AMDDCD_STDICT が指す地点辞書ファイル
 * (環境変数未定義時は amddic.txt となる) から
 * SRF_AMD_SINFO 構造型の配列 @p amd にアメダス地点情報を読出す。
 * 読出される地点は時刻 @p btime に存在するものが選ばれ、
 * さらに引数 @p amd_type によって次のように限定される。
 * 配列長 @p amdnum を越えて書き出すことはない。
 *
 * <DL>
 * <DT>SRF_KANS<DD>官署
 * <DT>SRF_ELM4<DD>4要素を観測している地点
 * <DT>SRF_AMEL<DD>ロボット雨量計
 * <DT>SRF_AIRP<DD>航空官署
 * <DT>SRF_YUKI<DD>積雪観測地点
 * <DT>SRF_ALL<DD>全地点
 * </DL>
 *
 * @retval 非負 地点数
 * @retval -1 地点種別が不正
 * @retval -2 結果格納配列の長さ不足
 * @retval -3 地点辞書ファイルが開けない
 *
 * <H3>参考</H3>
 * NAPS8 においては地点辞書は
 * /grpK/nwp/Open/Const/Pre/Dcd/amddic.txt に置かれている。
 * <H3>履歴</H3>
 * この関数は NAPS7 時代から存在した。
 */
int
srf_amd_rdic(SRF_AMD_SINFO * amd /**< 地点辞書格納配列 */,
	     int amdnum, /**< 地点辞書配列長 */
	     int btime, /**< 探索日時 (通算分) */
	     int amd_type /**< 地点種別 */)
{
    FILE *fp;
    char s[100], *env, fname[200], atype, name1[11], name2[15], ws[80];
    int i, cnt, num, ilat[3], ilon[3], height;
    int sy, sm, sd, sh, sf, ey, em, ed, eh, ef, st, et;

    switch (amd_type) {
	    case SRF_KANS:
	    case SRF_ELM4:
	    case SRF_AMEL:
	    case SRF_AMEN:
	    case SRF_AIRP:
	    case SRF_YUKI:
	    case SRF_ALL:
		    break;
	    default:
		    fprintf(stderr, "srf_amd_rdic: your selection type "
				    "<%d> is not defined!\n", amd_type);
		    return -1;
    }
    /* 辞書ファイルの読み込み */
    if ((env = getenv("NWP_AMDDCD_STDICT")) != NULL) {
	strcpy(fname, env);
    } else {
	strcpy(fname, "amddic.txt");
    }
    if ((fp = fopen(fname, "r")) == NULL) {
	fprintf(stderr, "srf_amd_rdic: amedas dictionary "
		"file (%s) open failed\n", fname);
	return -3;
    }

    cnt = 0;
    while (1) {
	if (fgets(s, 100, fp) == NULL) {
	    break;
	}
	if (s[0] == '#') {
	    continue;
	}
	if (sscanf(s, " %5d %2d.%2d.%1d %3d.%2d.%1d %d  %c %79s",
		   &num, &ilat[0], &ilat[1], &ilat[2], &ilon[0], &ilon[1],
		   &ilon[2], &height, &atype, ws) != 10
	    || sscanf(&s[40], "%10s%14s", name1, name2) != 2) {
	    fprintf(stderr, "amedas dictionary file line error!\n");
	    fprintf(stderr, "%s\n", s);
	    continue;
	}
	if (amd_type != SRF_ALL) {
	    if (amd_type == SRF_KANS && atype != '@')
		continue;
	    if (amd_type == SRF_ELM4 && atype != '#')
		continue;
	    if (amd_type == SRF_AMEL && atype != '\xDB')
		continue;
	    if (amd_type == SRF_AMEN && atype != '!')
		continue;
	    if (amd_type == SRF_AIRP && atype != 'A')
		continue;
	    if (amd_type == SRF_YUKI && atype != '*')
		continue;
	}
	if (sscanf(&s[65], "%4d%2d%2d%2d%2d %4d%2d%2d%2d%2d",
		   &sy, &sm, &sd, &sh, &sf, &ey, &em, &ed, &eh,
		   &ef) != 10) {
	    fprintf(stderr,
		    "srf_amd_rdic: amedas dictionary file line error!\n");
	    fprintf(stderr, ">>>>> %s\n", s);
	    continue;
	}
	st = nwp_ymdhm2seq(sy, sm, sd, sh, sf);
	et = nwp_ymdhm2seq(ey, em, ed, eh, ef);
	if (btime < st || btime > et) {
	    continue;
	}

	(amd + cnt)->snum = num;
	(amd + cnt)->lat =
	    ilat[0] + ((float) ilat[1] + (float) ilat[2] / 10) / 60;
	(amd + cnt)->lon =
	    ilon[0] + ((float) ilon[1] + (float) ilon[2] / 10) / 60;
	if (atype == '@')
	    (amd + cnt)->type = SRF_KANS;
	else if (atype == '#')
	    (amd + cnt)->type = SRF_ELM4;
	else if (atype == '\xDB')
	    (amd + cnt)->type = SRF_AMEL;
	else if (atype == '!')
	    (amd + cnt)->type = SRF_AMEN;
	else if (atype == 'A')
	    (amd + cnt)->type = SRF_AIRP;
	else if (atype == '*')
	    (amd + cnt)->type = SRF_YUKI;
	(amd + cnt)->hh = (float) height;
	if (strncmp(ws, "     ", 5) == 0)
	    (amd + cnt)->wh = 6.4;
	else
	    sscanf(ws, "%f", &((amd + cnt)->wh));
	strcpy((amd + cnt)->name1, name1);
	for (i = strlen(name1); i < 10; i++)
	    (amd + cnt)->name1[i] = ' ';
	strcpy((amd + cnt)->name2, name2);
	for (i = strlen(name2); i < 14; i++)
	    (amd + cnt)->name2[i] = ' ';
	if (++cnt >= amdnum) {
	    fprintf(stderr,
		    "srf_amd_rdic : your srf_amd_sinfo dimension <%d> is insufficient!\n",
		    amdnum);
	    return -2;
	}
    }
    fclose(fp);

    return cnt;
}
