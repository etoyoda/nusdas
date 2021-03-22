/*----------------------------------------------------------------------*
    2003. 1.20   Create rbr.h rbr_libs.c 
    2003. 4. 3   Update rbr_ll_grid() ... add time check
    2003. 4. 4   Update rbr_header() ... error code -2 --> -2x
                 Update rbr_block() ... error code -99 --> -3x
 *----------------------------------------------------------------------* 
    :! /usr/bin/cc % -c 
 *----------------------------------------------------------------------*/

#include   <stdio.h>
#include   <time.h>    /* mktime()          */
#include   <string.h>  /* memcpy(),memset() */


#include   "rbr.h"


#define   min(a,b)   ((a)<(b)?(a):(b))
#define   max(a,b)   ((a)>(b)?(a):(b))


static FILE *err = NULL;

void rbr_warning (FILE *out) {
	err = out;
	if (out == NULL)
		err = stdin;
}

static void pr(char *s, int c)
{
    if (err == NULL)
	err = stderr;
    if (err == stdin)
	return;
    fprintf (err, s, c);
    fprintf (err, "\n");
}



/*--------------------------------------------------------------------* 
 *  rbr_time : time_t --> YYYY.MM.DD.HH.GG
 *--------------------------------------------------------------------*/
char *
rbr_time (time_t t, int offset)
{
    struct tm  *tmp;
    static char str[20];

    t += offset * 3600;

    tmp = gmtime(&t);
    sprintf (str, "%4d.%02d.%02d.%02d.%02d"
                , tmp->tm_year + 1900 , tmp->tm_mon + 1 , tmp->tm_mday
                , tmp->tm_hour , tmp->tm_min);

    return  str;
}


/*--------------------------------------------------------------------* 
 *  rbr_to_utc : YYYY.MM.DD.HH.GG --> time_t
 *--------------------------------------------------------------------*/
time_t
rbr_to_utc (char *rbr_time, int offset)
{
    int    a[5];
    int    n = sscanf(rbr_time,"%4d.%2d.%2d.%2d.%2d",a,a+1,a+2,a+3,a+4);

    if (n == 5) {
        struct tm  *tmp, tm;

        tm.tm_year = a[0] - 1900;
        tm.tm_mon  = a[1] - 1;
        tm.tm_mday = a[2];
        tm.tm_hour = a[3];
        tm.tm_min  = a[4];
        tm.tm_sec  = 0;

        return  mktime (&tm) - offset * 3600;

    } else {
        return  0;
    }
}


/*--------------------------------------------------------------------* 
 *  pandora_time : time_t --> YYYY-MM-DDtHHGG
 *--------------------------------------------------------------------*/
char *
pandora_time (time_t t)
{
    static char  str[20];
    struct tm   *tmp = gmtime(&t);
    sprintf (str, "%4d-%02d-%02dt%02d%02d"
                , tmp->tm_year + 1900 , tmp->tm_mon + 1 , tmp->tm_mday
                , tmp->tm_hour , tmp->tm_min);
    return  str;
}


/*--------------------------------------------------------------------* 
 *  pandora_to_time_t : YYYY-MM-DDtHHGG --> time_t
 *--------------------------------------------------------------------*/
time_t
pandora_to_time_t (char *pan_time)
{
    int     a[5];
    int     n = sscanf(pan_time,"%4d-%2d-%2dt%2d%2d",a,a+1,a+2,a+3,a+4);

    if (n == 5) {
        struct tm   tm;

        tm.tm_year  = a[0] - 1900;
        tm.tm_mon   = a[1] - 1;
        tm.tm_mday  = a[2];
        tm.tm_hour  = a[3];
        tm.tm_min   = a[4];
        tm.tm_sec   = 0;

        return  mktime (&tm);

    } else {
        return  0;

    }
}




/*-----------------------------------------------------------*/
int           /* ReturnCode 0 : 正常                         */
rbr_header    /*            1 : END CODEをチェックできない   */ 
              /*          <-20: 対象でない情報               */ 
              /*          <-50: レーダデータでない           */
    ( void   *ptr            /* in   Buffer array (ptr[len]) */
    , int     len            /* in          length of Buffer */
    , int    *type           /* out  pack(Type1 // Type2)    */
    , char    ymdhg[17]      /* out  "YYYY.MM.DD.HH.GG\0"    */
    , int    *bnum           /* out  Block number            */
    , int    *size           /* out  Total Data size         */
    , char  **bdp            /* out  First-Block pointer     */
    )
{
    unsigned char   *u = ptr;
    int     i, t1, t2, ww;
    short   ss;
    int     rt = 0;
 

    memset (ymdhg, '\0', 17);
    *type = *bnum = *size = -1;

    if (*u != 0xFD) return -99;    /*  START CODE  */
    if (len < 32)   return -98;    /*  HEADER SIZE */

    if (len >= 36) {
#ifdef  NO_SWAP
        memcpy (&ss, u+34, 2);
#else
        ss = *(u+34)*256 + *(u+35);
#endif
        if (ss < 0) return -88;
        *bnum = ss;
    }

    if (len >= 40) {
#ifdef  NO_SWAP
        memcpy (&ww, u+36, 4);
#else
        unsigned char *p = u + 36;
        if (127 < (ww = *p++)) return -89;
        ww = ww * 256 + *p++;
        ww = ww * 256 + *p++;
        ww = ww * 256 + *p  ;
#endif
        if (ww <= 0) return -89;
        *size = ww;
    }


    memcpy (ymdhg, u+8, 16); /* ymdhg[16] = '\0'; */


    switch (i = *(u+1)) {
      case 0x70: break;
      default: pr("W!  地整No.%02X : 対象ではない", i);
        rt = min(rt,-24);
    }


    t1 = *(u+2);
    t2 = *(u+3);
    *type = t1 * 0x100 + t2;



    switch (t1) {

      case 0xC0:

        switch (t2) {
          case 0x01:
          case 0x05:
            break;
          default: pr("W!  データ種別1,2:%04x 対象ではない", *type);
            rt = min(rt,-22);
        }
        break;

      default: pr("W!  データ種別1,2:%04x 対象ではない", *type);
        rt = min(rt,-23);
    }


    switch (i = *(u+6)) {
      case 0x00: *bdp = (char *)u + 32; break;
      case 0x01: *bdp = (char *)u + 64; break;
      default: pr("W!  ヘッダー種別:%02x 不明", i);
        rt = min(rt,-21);
    }

    if (len >= ww) {
        if (*(u+ww-1) != 0xFE) return -97;    /*  END CODE  */
    } else {
        rt = min(rt,1);
    }

    return  rt;
}




int             /* ReturnCode  0:正常   -3x:パラメータエラー */
rbr_block (
      void            *bp          /* in  block data pointer */
    , float           *lat         /* out under latitude     */
    , float           *lon         /* out left  longitude    */
    , int             *num         /* out cell number        */
    , unsigned char  **dp          /* out data pointer       */
    )
{
    unsigned char   *u = bp;
    int     mc = *(u + 2);
    int     m1 = mc / 0x10;
    int     m2 = mc % 0x10;
    float   flat = *(u + 0) / 1.5 + m1 / 12.;
    float   flon = *(u + 1) / 1.0 + m2 / 8. + 100;
    int     n = *(u + 3);
    int     rt = 0;


    if (flat>90  || flat<0)   return -38;
    if (flon>180 || flon<100) return -39;

    if (n > 160) {
        pr("W!  セル数が仕様の160を越えている %d\n", n);
        rt = 1;
    }

    *lat = flat;
    *lon = flon;
    *num = n;

    *dp  = (unsigned char  *)bp + 4;

    return  rt;
}



int
rbr_ll_grid (void *rbrp, int size, time_t sec, unsigned char *datap)
{
    double  lat0 = RBR_LAT0;
    double  lon0 = RBR_LON0;
    double  dlat = 1.0 / RBR_DLAT_INV;
    double  dlon = 1.0 / RBR_DLON_INV;

    unsigned char   *rbs = rbrp;
    unsigned char   *rbe = rbs + size -1;


    while (rbs < rbe) {
        char   *bdp, ymdhg[17];
        time_t  ts;
        int     rt, typ, bn, sz, flag, m;

        rt = rbr_header (rbs, size, &typ, ymdhg, &bn, &sz, &bdp);
        if (rt != 0) {
            if (sz > 0) {
                rbs += sz;
                continue;
            } else {
                break;
            }
        }

        ts = rbr_to_utc (ymdhg, 9);

        for (m=0; m<bn; m++) {
            float           lat1, lon1;
            int             num;
            unsigned char  *dp; 
    
            rt = rbr_block (bdp, &lat1, &lon1, &num, &dp);
            if (rt != 0) {
                bdp = rbs + sz;
                break;
            }
    
            if (ts == sec) {
                int     i, j, k;

                for (k=0; k<num; k++) {
                    double  lat = lat1 + dlat * 10;
                    double  lon = lon1 + k * dlon * 10;
        
                    for (j=0; j<10; j++) {
                        int             ilat = (lat0 - lat) * RBR_DLAT_INV +.1;
                        int             ilon = (lon - lon0) * RBR_DLON_INV +.1;
                        unsigned char  *d    = datap + ilat * RBR_IM + ilon;
        
                        for (i=0; i<10; i++) {
                            unsigned char  u = *dp++;
        
                            if      (u <= 0xFA) *d++ = u + 1;   /*  Value    */
                            else if (u == 0xFC) *d++ = 0;       /*  No Data  */
                            else                *d++ = 0;       /*  ???      */
                        }
        
                        lat -= dlat;
                    }
                }
            }
    
            bdp += 4 + num * 10 * 10;
        }

        rbs = (void *)bdp;
    }

    return  0;
}
