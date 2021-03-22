#include "nusdas.h"
#include <stdio.h>
#include <string.h>

#ifdef TESTHDR
#include "libsrf.h"
#endif

/** @brief 2配列のクイックソート
 *
 * 要素数 @p n_st_n 個の配列 @p st_n および配列 @p d_n
 * (後者の型は NuSDaS と同様に @p t_n で指示される N_R4, N_I4, または N_I2)
 * を @p st_n の値の順にソートする。
 *
 * <H3>注意</H3>
 * 上記 3 種類以外の型を指示すると、黙って何もしない。
 * NuSDaS 1.3 より前は、N_I2 を指定できなかった。
 */

void
srf_q_sort(N_SI4 *st_n, int n_st_n, void *d_n, const char *t_n)
{
    int gap, i, j, temp, *id, tid;
    short *sd, tsd;
    float *fd, tfd;
    sd = (short *) d_n;
    id = (int *) d_n;
    fd = (float *) d_n;
    if (strncmp(t_n, N_R4, 2) == 0) {
	for (gap = n_st_n / 2; gap > 0; gap /= 2)
	    for (i = gap; i < n_st_n; i++)
		for (j = i - gap; j >= 0 && st_n[j] > st_n[j + gap];
		     j -= gap) {
		    temp = st_n[j];
		    st_n[j] = st_n[j + gap];
		    st_n[j + gap] = temp;
		    tfd = fd[j];
		    fd[j] = fd[j + gap];
		    fd[j + gap] = tfd;
		}
    } else if (strncmp(t_n, N_I4, 2) == 0) {
	for (gap = n_st_n / 2; gap > 0; gap /= 2)
	    for (i = gap; i < n_st_n; i++)
		for (j = i - gap; j >= 0 && st_n[j] > st_n[j + gap];
		     j -= gap) {
		    temp = st_n[j];
		    st_n[j] = st_n[j + gap];
		    st_n[j + gap] = temp;
		    tid = id[j];
		    id[j] = id[j + gap];
		    id[j + gap] = tid;
		}
    } else if (strncmp(t_n, N_I2, 2) == 0) {
	for (gap = n_st_n / 2; gap > 0; gap /= 2)
	    for (i = gap; i < n_st_n; i++)
		for (j = i - gap; j >= 0 && st_n[j] > st_n[j + gap];
		     j -= gap) {
		    temp = st_n[j];
		    st_n[j] = st_n[j + gap];
		    st_n[j + gap] = temp;
		    tsd = sd[j];
		    sd[j] = sd[j + gap];
		    sd[j + gap] = tsd;
		}
    }
}

/** @brief 整数配列の二分探索
 *
 * 長さ @p n_st_n の配列 @p st_n (ソートされているものと仮定する) から
 * 値が @p st_r に一致する要素を見出しその添字 (0 始まり) を返す。
 *
 * @retval 非負 見付かった要素の添字
 * @retval -1 みつからなかった
 */

int
srf_amd_slct_sub(int st_r, const N_SI4 *st_n, int n_st_n)
{
    int i, j, k;
    i = (n_st_n + 1) / 2;
    j = 0;
    k = (n_st_n + 1) / 2;
    while (1) {
	if (st_r == st_n[i])
	    break;
	if (j == 2) {
	    i = -1;
	    break;
	} else
	    k = (k + 1) / 2;
	if (k == 1)
	    j++;
	if (st_r > st_n[i])
	    i += k;
	else
	    i -= k;
	if (i >= n_st_n)
	    i = n_st_n - 1;
	else if (i < 0)
	    i = 0;
    }
    return (i);
}

/** @brief アメダスデータを指定の地点番号順に並べる
 *
 * 長さ @p n_st_n の地点番号配列 @p st_n と
 * 対応する順に並んだ @p t_n 型の配列 @p d_n から、
 * 別の地点番号配列 @p st_r (要素数 @p n_st_r 個) の順に並んだ
 * @p t_r 型の配列 @p d_r (要素数 @p n_st_r 個) を作る。
 *
 * 配列 @p st_n と @p d_n があらかじめソートされている場合 @p sort_f に
 * N_OFF (nusdas.h で定義される) を指定する。
 * そうでない場合 @p sort_f に N_ON を指定するとソートされる。
 *
 * @retval 0 配列 @p st_r の全地点が見付かった
 * @retval 正 みつからなかった地点数
 *
 * <UL>
 * <LI>型は nusdas.h で定義される N_R4, N_I4, N_I2 のいずれかで指定する。
 * <LI>配列 @p st_r に含まれる地点番号が @p st_n で見付からない場合は
 * nusdas.h で定義される欠損値 N_MV_R4, N_MV_SI4, N_MV_SI2 が入る。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NAPS7 時代から存在した。
 */

int
srf_amd_slct(const N_SI4 *st_r, /**< 結果の順を指示する地点番号表 */
		int n_st_r, /**< 結果配列長 */
		void *d_r, /**< INTENT(OUT) 結果配列 */
		const char *t_r, /**< 結果配列の型 */
		N_SI4 *st_n, /**< 元データ地点番号配列 */
		int n_st_n, /**< 元データ配列長 */
		void *d_n, /**< 元データ配列 */
		const char *t_n, /**< 元データ配列の型 */
		int sort_f /**< 未ソートフラグ */)
{
    int i, j, k = 0;
    short *sd_r, *sd_n;
    int *id_r, *id_n;
    float *fd_r, *fd_n;

    if (sort_f == N_ON)
	srf_q_sort(st_n, n_st_n, d_n, t_n);

    sd_r = (short *) d_r;
    id_r = (int *) d_r;
    fd_r = (float *) d_r;
    sd_n = (short *) d_n;
    id_n = (int *) d_n;
    fd_n = (float *) d_n;

    if (strncmp(t_r, N_R4, 2) == 0) {
	if (strncmp(t_n, N_R4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		fd_r[j] = (i == -1) ? N_MV_R4 : fd_n[i];
	    }
	} else if (strncmp(t_n, N_I4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		fd_r[j] = (i == -1) ? N_MV_R4 : (float) id_n[i];
	    }
	} else if (strncmp(t_n, N_I2, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		fd_r[j] = (i == -1) ? N_MV_R4 : (float) sd_n[i];
	    }
	}
    } else if (strncmp(t_r, N_I4, 2) == 0) {
	if (strncmp(t_n, N_R4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		id_r[j] = (i == -1) ? N_MV_SI4 : (int) fd_n[i];
	    }
	} else if (strncmp(t_n, N_I4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		id_r[j] = (i == -1) ? N_MV_SI4 : id_n[i];
	    }
	} else if (strncmp(t_n, N_I2, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		id_r[j] = (i == -1) ? N_MV_SI4 : (int) sd_n[i];
	    }
	}
    } else if (strncmp(t_r, N_I2, 2) == 0) {
	if (strncmp(t_n, N_R4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		sd_r[j] = (i == -1) ? N_MV_SI2 : (short) fd_n[i];
	    }
	} else if (strncmp(t_n, N_I4, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		sd_r[j] = (i == -1) ? N_MV_SI2 : (short) id_n[i];
	    }
	} else if (strncmp(t_n, N_I2, 2) == 0) {
	    for (j = 0; j < n_st_r; j++) {
		if ((i = srf_amd_slct_sub(st_r[j], st_n, n_st_n)) == -1)
		    k++;
		sd_r[j] = (i == -1) ? N_MV_SI2 : sd_n[i];
	    }
	}
    }
    return (k);
}
