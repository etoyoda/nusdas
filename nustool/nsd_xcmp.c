/*
:! cc -64 -O4 % -I/grpK/nwp/Open/Module/Comm/Lib/Nwp/Inc13 -L/grpK/nwp/Open/Module/Comm/Lib/Nwp -lnusdas13 -lnwp13 -lm
*/

#include   <stdio.h>
#include   <stdlib.h>
#include   <math.h>
#include   <string.h>
#include   <regex.h>
#include   "nwpl_capi.h"
#include   "nusdas.h"
#include   "nsd_param.h"
#include   "nsd_subc.h"

#define    max(a,b)    ((a)>(b)?(a):(b))
#define    min(a,b)    ((a)<(b)?(a):(b))


char   *myname = "?";
int     debug = 0;
int     verbs = 0;
int     force = 0;
int     subc  = 0;
int     ispec = 0;               /* SUBC ISPC (0:128, 1:512)              */
int     quiet = 0;               /* --quiet                               */
static int     noclose= 0;       /* --noclose                             */
static int     iaverage = 0;     /* --average                             */
static int     fdata = 1;        /* DATA check flag                       */
static int     fsubc = 0;        /* SUBC check flag                       */
static int     pmopt = 0;
static int     prhex = 0;
static int     large = 0;
static int     btdif = 0;        /* basetime diff                         */
static int     acc_s = -1;
static int     acc_r = -1;
static int     selbt1 = 0;       /* -b yyyymmddhh,                        */
static int     selbt2 = 0;       /* -b           ,yyyymmddhh              */
static int     selvt1 = 0;       /* -v yyyymmddhh,                        */
static int     selvt2 = 0;       /* -v           ,yyyymmddhh              */
static char   *seltpsrc[10];     /* -t t1t1t1t1t2t2t3t3                   */
static char   *seltpdst[10];     /*                    ,t1t1t1t1t2t2t3t3  */
static int     seltpnum = 0;     /*    type table number                  */
static int     regexp = 0;
static char   *selmm = NULL;     /* -m member                             */
static char   *selpl = NULL;
static char   *selel = NULL;
static int     long_long_size = sizeof(long long);

static char *
usage(int n, int argc, char *argv[])
{
    char   *p = strrchr(myname,'/');
    char   *msg = "\
 格納精度を考慮した NuSDaS データの比較ができる（NRDx は NUSDAS番号）。\n\
    NRD1 に存在するデータについて NRD2 のデータと比較します。←重要\n\
    NRD2 に信頼できる NuSDaSデータを指定して下さい。\n\
 1PAC,2UPC,2PAC,2UPJ,2UPP(PACKING),I2,I4(INTEGER no packing) の場合、\n\
    格納精度に許容範囲(-sN)を考慮して比較します。\n\
    許容範囲は、N までを OK とします。デフォルトは 0 です。（完全一致）\n\
 R4(REAL)の場合、\n\
    24bit精度を仮定し、許容範囲(-rN)を考慮して比較します。\n\
    この許容範囲は、整数の許容範囲を下回らないよう調整されます。\n\
    （整数の許容精度で -s2 と指定した場合、自動的に -r2 が仮定されます）\n\
\n\
 (option)\n\
    -sN   : レベル差 N までをOKとする。(-s2) for INTEGER, PACKING\n\
    -rN   : レベル差 N までをOKとする。(-r3) for REAL\n\
    -b... : basetime を限定する。 (200704051200[,200704061200])\n\
    -v... : validtime を限定する。(200704051200[,200704061200])\n\
    -t... : type1 type2 type3 を限定する。(_DCDSTSFOBSVAMD4)\n\
    -m... : member を限定する。\n\
    -p... : plane を限定する。\n\
    -e... : element を限定する。\n\
    -d    : 異なるデータを表示。\n\
    -d2   : 全てのデータを表示。\n\
    -x    : 16進数で表示。\n\
    -n    : データ個数を表示。\n\
    -D    : デバッグ情報表示。\n\
    -z    : 観測NuSDaS のように validtime が唯一で basetime が異なる比較。\n\
    その他: --quiet   : NRD1 に無いデータの表示 !NOTHING を抑止する。 \n\
            --regexp  : -m -p -e の比較に正規表現を使う。 \n\
            --no-data : DATA の比較をせず、NuSDaS INDEX FLAG のみ比較する。 \n\
            --no-subc : SUBC を比較しない。 \n\
            --ispcall : SUBC ISPC 512(byte)全てを比較する。 \n\
            --noclose : NUSDAS read write で file close しない\n\
            --average : 全格子平均値を表示する（比較判定はしない）\n\
            -t t1t1t1t1t2t2t3t3,txtx  type の異なるデータを比較する。 ";

    if (n < argc) return argv[n];
    printf("\n");
    printf("usage: %s [option] NRD1  NRD2\n", (p == NULL ? myname : p+1));
    printf("\n");
    printf(msg);
    printf("\n");
    exit(0);
}


static int cmp_bt(const void *key, const void *ary)
{
    int   *p1 = (int *)key;
    int   *p2 = (int *)ary;
    return  *p1 - *p2;
}

int
rcmp_i4(int size, void *dat1, void *dat2, int *flag, int *max_sa)
{
    int    *d1 = (int *)dat1;
    int    *d2 = (int *)dat2;
    int     i, mxsa = 0;
    int     acc = (acc_s < 0) ? 0 : acc_s;
    int     rt = 0;

    for (i=0; i<size; i++) {
        flag[i] = 0;
        if ((d1[i] != 0xFFFF) && (d2[i] != 0xFFFF)) {
            if (d1[i] != d2[i]) rt = 1;
            flag[i] = abs(d1[i] - d2[i]);
            mxsa = max(flag[i], mxsa);
        } else if (d1 != d2) {
            rt = 2;
            flag[i] = -1;
        }
    }
    *max_sa = mxsa;
    return  mxsa <= acc ? rt : 2;
}

int
rcmp_r8(int size, void *dat1, void *dat2, int *flag, int *max_sa)
{
    double  *f1 = (double *)dat1;
    double  *f2 = (double *)dat2;
    double       amp, scale;
    double       fmax, fmin;
    int          i, mxsa = 0;
    int          acc = (acc_r < 0) ? 0 : acc_r;
    int          rt = 0;

    fmax = DBL_MIN;
    fmin = DBL_MAX;
    for (i=0; i<size; i++) {
        if (f2[i] != DBL_MAX) {
            fmax = max(fmax, f2[i]);
            fmin = min(fmin, f2[i]);
        }
    }
    amp = (fmax - fmin) / ((1L << 53) - 1);
    if (amp == 0) amp = 1;
    scale = 1.0 / amp;
    for (i=0; i<size; i++) {
        double           d1 = f1[i];
        double           d2 = f2[i];
        long long        ii = fabs((d1 - d2) * scale) + 0.5;
        flag[i] = 0;
        if ((d1 != DBL_MAX) && (d2 != DBL_MAX)) {
            ii = (ii > 0x7FFFFFFF) ? 0x7FFFFFFF : ii;
            if (d1 != d2) rt = 1;
            mxsa = max(ii, mxsa);
            flag[i] = ii;
        } else if (d1 != d2) {
            rt = 2;
            flag[i] = -1;
        }
    }

    if (verbs > 1) {
        double  fmax1, fmin1;
        fmax1 = DBL_MIN;
        fmin1 = DBL_MAX;
        for (i=0; i<size; i++) {
            if (f1[i] != DBL_MAX) {
                fmax1 = max(fmax1, f1[i]);
                fmin1 = min(fmin1, f1[i]);
            }
        }
        printf("rcmp_r8: %d  min(%f,%f) max(%f,%f) max_sa=%d\n", size, fmin1, fmin, fmax1, fmax, mxsa);
    }
    *max_sa = mxsa;

    return  mxsa <= acc ? rt : 2;
}

int
rcmp_r4(int size, void *dat1, void *dat2, int *flag, int *max_sa)
{
    float  *f1 = (float *)dat1;
    float  *f2 = (float *)dat2;
    double       amp, scale;
    float        fmax, fmin;
    int          i, mxsa = 0;
    int          acc = (acc_r < 0) ? 0 : acc_r;
    int          rt = 0;

    fmax = FLT_MIN;
    fmin = FLT_MAX;
    for (i=0; i<size; i++) {
        if (f2[i] != FLT_MAX) {
            fmax = max(fmax, f2[i]);
            fmin = min(fmin, f2[i]);
        }
    }
    amp = (fmax - fmin) / ((1 << 24) - 1);
    if (amp == 0) amp = 1;
    scale = 1.0 / amp;
    for (i=0; i<size; i++) {
        double  d1 = f1[i];
        double  d2 = f2[i];
        flag[i] = 0;
        if ((d1 != FLT_MAX) && (d2 != FLT_MAX)) {
            int     ii = fabs((d1 - d2) * scale) + 0.5;
            if (d1 != d2) rt = 1;
            mxsa = max(ii, mxsa);
            flag[i] = ii;
        } else if (d1 != d2) {
            rt = 2;
            flag[i] = -1;
        }
    }

    if (verbs > 1) {
        float   fmax1, fmin1;
        fmax1 = FLT_MIN;
        fmin1 = FLT_MAX;
        for (i=0; i<size; i++) {
            if (f1[i] != FLT_MAX) {
                fmax1 = max(fmax1, f1[i]);
                fmin1 = min(fmin1, f1[i]);
            }    
        }
        printf("rcmp_r4: %d  min(%f,%f) max(%f,%f) max_sa=%d\n", size, fmin1, fmin, fmax1, fmax, mxsa);
    }
    *max_sa = mxsa;

    return  mxsa <= acc ? rt : 2;
}

int
rcmp_2p(int size, void *dat1, void *dat2, int *flag, int *max_sa)
{
    float  *f1 = (float *)dat1;
    float  *f2 = (float *)dat2;
    double  amp, scale;
    float   fmax, fmin;
    int     i, mxsa = 0;
    int     acc = (acc_s < 0) ? 0 : acc_s;
    int     rt = 0;

    fmax = FLT_MIN;
    fmin = FLT_MAX;
    for (i=0; i<size; i++) {
        if (f2[i] != FLT_MAX) {
            fmax = max(fmax, f2[i]);
            fmin = min(fmin, f2[i]);
        }
    }
    amp = (fmax - fmin) / ((1 << 16) - 1);
    if (amp == 0) amp = 1;
    scale = 1.0 / amp;
    for (i=0; i<size; i++) {
        double  d1 = f1[i];
        double  d2 = f2[i];
        flag[i] = 0;
        if ((d1 != FLT_MAX) && (d2 != FLT_MAX)) {
            int     ii = fabs((d1 - d2) * scale) + 0.5;
            if (d1 != d2) rt = 1;
            mxsa = max(ii, mxsa);
            flag[i] = ii;
        } else if (d1 != d2) {
            rt = 2;
            flag[i] = -1;
        }
    }

    if (verbs > 1) {
        float   fmax1, fmin1;
        fmax1 = FLT_MIN;
        fmin1 = FLT_MAX;
        for (i=0; i<size; i++) {
            if (f1[i] != FLT_MAX) {
                fmax1 = max(fmax1, f1[i]);
                fmin1 = min(fmin1, f1[i]);
            }
        }
        printf("rcmp_2p: %d  min(%g,%g) max(%g,%g) max_sa=%d\n", size, fmin1, fmin, fmax1, fmax, mxsa);
    }

    *max_sa = mxsa;

    return  mxsa <= acc ? rt : 2;
}


int
rcmp_1p(int size, void *dat1, void *dat2, int *flag, int *max_sa)
{
    float  *f1 = (float *)dat1;
    float  *f2 = (float *)dat2;
    double  amp, scale;
    float   fmax, fmin;
    int     i, mxsa = 0;
    int     acc = (acc_s < 0) ? 0 : acc_s;
    int     rt = 0;

    fmax = FLT_MIN;
    fmin = FLT_MAX;
    for (i=0; i<size; i++) {
        if (f2[i] != FLT_MAX) {
            fmax = max(fmax, f2[i]);
            fmin = min(fmin, f2[i]);
        }
    }
    amp = (fmax - fmin) / ((1 << 8) - 1);
    if (amp == 0) amp = 1;
    scale = 1.0 / amp;
    for (i=0; i<size; i++) {
        double  d1 = f1[i];
        double  d2 = f2[i];
        flag[i] = 0;
        if ((d1 != FLT_MAX) && (d2 != FLT_MAX)) {
            int     ii = fabs((d1 - d2) * scale) + 0.5;
            if (d1 != d2) rt = 1;
            mxsa = max(ii, mxsa);
            flag[i] = ii;
        } else if (d1 != d2) {
            rt = 2;
            flag[i] = -1;
        }
    }

    if (verbs > 1) {
        float   fmax1, fmin1;
        fmax1 = FLT_MIN;
        fmin1 = FLT_MAX;
        for (i=0; i<size; i++) {
            if (f1[i] != FLT_MAX) {
                fmax1 = max(fmax1, f1[i]);
                fmin1 = min(fmin1, f1[i]);
            }
        }
        printf("rcmp_1p: %d  min(%f,%f) max(%f,%f) max_sa=%d\n", size, fmin1, fmin, fmax1, fmax, mxsa);
    }

    *max_sa = mxsa;

    return  mxsa <= acc ? rt : 2;
}


int
rcmp(char *pack, int size, void *dat1, void *dat2, int *flag, int *max_sa)
{
    int     (*proc)() = NULL;

    if      (strncmp(pack,N_P_2PAC,4) == 0) proc = rcmp_2p;
    else if (strncmp(pack,N_P_2UPC,4) == 0) proc = rcmp_2p;
    else if (strncmp(pack,N_P_2UPJ,4) == 0) proc = rcmp_2p;
    else if (strncmp(pack,N_P_2UPP,4) == 0) proc = rcmp_2p;
    else if (strncmp(pack,N_P_1PAC,4) == 0) proc = rcmp_1p;
    else if (strncmp(pack,N_P_R4,4)   == 0) proc = rcmp_r4;
    else if (strncmp(pack,N_P_I1,4)   == 0) proc = rcmp_i4;
    else if (strncmp(pack,N_P_I2,4)   == 0) proc = rcmp_i4;
    else if (strncmp(pack,N_P_I4,4)   == 0) proc = rcmp_i4;
    else if (strncmp(pack,N_P_N1I2,4) == 0) proc = rcmp_i4;
    else if (strncmp(pack,N_P_RLEN,4) == 0) proc = rcmp_i4;
    else if (large == 1) {
         if (strncmp(pack,N_P_R8,4)   == 0) proc = rcmp_r8;
    }
    return  proc(size, dat1, dat2, flag, max_sa);
}


int
proc_read(char *type, int bt, char *mb, int vt, char *pl, char *el,
          void *data, int size, int *ipac)
{
    int     i, rt, xy[2], sz, dsz = 4;
    char   *pac = (char *)ipac;
    char   *dtyp = NULL;

    i = 1;
    rt = nusdas_inq_data(type,type+8,type+12,&bt,mb,&vt,pl,el,N_PC_PACKING, ipac,&i);
    if (rt != 1) {
        *ipac = 0;
        printf("!E  rt=%d,  nusdas_inq_data(%s,%d,%s,%d,%s,%s,N_PC_PACKING) \n", rt, type, bt, mb, vt, pl, el);
        return  -999;
    }

    if      (strncmp(pac,N_P_2PAC,4) == 0) { dtyp = N_R4; dsz = 4; }
    else if (strncmp(pac,N_P_2UPC,4) == 0) { dtyp = N_R4; dsz = 4; }
    else if (strncmp(pac,N_P_2UPJ,4) == 0) { dtyp = N_R4; dsz = 4; }
    else if (strncmp(pac,N_P_2UPP,4) == 0) { dtyp = N_R4; dsz = 4; }
    else if (strncmp(pac,N_P_1PAC,4) == 0) { dtyp = N_R4; dsz = 4; }
    else if (strncmp(pac,N_P_R4,4)   == 0) { dtyp = N_R4; dsz = 4; }
    else if (strncmp(pac,N_P_I1,4)   == 0) { dtyp = N_I4; dsz = 4; }
    else if (strncmp(pac,N_P_I2,4)   == 0) { dtyp = N_I4; dsz = 4; }
    else if (strncmp(pac,N_P_I4,4)   == 0) { dtyp = N_I4; dsz = 4; }
    else if (strncmp(pac,N_P_N1I2,4) == 0) { dtyp = N_I4; dsz = 4; }
    else if (strncmp(pac,N_P_RLEN,4) == 0) { dtyp = N_I4; dsz = 4; }
    else if (strncmp(pac,N_P_R8,4)   == 0) {
         if (large == 1) {                   dtyp = N_R8; dsz = 8;
         } else {
             printf("!!  PACKING(%s) try -X option, if you have enough memory.\n", pac);
             return  -999;
         }
    } else {
        printf("!E  Not support PACKING(%s)\n", pac);
        return  -999;
    }

    if (debug > 1) printf("dtyp,dsz=%s,%d \n", dtyp, dsz);
    i = 2;
    nusdas_inq_data (type,type+8,type+12,&bt,mb,&vt,pl,el,N_GRID_SIZE,&xy,&i);
    sz = xy[0] * xy[1];
    rt = nusdas_read(type,type+8,type+12,&bt,mb,&vt,pl,el,data,dtyp,&sz);
    if (rt <= 0) {
        printf("!E  rt=%d,  nusdas_read(%s,%d,%s,%d,%s,%s \n", rt, type, bt, mb, vt, pl, el);
        return  rt;
    }

    return  rt;
}



static int
diff_list(int *ipck, void *dat1, void *dat2, int *flag, int size[], int sz)
{
    int     i;
    char   *pac = (char *)ipck;
    int     acc = (acc_s < 0) ? 0 : acc_s;

    if (strncmp(pac,N_P_R4,4) == 0) acc = (acc_r < 0) ? 0 : acc_r;

    if (verbs > 1) printf("     data size = %d (%d * %d)\n", sz, size[0], size[1]);
    if ((strncmp(pac,N_P_I1  ,4) == 0) ||
        (strncmp(pac,N_P_I2  ,4) == 0) ||
        (strncmp(pac,N_P_I4  ,4) == 0) ||
        (strncmp(pac,N_P_N1I2,4) == 0) ||
        (strncmp(pac,N_P_RLEN,4) == 0) || prhex == 1) {
        int    *d1 = (int *)dat1;
        int    *d2 = (int *)dat2;
        int     dmax = *d1;
        int     dmin = *d1;
        char   *fmt = "%4s data[%d](%d,%d) %12d:%-12d  %10d\n";

        if (prhex == 1) fmt = "%4s data[%d](%d,%d)   x%08x:%08x  %10d\n";

        for (i=0; i<sz; i++) {
            int     ix = i % size[0] + 1;
            int     iy = i / size[0] + 1;
            if (dmax < d1[i]) dmax = d1[i];
            if (dmin > d1[i]) dmin = d1[i];
            if ((verbs ==1) && (flag[i] <= acc)) continue;
            printf(fmt, (flag[i] <= acc) ? "    " : "diff", i+1, ix, iy, d1[i], d2[i], flag[i]);
        }
        printf("          min < < max : %d < < %d \n", dmin, dmax);

    } else {
        float  *d1 = (float *)dat1;
        float  *d2 = (float *)dat2;
        float   fmax = FLT_MIN;
        float   fmin = FLT_MAX;
        char   *fmt = "%4s data[%d](%d,%d)   %g \t%g  %10d\n";

        for (i=0; i<sz; i++) {
            int     ix = i % size[0] + 1;
            int     iy = i / size[0] + 1;
            if (d1[i] != FLT_MAX) {
                if (fmax < d1[i]) fmax = d1[i];
                if (fmin > d1[i]) fmin = d1[i];
            }
            if ((verbs ==1) && (flag[i] <= acc)) continue;
            printf(fmt, (flag[i] <= acc) ? "    " : "diff", i+1, ix, iy, d1[i], d2[i], flag[i]);
        }
        printf("          min < < max : %g < < %g \n", fmin, fmax);
    }
    return  0;
}


static int
average(char *pack, int size, void *dat1, void *dat2)
{
    int     i;
    int     nave = 0;
    if        ((strncmp(pack,N_P_I1,4)   == 0) ||
               (strncmp(pack,N_P_I2,4)   == 0) ||
               (strncmp(pack,N_P_I4,4)   == 0) ||
               (strncmp(pack,N_P_N1I2,4) == 0) ||
               (strncmp(pack,N_P_RLEN,4) == 0)) {
               int    *d1 = (int *)dat1;
               int    *d2 = (int *)dat2;  
               float     ave1 = 0;
               float     ave2 = 0;
               char *fmt = " average(%d,%d) diff %d / %6.3f%c\n";

               for (i=0; i<size; i++) {
                   if (d2[i] != 0xFFFF) {
                       nave++;
                       ave1 += d1[i];
                       ave2 += d2[i];
                   }
               }
               ave1 /= nave;
               ave2 /= nave;

               printf(fmt, ave1, ave2, ave1-ave2, fabs(100*(ave1-ave2)/ave2), '%');

    } else if ((strncmp(pack,N_P_2PAC,4) == 0) ||
               (strncmp(pack,N_P_2UPC,4) == 0) ||
               (strncmp(pack,N_P_2UPJ,4) == 0) ||
               (strncmp(pack,N_P_2UPP,4) == 0) ||
               (strncmp(pack,N_P_1PAC,4) == 0) ||
               (strncmp(pack,N_P_R4,4)   == 0)) {
               float  *d1 = (float *)dat1;
               float  *d2 = (float *)dat2;
               float     ave1 = 0;
               float     ave2 = 0;
               char *fmt = " average(%g,%g) diff %g / %6.3f%c\n";

               for (i=0; i<size; i++) {
                   if (d2[i] != FLT_MAX) {
                       nave++;
                       ave1 += d1[i];
                       ave2 += d2[i];
                   }
               }
               ave1 /= nave;
               ave2 /= nave;

               printf(fmt, ave1, ave2, ave1-ave2, fabs(100*(ave1-ave2)/ave2), '%');

    } else if  (large == 1) {
               if (strncmp(pack,N_P_R8,4)   == 0) {
               double  *d1 = (double *)dat1;
               double  *d2 = (double *)dat2;
               double   ave1 = 0;
               double   ave2 = 0;
               char *fmt = " average(%g,%g) diff %g / %6.3f%c\n";

               for (i=0; i<size; i++) {
                   if (d2[i] != DBL_MAX) {
                       nave++;
                       ave1 += d1[i];
                       ave2 += d2[i];
                   }
               }
               ave1 /= nave;
               ave2 /= nave;

               printf(fmt, ave1, ave2, ave1-ave2, fabs(100*(ave1-ave2)/ave2), '%');

               }
    }
    return 0;
}


int
proc_data(struct nsd_parm csd1, struct nsd_parm csd2, int *recp)
{
    int     c_m1, c_k1;
    int     b1, m1, k1, l1, e1;
    int     b2, m2, k2, l2, e2;
    int     i, rtn, rt1, rt2;
    char   *t1, *t2, *t3;
    int     sz = csd1.size[0] * csd1.size[1];
    int     word = (large == 1 ? 8 : 4);
    int     pck1[2], pck2[2], ng_count = 0;

    int    *flag = malloc(sz * sizeof(int));
    void   *dat1 = malloc(sz * word);
    void   *dat2 = malloc(sz * word);

    int    *tb1vt = malloc(csd1.nf * sizeof(int));
    int    *tb2vt = malloc(csd2.nf * sizeof(int));
    char   *tb1mb = calloc(csd1.nm * 4 + 1, 1);
    char   *tb2mb = calloc(csd2.nm * 4 + 1, 1);
    char   *tb1pl = calloc(csd1.np * 6 + 1, 1);
    char   *tb2pl = calloc(csd2.np * 6 + 1, 1);
    char   *tb1el = calloc(csd1.ne * 6 + 1, 1);
    char   *tb2el = calloc(csd2.ne * 6 + 1, 1);

    if (debug) printf("proc_data malloc(%d * %d)\n", sz, word);
    if (dat1 == NULL) { printf("!E  dat1 = malloc(%d)\n", sz * word); exit(1); }
    if (dat2 == NULL) { printf("!E  dat2 = malloc(%d)\n", sz * word); exit(1); }
    if (flag == NULL) { printf("!E  flag = malloc(%d)\n", sz * sizeof(int)); exit(1); }

    if ((i = (tb1vt == NULL ? 0x01 : 0) | (tb2vt == NULL ? 0x10 : 0) |
             (tb1mb == NULL ? 0x02 : 0) | (tb2mb == NULL ? 0x20 : 0) |
             (tb1pl == NULL ? 0x04 : 0) | (tb2pl == NULL ? 0x40 : 0) |
             (tb1el == NULL ? 0x08 : 0) | (tb2el == NULL ? 0x80 : 0)   ) != 0) {
        printf("!E  tb[12](vt|mb|pl|el) malloc (%x)\n", i);
        exit(1);
    }

    qsort(csd1.bt, csd1.nb, sizeof(N_SI4), cmp_bt);
    qsort(csd2.bt, csd2.nb, sizeof(N_SI4), cmp_bt);
    if (debug) {
        for (i=0; i<csd1.nb-1; i+=max(1,csd1.nb/10)) {
            printf("    csd1.bt[%d] = %d \n", i, csd1.bt[i]);
        }   printf("    csd1.bt[%d] = %d \n", csd1.nb-1, csd1.bt[csd1.nb-1]);
        for (i=0; i<csd2.nb-1; i+=max(1,csd2.nb/10)) {
            printf("    csd2.bt[%d] = %d \n", i, csd2.bt[i]);
        }   printf("    csd2.bt[%d] = %d \n", csd2.nb-1, csd2.bt[csd2.nb-1]);
    }

#define   ERR_STOP(f,s,i,n)   if(f){printf("Stop! %s:%d:%d\n", s, i, n);exit(99);}

    for (b1=0; b1<csd1.nb; b1++) {
        int     py, pm, pd, ph, pg;
        char    abt1[20];
        int     bt1 = csd1.bt[b1];                                              /* nsd1: basetime */

        if (selbt1 != 0) { /* select basetime */
            if (selbt2 != 0) {
                if ((selbt1 > bt1) || (selbt2 < bt1)) continue;
            } else {
                if (selbt1 != bt1) continue;
            }
        }

        nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, bt1);
        sprintf(abt1, "%04d%02d%02d%02d%02d", py, pm, pd, ph, pg);
        if (verbs|debug) printf("  basetime(%s)\n", abt1);

        /* member は、nusdas_def で定義されたものを全て検索する */
        for (c_m1=0; c_m1<csd1.nm; c_m1++) {
            char    mmb[5];                                                     /* nsd1: member */

            strncpy(mmb, csd1.mb + c_m1 * 4, 4); mmb[4] = '\0';

            /* if ((selmm[0] != '\0') && (strncmp(mmb,selmm,strlen(selmm)) != 0)) continue; * select */
            if (selmm != NULL) {
                char    str[5];
                if (regexp == 1) {
                    regex_t         preg ;
                    regmatch_t      pmatch[1] ;
                    regcomp(&preg, selmm, REG_EXTENDED|REG_NEWLINE) ;
                    sscanf(mmb,"%s",str);
                    i = regexec(&preg, str, 1, pmatch, 0) ;
                    regfree(&preg) ;
                    if (i == REG_NOMATCH) continue;
                } else {
                    strcpy (str,"    ");
                    strncpy(str,selmm,min(4,strlen(selmm)));
                    if (strncmp(mmb,str,4) != 0) continue; /* select member */
                }
            }
                                                                                /* check nsd2:member */
            if (verbs|debug) printf("  member(%s)\n", mmb);
            for (m2=0; m2<csd2.nm; m2++) if (strncmp(csd2.mb + m2*4, mmb, 4) == 0) break;
            if (m2 >= csd2.nm) {
                printf("!W  NUSDAS%02d nusdas_def nothing member(%s) \n", csd2.nsd, mmb);
                continue;
            }

            /* validtime も、nusdas_def で定義されたものを全て検索する */
            for (c_k1=0; c_k1<csd1.nf; c_k1++) {
                int     vt = time_add(bt1, csd1.ft[c_k1], csd1.fu);             /* nsd1: validtime */
                int     ft1 = vt - bt1;
                char    avt[20], abt2[20];
                int     n1mb, n1vt, n1pl, n1el;
                int     n2mb, n2vt, n2pl, n2el, bt2, ft2;

                if (selvt1 != 0) {   /* select validtime */
                    if (selvt2 != 0) {
                        if ((selvt1 > vt) || (selvt2 < vt)) continue;
                    } else {
                        if (selvt1 != vt) continue;
                    }
                }

                nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, vt);
                sprintf(avt, "%04d%02d%02d%02d%02d", py, pm, pd, ph, pg);
                if (verbs|debug) printf("  validtime(%s) \n", avt);

                t1 = csd1.types;
                t2 = csd1.types + 8;
                t3 = csd1.types + 12;
                nusdas_parameter_change(N_PC_ID_SET, &csd1.nsd);

                /* csd1 に mmb vt のデータが存在しているかのチェック */
                i = csd1.nw;                                                    /* nsd1: element data map */
                csd1.nr = nusdas_inq_cntl(t1,t2,t3,&bt1,mmb,&vt,N_DATA_MAP,csd1.er,&i);
                if (debug) printf("    rt1=%d  NUSDAS%02d nusdas_inq_cntl(N_DATA_MAP) %s:%s:%s \n", csd1.nr, csd1.nsd, abt1, mmb, avt);
                if (csd1.nr <= 0) {
                    if (csd1.nr != -51) {
                        printf("!E  rt1=%d  NUSDAS%02d nusdas_inq_cntl(N_DATA_MAP) %s:%s:%s \n", csd1.nr, csd1.nsd, abt1, mmb, avt);
                    }
                    continue;
                }

                i = 1;
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt1, mmb, &vt, N_MEMBER_NUM, &n1mb, &i);
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt1, mmb, &vt, N_VALIDTIME_NUM, &n1vt, &i);
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt1, mmb, &vt, N_PLANE_NUM, &n1pl, &i);
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt1, mmb, &vt, N_ELEMENT_NUM, &n1el, &i);

                rtn = nusdas_inq_cntl(t1, t2, t3, &bt1, mmb, &vt, N_MEMBER_LIST, tb1mb, &n1mb);
                if (debug) { ERR_STOP(rtn != n1mb, "N_MEMBER_LIST", rtn, n1mb) }
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt1, mmb, &vt, N_VALIDTIME_LIST, tb1vt, &n1vt);
                if (debug) { ERR_STOP(rtn != n1vt,"N_VALIDTIME_LIST",rtn,n1vt) }
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt1, mmb, &vt, N_PLANE_LIST, tb1pl, &n1pl);
                if (debug) { ERR_STOP(rtn != n1pl,"N_PLANE_LIST",rtn,n1pl) }
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt1, mmb, &vt, N_ELEMENT_LIST, tb1el, &n1el);
                if (debug) { ERR_STOP(rtn != n1el,"N_ELEMENT_LIST",rtn,n1el) }

                for (m1=0; m1<n1mb; m1++) if (strncmp(tb1mb + m1*4, mmb, 4) == 0) break;
                if (m1 >= n1mb) continue;
                for (k1=0; k1<n1vt; k1++) if (tb1vt[k1] == vt) break;
                if (k1 >= n1vt) continue;

                if (debug == 2) {
                    print_map(m1, k1, csd1.nsd, n1mb, n1vt, n1pl, n1el, csd1.nr, tb1mb, tb1vt, tb1pl, tb1el, csd1.er);
                } else if (debug) {
                    printf("NUSDAS%02d:n1mb,n1vt,n1pl,n1el=%d,%d,%d,%d, map_size=%d \n", csd1.nsd, n1mb, n1vt, n1pl, n1el, csd1.nr);
                    printf("N_MEMBER  LIST(%s)\n",tb1mb);
                    printf("N_PLANE   LIST(%s)\n",tb1pl);
                    printf("N_ELEMENT LIST(%s)\n",tb1el);
                    for (i=0; i<n1vt-1; i+=max(1,n1vt/10)) {
                        printf("%4d: %d\n", i, tb1vt[i]);
                    }   printf("%4d: %d\n", n1vt-1, tb1vt[n1vt-1]);
                    printf("(%s) m1 = %d / %d - (%s)\n", m1 < n1mb ? "OK" : "NG", m1, n1mb, mmb);        /* csd1 に mmb の有無 */
                    printf("(%s) k1 = %d / %d ... %d\n", k1 < n1vt ? "OK" : "NG", k1, n1vt, tb1vt[k1]);  /* csd1 に vt  の有無 */
                    printf("\n");
                }

                /* csd2 に vt が存在するかのチェック */
                t1 = csd2.types;
                t2 = csd2.types + 8;
                t3 = csd2.types + 12;
                nusdas_parameter_change(N_PC_ID_SET, &csd2.nsd);

                if (btdif == 0) {
                    int    *p;
                    bt2 = bt1;
                    ft2 = ft1;
                    p = bsearch(&bt2, csd2.bt, csd2.nb, sizeof(N_SI4), cmp_bt);
                    if (p == NULL) {
                        if (debug) printf("!W  NUSDAS%02d  nothing  basetime  %s\n", csd2.nsd, abt1);
                        continue;
                    }
                    i = 1;
                    rtn = nusdas_inq_cntl(t1, t2, t3, &bt2, mmb, &vt, N_MEMBER_NUM, &n2mb, &i);
                    if ((rtn != 1) ||
                        (nusdas_inq_cntl(t1, t2, t3, &bt2, mmb, &vt, N_VALIDTIME_NUM, &n2vt, &i) != 1)) {
                        if (debug) printf("!NOTHING  NUSDAS%02d  %s   member,vt  %s:%d\n", csd1.nsd, avt, mmb, vt);
                        continue;
                    }

                } else {

                    bt2 = 0;
                    for (k2=0; k2<csd2.nf; k2++) {                              /* nsd2: search basetime */
                        int    x = time_add(vt, -csd2.ft[k2], csd2.fu);
                        if (bsearch(&x, csd2.bt, csd2.nb, sizeof(N_SI4), cmp_bt) != NULL) {
                            i = 1;
                            rtn = nusdas_inq_cntl(t1, t2, t3, &x, mmb, &vt, N_MEMBER_NUM, &n2mb, &i);
                            if ((rtn == 1) &&
                                (nusdas_inq_cntl(t1, t2, t3, &x, mmb, &vt, N_VALIDTIME_NUM, &n2vt, &i) == 1) ) {
                                bt2 = x;
                                ft2 = vt - bt2;
                                break;
                            }
                        }
                    }
                    if (bt2 <= 0) {
                        if (debug) printf("!W  NUSDAS%02d  %s  nothing  member,vt  %s:%d\n", csd2.nsd, avt, mmb, vt);
                        continue;
                    }
                }
                nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, bt2);
                sprintf(abt2, "%04d%02d%02d%02d%02d", py, pm, pd, ph, pg);

                i = csd2.nw;                                                    /* nsd2: element data map */
                csd2.nr = nusdas_inq_cntl(t1,t2,t3,&bt2,mmb,&vt,N_DATA_MAP,csd2.er,&i);
                if (csd2.nr <= 0) {
                    printf("!E  rt2=%d  NUSDAS%02d  %s:%s  nusdas_inq_cntl(N_DATA_MAP)\n", csd2.nr, csd2.nsd, abt2, avt);
                    continue;
                }

                i = 1;
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt2, mmb, &vt, N_PLANE_NUM, &n2pl, &i);
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt2, mmb, &vt, N_ELEMENT_NUM, &n2el, &i);

                rtn = nusdas_inq_cntl(t1, t2, t3, &bt2, mmb, &vt, N_MEMBER_LIST, tb2mb, &n2mb);
                if (debug) { ERR_STOP(rtn != n2mb,"N_MEMBER_LIST",rtn,n2mb) }
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt2, mmb, &vt, N_VALIDTIME_LIST, tb2vt, &n2vt);
                if (debug) { ERR_STOP(rtn != n2vt, "N_VALIDTIME_LIST",rtn,n2vt) }
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt2, mmb, &vt, N_PLANE_LIST, tb2pl, &n2pl);
                if (debug) { ERR_STOP(rtn != n2pl, "N_PLANE_LIST",rtn,n2pl) }
                rtn = nusdas_inq_cntl(t1, t2, t3, &bt2, mmb, &vt, N_ELEMENT_LIST, tb2el, &n2el);
                if (debug) { ERR_STOP(rtn != n2el, "N_ELEMENT_LIST",rtn,n2el) }

                for (m2=0; m2<n2mb; m2++) if (strncmp(tb2mb + m2*4, mmb, 4) == 0) break;
                for (k2=0; k2<n2vt; k2++) if (tb2vt[k2] == vt) break;

                if (debug == 2) {
                    print_map(m2, k2, csd2.nsd, n2mb, n2vt, n2pl, n2el, csd2.nr, tb2mb, tb2vt, tb2pl, tb2el, csd2.er);
                } else if (debug > 0) {
                    printf("NUSDAS%02d:n2mb,n2vt,n2pl,n2el=%d,%d,%d,%d, map_size=%d \n", csd2.nsd, n2mb, n2vt, n2pl, n2el, csd2.nr);
                    printf("N_MEMBER  LIST(%s)\n",tb2mb);
                    printf("N_PLANE   LIST(%s)\n",tb2pl);
                    printf("N_ELEMENT LIST(%s)\n",tb2el);
                    for (i=0; i<n2vt-1; i+=max(1,n2vt/10)) {
                        printf("%4d: %d\n", i, tb2vt[i]);
                    }   printf("%4d: %d\n", n2vt-1, tb2vt[n2vt-1]);
                    printf("(%s) m2 = %d / %d - (%s)\n", m2 < n2mb ? "OK" : "NG", m2, n2mb, mmb);        /* csd2 に mmb の有無 */
                    printf("(%s) k2 = %d / %d ... %d\n", k2 < n2vt ? "OK" : "NG", k2, n2vt, tb2vt[k2]);  /* csd2 に vt  の有無 */
                    printf("\n");
                }

                for (l1=0; l1<n1pl; l1++) {
                    char    pln[7];

                    strncpy(pln, tb1pl + l1 * 6, 6); pln[6] = '\0';

                    /* if ((selpl[0] != '\0') && (strncmp(pln,selpl,strlen(pln)) != 0)) continue; * select plane */
                    if (selpl != NULL) {
                        char    str[7];
                        if (regexp == 1) {
                            regex_t         preg ;
                            regmatch_t      pmatch[1] ;
                            regcomp(&preg, selpl, REG_EXTENDED|REG_NEWLINE) ;
                            sscanf(pln,"%s",str);
                            i = regexec(&preg, str, 1, pmatch, 0) ;
                            regfree(&preg) ;
                            if (i == REG_NOMATCH) continue;
                        } else {
                            strcpy (str,"      ");
                            strncpy(str,selpl,min(6,strlen(selpl)));
                            if (strncmp(pln,str,6) != 0) continue; /* select plane */
                        }
                    }
                                                                                 /* check nsd2:plane */
                    for (l2=0; l2<n2pl; l2++) if (strncmp(tb2pl + l2*6, pln, 6) == 0) break;
                    if (l2 >= n2pl) {
                        printf("!W  NUSDAS%02d  index nothing  %s FT=%d plane(%s) \n", csd2.nsd, abt2, ft2, pln);
                        continue;
                    }

                    for (e1=0; e1<n1el; e1++) {
                        char    elm[7];
                        int     ng_flag = 0;
                        int     i1, i2, er2;

                        strncpy(elm, tb1el + e1 * 6, 6); elm[6] = '\0';

                        /* if ((selel[0] != '\0') && (strncmp(elm,selel,strlen(elm)) != 0)) continue; * select element */
                        if (selel != NULL) {
                            char    str[7];
                            if (regexp == 1) {
                                regex_t         preg ;
                                regmatch_t      pmatch[1] ;
                                regcomp(&preg, selel, REG_EXTENDED|REG_NEWLINE) ;
                                sscanf(elm,"%s",str);
                                i = regexec(&preg, str, 1, pmatch, 0) ;
                                regfree(&preg) ;
                                if (i == REG_NOMATCH) continue;
                            } else {
                                strcpy (str,"      ");
                                strncpy(str,selel,min(6,strlen(selel)));
                                if (strncmp(elm,str,6) != 0) continue; /* select element */
                            }
                        }

                        if (debug > 1) printf("NUSDAS%02d:%d:%s:%d:%s:%s.\n",csd1.nsd,bt1,mmb,ft1,pln,elm);

                        i1 = e1 + n1el * (l1  + n1pl * (k1  + n1vt * m1));

                        if (debug > 1) {
                            printf("  NUSDAS%02d(map[%d]=%d)%d:%4s:%d:%s:%s \n", csd1.nsd, i1, csd1.er[i1], bt1, mmb, vt, pln, elm);
                            printf("              [%d] ... m1,k1,l1,e1 = %d, %d, %d, %d\n", i1, m1, k1, l1, e1);
                        }

                                                                                /* check nsd2:element */
                        for (e2=0; e2<n2el; e2++) if (strncmp(tb2el + e2*6, elm, 6) == 0) break;
                        if (e2 >= n2el) {
                            printf("!W  NUSDAS%02d  index nothing  %s FT=%d element(%s) \n", csd2.nsd, abt2, ft2, elm);
                            continue;
                        }

                        if (debug > 1) printf("NUSDAS%02d:%d:%s:%d:%s:%s.\n",csd2.nsd,bt2,mmb,ft2,pln,elm);

                        if        (m2 >= n2mb) {
                            i2 = 0;
                            er2 = -1;
                        } else if (k2 >= n2vt) {  /* この条件は満たされない */
                            i2 = 0;
                            er2 = -999;
                        } else {
                            i2 = e2 + n2el * (l2 + n2pl * (k2 + n2vt * m2));
                            er2 = csd2.er[i2];
                        }

                        if (debug > 1) {
                            printf("  NUSDAS%02d(map[%d]=%d)%d:%4s:%d:%s:%s \n", csd2.nsd, i2, er2, bt2, mmb, vt, pln, elm);
                            printf("              [%d] ... m2,k2,l2,e2 = %d, %d, %d, %d\n", i2, m2, k2, l2, e2);
                        }

                        if (csd1.er[i1] == 0) {     /* nsd1: data nothing? */
                            if (!fdata) *recp += 1;
                            if (er2 <= 0) {         /* nsd2: data nothing? */
                                continue;
                            } else {                /* nsd1 nothing : nsd2 exist */
                                if (!fdata) ++ng_count;
                                printf("!NOTHING(NUSDAS%02d)  %s FT=%d (%4s:%s:%s) \n", csd1.nsd, abt1, ft1, mmb, pln, elm);
                            }
                            continue;
                        } else {
                            *recp += 1;
                            if (er2 <= 0) {         /* nsd1 exist : nsd2 nothing */
                                if (!fdata) ++ng_count;
                                printf("!W  NUSDAS%02d  index nothing  %s FT=%d (%4s:%s:%s) \n", csd2.nsd, abt2, ft2, mmb, pln, elm);
                                continue;
                            } /* else                  nsd1 exist : nsd2 exist   */
                        }

if (fdata) {
    pck1[1] = pck2[1] = 0;
                                                                                /* nsd1: read data */
    t1 = csd1.types;
    t2 = csd1.types + 8;
    t3 = csd1.types + 12;
    rtn = nusdas_parameter_change(N_PC_ID_SET, &csd1.nsd);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET,%d)\n", rtn, csd1.nsd);

    if (debug > 1) {
        char    miss_mode[5];
        i = 1;
        strcpy(miss_mode,"????");
        rtn = nusdas_inq_data(t1, t2, t3, &bt1, mmb, &vt, pln, elm, N_MISSING_MODE, miss_mode, &i);
        miss_mode[4] = '\0';
        printf("    NUSDAS%02d: nusdas_inq_data(N_MISSING_MODE) rt=%d [%s] \n", csd1.nsd, rtn, miss_mode);
    }

    rt1 = proc_read(csd1.types,bt1,mmb,vt,pln,elm,dat1,sz,pck1);
    pck1[1] = 0;
    if (rt1 <= 0) {
        printf("!E  NUSDAS%02d NOTHING data rt1=%d\n", csd1.nsd, rt1);
        continue;
    }
    if (debug) printf(" <NUSDAS%02d:%s:%d:%4s:%d:%s:%s(%s:%d)\n", csd1.nsd, csd1.types, bt1, mmb, vt, pln, elm, (char *)pck1, rt1);
    if (debug > 1) printf("((base,valid=%s,%s))\n",abt1,avt);

                                                                                /* nsd2: read data */
    t1 = csd2.types;
    t2 = csd2.types + 8;
    t3 = csd2.types + 12;
    rt2 = nusdas_parameter_change(N_PC_ID_SET, &csd2.nsd);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET,%d)\n", rt2, csd2.nsd);

    rt2 = proc_read(csd2.types,bt2,mmb,vt,pln,elm,dat2,sz,pck2);
    pck2[1] = 0;
    if (debug) printf(" >NUSDAS%02d:%s:%d:%4s:%d:%s:%s(%s:%d)\n", csd2.nsd, csd2.types, bt2, mmb, vt, pln, elm, (char *)pck2, rt2);
    if (debug > 1) printf("((base,valid=%s,%s))\n",abt2,avt);

    if (rt2 <= 0) {
        printf("!W  NUSDAS%02d data nothing (rt2=%d) %s:%s:%s\n", csd2.nsd, rt2, avt, pln, elm);
    } else if (rt1 != rt2) {
        ng_flag = 1;
        printf("DIFF DATA size NUSDAS%02d(%d) NUSDAS%02d(%d) %s:%s:%s:%s\n", csd1.nsd, rt1, csd2.nsd, rt2, mmb, avt, pln, elm);
    } else {
        char   *ans, *fmt = "%s%s %s:%s:%4s:%s:%s:%s %s:%-8d %d\n";
        char    str[100], abt[100];
        int     max_sa;
        int     rt = rcmp((char *)pck1, rt1, dat1, dat2, flag, &max_sa);
        if        (rt == 0) { ans = "  EQ";
        } else if (rt == 1) { ans = "  OK";
        } else if (rt == 2) { ans = "DIFF"; ng_flag = 1;
        } else { printf("!E  rcmp()  rt=%d \n", rt); exit(1);
        }
        if (prhex == 1) fmt = "%s%s %s:%s:%4s:%s:%s:%s %s:%-8d x%x\n";
        if (pmopt & 1) sprintf(str, ".%d", *recp);
        else           sprintf(str, "");
        if (bt1 == bt2)                     sprintf(abt, "%s", abt1);
        else if (strncmp(abt1,abt2,4) == 0) sprintf(abt, "%s.%s", abt1, abt2+4);
        else                                sprintf(abt, "%s.%s", abt1, abt2);
        printf(fmt, ans, str, csd1.types+12, abt, mmb, avt, pln, elm, (char *)pck1, rt1, max_sa);

        if (verbs) diff_list(pck1, dat1, dat2, flag, csd1.size, rt1);

        if (iaverage) average((char *)pck1, rt1, dat1, dat2);

        if (fsubc >= 0) {
            struct nsd_subc_s  subc1, subc2;
            subc1.nsd   = csd1.nsd;
            subc1.bt    = bt1;
            subc1.types = csd1.types;
            subc2.nsd   = csd2.nsd;
            subc2.bt    = bt2;
            subc2.types = csd2.types;
            if (proc_subc(&subc2, &subc1, mmb, vt, pln, elm, "GET")) ng_flag = 1;
        }
    }
    if (ng_flag != 0) ++ng_count;
}

                    } /* element */
                } /* plane */

                if (quiet == 0) {  /* csd2 に有り、csd1 に無い要素のリストアップ */
                    for (l2=0; l2<n2pl; l2++) {
                        char    pln[7];

                        strncpy(pln, tb2pl + l2*6, 6); pln[6] = '\0';
                        for (l1=0; l1<n1pl; l1++) if (strncmp(tb1pl + l1*6, pln, 6) == 0) break;
                        if (l1 >= n1pl) {
                            printf("!NOTHING  NUSDAS%02d  %s FT=%d plane(%s) \n", csd1.nsd, abt1, ft1, pln);
                            continue;
                        }

                        for (e2=0; e2<n2el; e2++) {
                            char    elm[7];
                            int     i1, i2;

                            strncpy(elm, tb2el + e2 * 6, 6); elm[6] = '\0';
                            for (e1=0; e1<n1el; e1++) if (strncmp(tb1el + e1*6, elm, 6) == 0) break;
                            if (e1 >= n1el) {
                                printf("!NOTHING  NUSDAS%02d  %s FT=%d element(%s) \n", csd1.nsd, abt1, ft1, elm);
                                continue;
                            }


                            i1 = e1 + n1el * (l1 + n1pl * (k1 + n1vt * m1));
                            i2 = e2 + n2el * (l2 + n2pl * (k2 + n2vt * m2));
                            if ((csd1.er[i1] == 0) && (csd2.er[i2] == 1)) {
                                printf("!NOTHING(NUSDAS%02d) %s:%s:%4s:%s:%s:%s \n", csd1.nsd, csd1.types+12, abt1, mmb, avt, pln, elm);
                                continue;
                            }
                        }
                    }
                } /* quiet == 0 */
            } /* validtime */
        } /* member */
    } /* basetime */

#define   FREE(p)   if ((p) != NULL) free(p)
    FREE(tb1vt);
    FREE(tb2vt);
    FREE(tb1mb);
    FREE(tb2mb);
    FREE(tb1pl);
    FREE(tb2pl);
    FREE(tb1el);
    FREE(tb2el);

    FREE(dat1);
    FREE(dat2);
    FREE(flag);
#undef    FREE

    return  ng_count;
}


int
proc_type(int nsd1, int nsd2, char *types1, char *types2, int *recp)
{
    struct nsd_parm   csd1, csd2;
    int     i, rt;

    *recp = 0;

    rt = nusdas_parameter_change(N_PC_ID_SET, &nsd1);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET,NUSDAS%02d) \n", rt, nsd1);
    rt = nsd_param(types1, &csd1, selbt1, selbt2);
    if (rt != 0) {
        printf("!NOTHING  NUSDAS%02d(%s) !!!\n", nsd1, types1);
        return rt;
    }
    csd1.nsd = nsd1;

    if (force == 0 && (selbt1 == 0 && csd1.nb > 300)) {
        for (i=0; i<csd1.nb; i++) {
            int     y, m, d, h, g;
            char    abt[100];
            nwp_seq2ymdhm(&y, &m, &d, &h, &g, csd1.bt[i]);
            sprintf(abt, "%04d%02d%02d%02d%02d", y, m, d, h, g);
            if ((i%5) == 0) printf("\n");
            printf("  %s", abt);
        }
        printf("\n");
        printf("!E  NUSDAS%02d(%s) basetime too many : %d !!!\n", nsd1, types1, csd1.nb);
        printf("    please, option use (-b YYYYMMDDHHGG) or (--force)\n");
        return  -1;
    }

    rt = nusdas_parameter_change(N_PC_ID_SET, &nsd2);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET,NUSDAS%02d) \n", rt, nsd2);
    rt = nsd_param(types2, &csd2, selbt1, selbt2);
    if (rt != 0) {
        printf("!E  NUSDAS%02d(%s) nothing. !!!\n", nsd2, types2);
        return rt;
    }
    csd2.nsd = nsd2;

    rt = proc_data(csd1, csd2, recp);

    return  rt;
}


static int
rd_nusdef_type(int nsd1, char ***types)
{
    int     i, n, rt;
    char   *ary[1000], str[17];
    int     num = 0;
    
    rt = nusdas_parameter_change(N_PC_ID_SET, &nsd1);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET,%d)\n",rt,nsd1);
    while (nusdas_scan_ds(str,str+8,str+12,&n) == 0) {
        if (n != nsd1) continue;
        str[16] = '\0';
        if (debug>1) printf("NUSDAS%02d: %s \n",n,str);
        ary[num++] = strdup(str);
    }
    *types = (char **)malloc(num * sizeof(char *));
    memcpy(*types,ary,num * sizeof(char *));
    if (debug) printf("NUSDAS%02d: nusdas_scan_ds() total num = %d \n", nsd1, num);
    return  num;
}


int
main(int argc, char **argv)
{
    int     i, rt, rec = 0;

    int     narg = 0;
    int     ng = 0;
    int     ntype, nsd1, nsd2;
    char  **types;

    myname = argv[0];
    nsd1 = nsd2 = -1;
    while (++narg < argc) {
        int     y, m, d, h, g;
        char   *p = argv[narg];
        if (*p == '-') {
            char   *p1 = p;
            while(*(++p1)) {
                if (strncmp(p1,"D2",2) == 0) { debug = 2; break; }
                else if (strncmp(p1,"-quiet",6) == 0) {
                    quiet = atoi(p1 += 6);
                    if (quiet <= 0) quiet = 1;
                    if (quiet >  9) quiet = 1;
                    break;
                }
                else if (strncmp(p1,"-ispcall",8) == 0) { ispec =  1; break; }
                else if (strncmp(p1,"-no-data",8) == 0) { fdata =  0; break; }
                else if (strncmp(p1,"-no-subc",5) == 0) { fsubc = -1; break; }
                else if (strncmp(p1,"-force"  ,6) == 0) { force =  1; break; }
                else if (strncmp(p1,"-nusbuf" ,7) == 0) { noclose = 0; break; }
                else if (strncmp(p1,"-noclose",8) == 0) { noclose = 1; break; }
                else if (strncmp(p1,"-rege"   ,5) == 0) { regexp  = 1; break; }
                else if (strncmp(p1,"-average",8) == 0) { iaverage = 1; break; }
                else if (*p1 == '0') debug = 0;
                else if (*p1 == 'z') btdif = 1;
                else if (*p1 == 'x') prhex = 1;
                else if (*p1 == 'X') large = 1;
                else if (*p1 == 'n') pmopt |= 1;
                else if (*p1 == 'h') usage(0,0,NULL);
                else if (*p1 == '?') usage(0,0,NULL);
                else if (*p1 == 'D') {
                    if (*(p1+1) == '\0') debug = 1;
                    else                 debug = max(0,atoi(p1+1));
                    break;
                }
                else if (*p1 == 'd') {
                    verbs = max(1,atoi(p1+1));
                    break;
                } else if (*p1 == 's') {
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    acc_s = atoi(p1);
                    break;
                } else if (*p1 == 'r') {
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    acc_r = atoi(p1);
                    break;
                } else if (*p1 == 't') {
                    char   *p2;
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    p1 = strdup(p1);
                    if ((p2 = strchr(p1,',')) != NULL) *p2++ = '\0';
                    seltpsrc[seltpnum] = p1;
                    seltpdst[seltpnum] = p2;
                    ++seltpnum;
                    break;
                } else if (*p1 == 'b') {  /* yyyymmddhhgg */
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    sscanf(p1,"%4d%2d%2d%2d%2d",&y,&m,&d,&h,&g);
                    selbt2 = selbt1 = nwp_ymdhm2seq(y,m,d,h,g);
                    if ((p1 = strchr(p1,',')) != NULL) {
                        sscanf(p1,",%4d%2d%2d%2d%2d",&y,&m,&d,&h,&g);
                        selbt2 = nwp_ymdhm2seq(y,m,d,h,g);
                    }
                    break;
                } else if (*p1 == 'v') {  /* yyyymmddhhgg */
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    sscanf(p1,"%4d%2d%2d%2d%2d",&y,&m,&d,&h,&g);
                    selvt2 = selvt1 = nwp_ymdhm2seq(y,m,d,h,g);
                    if ((p1 = strchr(p1,',')) != NULL) {
                        sscanf(p1,",%4d%2d%2d%2d%2d",&y,&m,&d,&h,&g);
                        selvt2 = nwp_ymdhm2seq(y,m,d,h,g);
                    }
                    break;
                } else if (*p1 == 'm') {  /* member */
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    selmm = p1;
                    break;
                } else if (*p1 == 'p') {  /* plane */
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    selpl = p1;
                    break;
                } else if (*p1 == 'e') {  /* element */
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    selel = p1;
                    break;
                } else {
                    printf("!E  unknown option (%s)\n", p);
                    usage(0,0,NULL);
                }
            }
        } else {
            if      (nsd1 < 0) nsd1 = atoi(p);
            else if (nsd2 < 0) nsd2 = atoi(p);
            else {
                printf ("too many arguments. %s \n", p);
                exit(1);
            }
        }
    }

    if (nsd2 <= 0) usage(0,0,NULL);
    acc_r = max(acc_r, acc_s);
    if (debug) {
        printf ("debug  = %d \n", debug);
        printf ("verbs  = %d \n", verbs);
        printf ("noclose= %d \n", noclose);
        printf ("average= %d \n", iaverage);
        printf ("quiet  = %d \n", quiet);
        printf ("nsd1,nsd2 = %d, %d \n", nsd1, nsd2);
        printf ("acc_s,acc_r = %d, %d \n", acc_s, acc_r);
        printf ("select type (%d)\n", seltpnum);
        for (i=0; i<seltpnum; i++) printf ("            %-16s : %s\n", seltpsrc[i], (seltpdst[i] == NULL) ? "" : seltpdst[i]);
        printf ("select basetime  = %d, %d \n", selbt1, selbt2);
        printf ("select validtime = %d, %d \n", selvt1, selvt2);
        printf ("select member     (%s)    \n", selmm);
        printf ("select plane      (%s)    \n", selpl);
        printf ("select element    (%s)    \n", selel);
        printf ("long long size = %d \n", long_long_size);
        printf ("==\n");
    }

    nusdas_iocntl(N_IO_WARNING_OUT,debug);

    if (noclose > 0) {
        nusdas_iocntl(N_IO_W_FCLOSE,N_OFF);
        nusdas_iocntl(N_IO_R_FCLOSE,N_OFF);
    }

    ntype = rd_nusdef_type(nsd1,&types);
    for (i=0; i<ntype; i++) {
        int     n;
        char    dstyp[17];

        strncpy(dstyp, types[i], 16);
        dstyp[16] = '\0';

        if (seltpnum > 0) {
            int     j, len;
            for (j=0; j<seltpnum; j++) {
                char   *src = seltpsrc[j];
                char   *dst = seltpdst[j];
                if (strncmp(types[i],src,strlen(src)) == 0) {
                    if (dst != NULL) {
                        len = strlen(dst);
                        strncpy(dstyp + 16 - len, dst, len);
                    }
                    break;
                }
            }
            if (j >= seltpnum) continue;
        }

        printf("\n  * compare(%s)\n", types[i]);
        rt = proc_type(nsd1, nsd2, types[i], dstyp, &n);
        rec += n;
        if (rt >= 0) {
            if (strcmp(types[i], dstyp) != 0) printf("  %s: NUSDAS%02d \n", dstyp, nsd2);
            printf("  %s: DIFF %s count = %d/%d \n", types[i], fdata ? "recode" : "NuSDaS INDEX FLAG", rt, n);
            ng += rt;
        } else {
            printf("!W  return code = %d: %s \n", rt, types[i]);
        }
    }
    nusdas_allfile_close(N_FOPEN_ALL);

    printf("\n    Total DIFF %s count = %d/%d \n", fdata ? "recode" : "NuSDaS INDEX FLAG", ng, rec);
#ifdef TEST
    printf("DIFF OK EQ !!!!!!!!!! TEST !!!!!!!!!!\n");
#endif

    return  0;
}
