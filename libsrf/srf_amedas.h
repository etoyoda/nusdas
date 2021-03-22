/* -------------------------------------------------------------------------- */
/* アメダス地点辞書読込サブルーチン用                                         */
/* -------------------------------------------------------------------------- */
/* ヘッダーファイル                               Ver.1 2000.11.09 N.Fuji     */
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>

typedef struct amd_data {
    N_SI4 snum, type;		/* 地点番号、地点種別 */
    float lat, lon;		/* 緯度、経度         */
    float hh, wh;		/* 地点標高、風速計の高さ */
    char name1[10], name2[14];	/* 地点名カタカナ、漢字 */
} SRF_AMD_SINFO;		/* １地点データ構造体 */

#define SRF_KANS  0
#define SRF_ELM4  1
#define SRF_AMEL  2
#define SRF_AMEN  3
#define SRF_AIRP  4
#define SRF_YUKI  5
#define SRF_ALL  10

#ifdef __GNUC__
# define UNUSED __attribute__((unused))
#else
# define UNUSED
#endif
