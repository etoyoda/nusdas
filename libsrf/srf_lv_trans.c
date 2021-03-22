#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <nusdas.h>

#ifdef TESTHDR
#include "libsrf.h"
#endif

static void
fill_lowercase_upto4(char dest[5], const char *source)
{
    int i;
    for (i = 0; i < 4; i++) {
	int c = ((unsigned char *) source)[i];
	dest[i] = tolower(c);
	if (c == '\0') {
	    break;
	}
    }
    dest[4] = '\0';		/* failsafe */
}

/** @brief 実数からレベル値への変換
 *
 * 配列 @p fdat の実数データをレベル値 @p idat に変換し、
 * ISPC 配列 @p ispec にレベル代表値をセットする。
 * 変換テーブルとしてファイル ./SRF_LV_TABLE/param.ltb を読む。
 * ここで @p param は変換テーブル名 (最長 4 字) である。
 *
 * @retval -1 変換テーブルを開くことができない
 * @retval -2 変換テーブルに 256 以上のレベルが指定されている
 * @retval 正 変換に成功した。返却値はレベル数
 * 
 * <H3>注</H3>
 * <UL>
 * <LI>
 * NAPS7 では変換テーブルとして
 * her ier prr pmf srr srf pr2
 * を用意していた。
 * NAPS8 では <BR>
 * /grpK/nwp/Open/Const/Vsrf/Comm/lvtbl.txd 以下に
 * her ie2 ier kor pft pm2 pmf pr2 prr sr1 sr2 sr3 srf srj srr yar yrr
 * が置かれている。
 * ルーチンジョブではこのディレクトリへシンボリックリンク SRF_LV_TABLE
 * を張って利用する。
 * <LI>
 * 変換テーブル名が ie2, kor, pft のとき,
 * ISPEC には変換テーブルに書かれた代表値の 1/10 が書かれる。
 * それ以外の場合は変換テーブルの代表値がそのまま書かれる。
 * <LI>
 * 変換テーブル名が sr2 または srj のときは実数データが -900.0 より小さい
 * ものが欠損値とみなされる。そうでなければ、負値が欠損値とみなされる
 * (NAPS7 のマニュアルでは欠損値は -1 を指定することとされている)。
 * <LI>
 * 変換テーブル名が srj のときは、実数データが変換テーブルの下限値に
 * 正確に一致しないと最も上の階級 (具体的には 42 で 21.0を意味する)
 * に割り当てられる。この挙動はバグかもしれない。
 * <LI>
 * 変換テーブルに 191 行以上書かれているとき、
 * 最初の 190 行だけが用いられ、レベル値は 0..190 となるが、
 * 返却値には実際のレベル数 (変換テーブルの行数 + 1) が返される。
 * これは ispec の配列をオーバーフローしないためである。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NAPS7 時代から存在した。
 * Fortran ラッパーが文字列の長さを伝えないバグは NuSDaS 1.3 で解決した。
 */
int
srf_lv_set(N_SI4 idat[],		/**< INTENT(OUT) レベル値格納配列 */
	   const float fdat[],	/**< 変換元データ配列 */
	   int dnum,	  	/**< データ配列要素数 */
	   N_SI4 ispec[],	        /**< 新 ISPC */
	   const char *param    /**< 変換テーブル名 */)
{
    char fname[256], s1[80], s2[80];
    char line[256], s3[80];
    char param_lc[5];
    short rep_v[256], nlevs, nlevs_real, lv0, *sp;
    int i, j, order;
    float low_v[256], w;
    FILE *fp;
    fill_lowercase_upto4(param_lc, param);
    sprintf(fname, "./SRF_LV_TABLE/%s.ltb", param_lc);
    if ((fp = fopen(fname, "r")) == NULL) {
	fprintf(stderr,
		"srf_lv_set: level table file <%s> can not be opened\n",
		fname);
	return (-1);
    }
    if (strcmp(param, "ie2") == 0 || strcmp(param, "kor") == 0
		    || strcmp(param, "pft") == 0) {
	order = 1;
    } else {
	order = 10;
    }
    nlevs = 0;
    while (fgets(line, sizeof line, fp)) {
	/* scanf の使い方
	 * %s は空白区切りの文字列を読む指示なので
	 * "%s %s" のように間にスペースを入れなくてよい。
	 * %79s と書かないと 80 文字以上書かれていると配列侵害を起こす。
	 * */
	if (sscanf(line, "%79s%79s%79s", s1, s2, s3) < 3) {
   	    continue;
	}
	if (s1[0] == '#') {
	    continue;
	}
	low_v[nlevs] = (float)atoi(s2);
	rep_v[nlevs] = atoi(s3) / order;
	if (++nlevs >= 256) {
	    fprintf(stderr,
		    "srf_lv_set: number of level for %s exceeds 256\n",
		    param);
	    return (-2);
	}
    }
    fclose(fp);
    /* nlevs_real はファイルから読み取ったレベル数そのもの
     * ispec[33] から ispec[127] には (128 - 33) * 4 / 2 = 190 個しか
     * 書けないので nlevs は 191 を上限とする (1 は lv0 用)
     */
    nlevs++;
    nlevs_real = nlevs;
    if (nlevs > 191) {
	nlevs = 191;
    }

    if (strcmp(param, "sr2") == 0) {
	sp = (short *) (&ispec[32]);
	memcpy(sp, &nlevs, sizeof(short));
	lv0 = -1;
	memcpy((short *) (sp + 1), &lv0, sizeof(short));
	memcpy((short *) (sp + 2), rep_v, sizeof(short) * (nlevs - 1));
	for (i = 0; i < dnum; i++) {
	    w = fdat[i] * 100;
	    if (w < -90000)
		idat[i] = 0;
	    else {
		for (j = 0; j < 63; j++)
		    if ((w == low_v[j] && w == low_v[j + 1]) ||
			(w >= low_v[j] && w < low_v[j + 1])) {
			idat[i] = j + 1;
			break;
		    }
		if (j == 63) {
		    if (w >= low_v[j])
			idat[i] = j + 1;
		    else {
			for (j = 63; j < 127; j++)
			    if ((w == low_v[j] && w == low_v[j + 1]) ||
				(w >= low_v[j + 1] && w < low_v[j])) {
				idat[i] = j + 2;
				break;
			    }
			if (j == 127)
			    idat[i] = 127;
		    }
		}
	    }
	}
    } else if (strcmp(param, "srj") == 0) {
	sp = (short *) (&ispec[32]);
	memcpy(sp, &nlevs, sizeof(short));
	lv0 = -1;
	memcpy((short *) (sp + 1), &lv0, sizeof(short));
	memcpy((short *) (sp + 2), rep_v, sizeof(short) * (nlevs - 1));
	for (i = 0; i < dnum; i++) {
	    w = fdat[i] * 100;
	    if (w < -90000)
		idat[i] = 0;
	    else {
		for (j = 0; j < nlevs - 1; j++)
		    if (w == low_v[j]) {
			idat[i] = j + 1;
			break;
		    }
		if (j == nlevs - 1) {
		    idat[i] = j;
		}
	    }
	}
    } else {
	low_v[nlevs - 1] = FLT_MAX;
	sp = (short *) (&ispec[32]);
	memcpy(sp, &nlevs, sizeof(short));
	lv0 = -1;
	memcpy((short *) (sp + 1), &lv0, sizeof(short));
	memcpy((short *) (sp + 2), rep_v, sizeof(short) * (nlevs - 1));
	for (i = 0; i < dnum; i++) {
	    w = fdat[i] * 100;
	    if (w < 0)
		idat[i] = 0;
	    else {
		for (j = 0; j < nlevs - 1; j++) {
		    if ((w == low_v[j] && w == low_v[j + 1]) ||
			(w >= low_v[j] && w < low_v[j + 1])) {
			idat[i] = j + 1;
			break;
		    }
		}
		/* 次の if 節はルーチン版ではコメントアウトされているが、
		 * w == FLT_MAX の時はやっぱりここに落ちる */
		if (j == nlevs - 1) {
		    idat[i] = j;
		}
	    }
	}
    }
    endian_swab2(&ispec[32], (128 - 32) * 2);
    endian_swab4(&ispec[32], (128 - 32));
    return nlevs_real;
}

/** @brief レベル値を実数データ (代表値) に変換
 *
 * 新 ISPEC 配列 @p ispec にしたがって
 * 配列 @p idat のレベル値を代表値 @p fdat に変換する。
 *
 * <H3>返却値</H3>
 * 不明値以外となったデータの要素数
 *
 * <H3>注</H3>
 * <UL>
 * <LI>
 * 不明値は -1 となる。
 * ただし、ISPEC のデータ種別 (先頭4バイト) が
 * SRR2, SRF2, SRRR, SRFR の場合に限り -9999.0 となる。
 * <LI>
 * ISPEC のレベル表は通常 0.1mm 単位と解釈される。
 * ただし、ISPEC の先頭 3 バイトが `<TT>IER</TT>' であるか、
 * あるいは ISPEC の先頭から 4 バイト目が `<TT>1</TT>' のときは
 * 0.01mm 単位と解釈される。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NAPS7 時代から存在した。
 */
int
srf_lv_trans(const N_SI4 idat[], /**< 入力データ */
	     float fdat[],     /**< INTENT(OUT) 結果格納配列 */
	     N_SI4 dnum,	       /**< データ要素数 */
	     const N_SI4 ispec[] /**< ISPEC 配列 */)
{
    int i, lnum, rt = 0;
    short *sdat;
    float fdat_0, rev_order;
    unsigned char *cdat;
    int tmp_ispec[128];

    memcpy(tmp_ispec, ispec, 128 * sizeof(int));
    endian_swab4(&tmp_ispec[32], (128 - 32));
    endian_swab2(&tmp_ispec[32], (128 - 32) * 2);
    endian_swab4(&tmp_ispec[0], 1);
    sdat = (short *) (&tmp_ispec[32]);
    lnum = *sdat;
    cdat = (unsigned char *) (&tmp_ispec[0]);
    /* "1" in 4th byte of ispec[0] means order=100 */
    rev_order = ((memcmp(cdat, "IER", 3) == 0 && lnum > 20) || (cdat[3] == '1')) ? 0.01f : 0.1f;
    fdat_0 = (memcmp(cdat, "SRR2", 4) == 0 ||
	      memcmp(cdat, "SRF2", 4) == 0 ||
	      memcmp(cdat, "SRRR", 4) == 0 ||
	      memcmp(cdat, "SRFR", 4) == 0) ? -9999.0f : -1.0f;
    for (i = 0; i < dnum; i++) {
	if (idat[i] == 0) {
		fdat[i] = fdat_0;
	        rt++;
	} else if (idat[i] > 0 && idat[i] < lnum) {
	    fdat[i] = (float) (*(sdat + idat[i] + 1)) * rev_order;
	    rt++;
	} else {
	    fprintf(stderr,
		    "srf_lv_trans: input data level <%d> is out of range\n",
		    idat[i]);
	    fprintf(stderr,
		    "No.%-d output data is set to missing value -1\n", i);
	    fdat[i] = -1.0f;
	}
    }
    return (rt);
}

/** @brief レベル値から代表値への変換
 *
 * 配列 @p idat のレベル値を代表値 @p fdat に変換する。
 * 変換テーブルとしてファイル ./SRF_LV_TABLE/param.ltb を読む。
 * ここで @p param は変換テーブル名 (最長 4 字) である。
 *
 * @retval -1 変換テーブルを開くことができない
 * @retval -2 変換テーブルに 256 以上のレベルが指定されている
 * @retval 非負 変換に成功。返却値は不明値以外となったデータの要素数
 *
 * <H3>注</H3>
 * <UL>
 * <LI>不明値は -1 となる。
 * <LI>
 * NAPS8 では変換テーブルとして
 * /grpK/nwp/Open/Const/Vsrf/Comm/lvtbl.txd 以下に
 * her ie2 ier kor pft pi10 pm2 pmf pr2 prr rr60
 * sr1 sr2 sr3 srf srj srr yar yrr
 * が置かれている。
 * ルーチンジョブではこのディレクトリへシンボリックリンク SRF_LV_TABLE
 * を張って利用する。
 * <LI>
 * 上記変換テーブルのうち、
 * pi10 と rr60 は1行にレベル値と代表値の2列が書かれており、
 * その他はレベル値、最小値、代表値の3列が書かれているが、
 * 本サブルーチンはどちらにも対応している。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NAPS7 時代には存在しなかったようである。
 * レーダー情報作成装置に関連して開発されたと考えられているが、
 * NuSDaS 1.3 以前にはきちんとメンテナンスされていなかった。
 */
int
rdr_lv_trans(N_SI4 idat[], /**< 入力データ */
		float fdat[], /**< INTENT(OUT) 結果格納配列 */
		int dnum, /**< データ要素数 */
		const char *param /**< テーブル名 */)
{
    int i, rt = 0;
    short rep_v[256], k;
    char fname[256], s1[80], s2[80];
    char line[256], s3[80];
    char param_lc[5];
    FILE *fp;
    fill_lowercase_upto4(param_lc, param);
    sprintf(fname, "./SRF_LV_TABLE/%s.ltb", param_lc);
    if ((fp = fopen(fname, "r")) == NULL) {
	fprintf(stderr,
		"rdr_lv_set: level table file <%s> can not be opened\n",
		fname);
	return (-1);
    }
    k = 0;
    while (fgets(line, sizeof line, fp)) {
	int nwords;
	nwords = sscanf(line, "%79s%79s%79s", s1, s2, s3);
	if (nwords < 2 || s1[0] == '#') {
	    continue;
	}
	if (nwords >= 3) {
	    rep_v[k] = atoi(s3);
	} else {
	    rep_v[k] = atoi(s2);
	}
	if (++k >= 256) {
	    fprintf(stderr,
		    "srf_lv_set: number of level for %s exceeds 256\n",
		    param);
	    return (-2);
	}
    }
    fclose(fp);

    for (i = 0; i < dnum; i++) {
	if (idat[i] < 0 || idat[i] > k - 1) {
	    if (idat[i] != 255) {
		fprintf(stderr,
			"srf_lv_trans: input data level <%d> is out of range\n",
			idat[i]);
		fprintf(stderr,
			"No.%-d output data is set to missing value -1\n",
			i);
	    }
	    fdat[i] = -1;
	} else {
	    fdat[i] = (float) rep_v[idat[i]] / 100;
	    rt++;
	}
    }
    return (rt);
}
