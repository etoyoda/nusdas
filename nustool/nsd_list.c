/*
#! cc -64 -O % nsd_param.c nsd_subc.c \
          -I /grpK/nwp/Open/Module/Comm/Lib/Nwp/Inc13 \
          -L /grpK/nwp/Open/Module/Comm/Lib/Nwp  -lnusdas13  -lnwp13  -lm
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
int     exitstatus  = 0;
int     debug  = 0;
int     ispec  = 0;
int     subc   = 0;              /* -d                       */
int     verbs  = 0;              /* -d2                      */
static int     noclose= 0;       /* --noclose                */
static int     large  = 0;
static int     force  = 0;
static int     pmopt  = 0;       /* none |...| missing | num */
static int     selbt1 = 0;       /* -b yyyymmddhh            */
static int     selbt2 = 0;       /* -b           ,yyyymmddhh */
static int     selvt1 = 0;       /* -v yyyymmddhh            */
static int     selvt2 = 0;       /* -v           ,yyyymmddhh */
static char   *seltp  = NULL;    /* -t t1t1t1t1t2t2t3t3      */
static int     regexp = 0;
static char   *selmm  = NULL;    /* -m member                */
static char   *selpl  = NULL;    /* -p plane                 */
static char   *selel  = NULL;    /* -e element               */

static char *
usage(int n, int argc, char *argv[])
{
    char   *p = strrchr(myname,'/');
    char   *msg = "\n\
    NuSDaS Data List  NRD （NRD は NUSDAS番号）。\n\
    -q    : typeのみを表示する。(-q|-qd|-qd -t...)\n\
    -t... : type1 type2 type3 を限定する。(_DCDSTSFOBSVAMD4)\n\
    -b... : basetime を限定する。 (200706270000[,200706271200])\n\
    -v... : validtime を限定する。(200706271200[,200706301200])\n\
    -m... : member を限定する。\n\
    -p... : plane を限定する。\n\
    -e... : element を限定する。\n\
    -d    : その他の情報を表示する。\n\
    -d2   : その他の情報とデータ値(-d3)を表示する。\n\
    -D    : デバッグ情報を表示する\n\
    --regexp  : -m -p -e の比較に正規表現を使う。 \n\
    --noclose : NUSDAS read write で file close しない\n\
    --ispcall : SUBC ISPC 512(byte)全てを表示する \n\
\n";
    if (n < argc) return argv[n];
    printf("\n  usage: %s [option] NRD\n", (p == NULL ? myname : p+1));
    printf(msg);
    exit(0);
}


int
proc_read(char *type, int bt, char *mb, int vt, char *pl, char *el)
{
    int     i, j, k, rt, xy[2], sz, dsz = 4;
    char   *dtyp = NULL, pac[5];
    void   *data;
    char   *n_r4 = N_R4;
    char   *n_i4 = N_I4;
    char   *n_r8 = N_R8;

    i = 1;
    rt = nusdas_inq_data(type,type+8,type+12,&bt,mb,&vt,pl,el,N_PC_PACKING, (int *)pac,&i);
    if (rt != 1) {
        strcpy(pac,"????");
        printf("!E  rt=%d,  nusdas_inq_data(%s,%d,%s,%d,%s,%s,N_PC_PACKING) \n", rt, type, bt, mb, vt, pl, el);
        return  -999;
    } else {
        pac[4] = '\0';
    }

    if      (strncmp(pac,N_P_2PAC,4) == 0) { dtyp = n_r4; dsz = 4; }
    else if (strncmp(pac,N_P_2UPC,4) == 0) { dtyp = n_r4; dsz = 4; }
    else if (strncmp(pac,N_P_2UPP,4) == 0) { dtyp = n_r4; dsz = 4; }
    else if (strncmp(pac,N_P_2UPJ,4) == 0) { dtyp = n_r4; dsz = 4; }
    else if (strncmp(pac,N_P_1PAC,4) == 0) { dtyp = n_r4; dsz = 4; }
    else if (strncmp(pac,N_P_R4,4)   == 0) { dtyp = n_r4; dsz = 4; }
    else if (strncmp(pac,N_P_I1,4)   == 0) { dtyp = n_i4; dsz = 4; }
    else if (strncmp(pac,N_P_I2,4)   == 0) { dtyp = n_i4; dsz = 4; }
    else if (strncmp(pac,N_P_I4,4)   == 0) { dtyp = n_i4; dsz = 4; }
    else if (strncmp(pac,N_P_N1I2,4) == 0) { dtyp = n_i4; dsz = 4; }
    else if (strncmp(pac,N_P_RLEN,4) == 0) { dtyp = n_i4; dsz = 4; }
    else if (strncmp(pac,N_P_R8,4)   == 0) {
         if (large == 1) {                   dtyp = n_r8; dsz = 8;
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
    if ((data = malloc(sz * dsz)) == NULL) {
        printf("!E  malloc(%d byte)\n", sz * dsz);
        exit(1);
    }
    rt = nusdas_read(type,type+8,type+12,&bt,mb,&vt,pl,el,data,dtyp,&sz);
    if (rt <= 0) {
        printf("!E  rt=%d,  nusdas_read(%s,%d,%s,%d,%s,%s \n", rt, type, bt, mb, vt, pl, el);
        free(data);
        return  rt;
    } else {
        if (dtyp == n_i4) {
            int     imax = INT_MIN;
            int     imin = INT_MAX;
            for (j=0; j<xy[1]; j++) {
                for (i=0; i<xy[0]; i++) {
                    int     k = j * xy[0] + i;
                    int     d = *((int *)data + k);
                    if (verbs > 2) printf("    data[%d](%d,%d) %12d \n", k, i, j, d);
                    if (d != N_MV_SI4) {
                        if (imax < d) imax = d;
                        if (imin > d) imin = d;
                    }
                }
            }
            printf("         (min, max) = (%d, %d) [%s]\n", imin, imax, dtyp);
        } else if (dtyp == n_r4) {
            float   fmax = FLT_MIN;
            float   fmin = FLT_MAX;
            for (j=0; j<xy[1]; j++) {
                for (i=0; i<xy[0]; i++) {
                    int     k = j * xy[0] + i;
                    float   d = *((float *)data + k);
                    if (verbs > 2) printf("    data[%d](%d,%d) %g \n", k, i, j, d);
                    if (d != N_MV_R4) {
                        if (fmax < d) fmax = d;
                        if (fmin > d) fmin = d;
                    }
                }
            }
            printf("         (min, max) = (%g, %g) [%s]\n", fmin, fmax, dtyp);
        } else if (dtyp == n_r8) {
            long double   fmax = DBL_MIN;
            long double   fmin = DBL_MAX;
            for (j=0; j<xy[1]; j++) {
                for (i=0; i<xy[0]; i++) {
                    int         k = j * xy[0] + i;
                    long double d = *((long double *)data + k);
                    if (verbs > 2) printf("    data[%d](%d,%d) %g \n", k, i, j, d);
                    if (d != N_MV_R8) {
                        if (fmax < d) fmax = d;
                        if (fmin > d) fmin = d;
                    }
                }
            }
            printf("         (min, max) = (%g, %g) [%s]\n", fmin, fmax, dtyp);
        }
    }
    free(data);

    return  rt;
}


static int  prnum = 0;

int
proc_list(int nsd, char *types, int bt, char *mmb, int vt, int n_pl, char *plnp, int n_el, char *elmp, char *map, int *n)
{
    int     i, l, e, rt = 0;
    int     pck[2];
    int     py, pm, pd, ph, pg;
    char    abt[20], avt[20];
    char   *t1 = types;
    char   *t2 = types + 8;
    char   *t3 = types + 12;
    int     all = n_pl * n_el;

    if (debug > 1) {
        printf("proc_list: n_pl = %d (%s)\n", n_pl, plnp);
        printf("           n_el = %d (%s)\n", n_el, elmp);
        printf("       data map = ");
        for(i=0; i<all; i++) printf("%d", map[i]);
        printf("\n");
    }

    nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, bt);
    sprintf(abt, "%04d%02d%02d%02d%02d", py, pm, pd, ph, pg);
    nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, vt);
    sprintf(avt, "%04d%02d%02d%02d%02d", py, pm, pd, ph, pg);

    if (debug) printf("   nusdas_inq_cntl(%s:%s:%s:%s) \n", types, abt, mmb, avt);
    for (l=0; l<n_pl; l++) {
        char    pln[7], elm[7], pack[5];

        strncpy(pln, plnp + l * 6, 6); pln[6] = '\0';

        /* if ((selpl[0] != '\0') && (strncmp(pln,selpl,strlen(selpl)) != 0)) continue; * select plane */
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

        for (e=0; e<n_el; e++) {
            int     byte, size[2];
            int     pos = l * n_el + e;
            char    str[100], tmp[100];

            if (map[pos] == 0) continue;

            strncpy(elm, elmp + e * 6, 6); elm[6] = '\0';

            /* if ((selel[0] != '\0') && (strncmp(elm,selel,strlen(selel)) != 0)) continue; * select element */
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

            i = 2;
            rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_GRID_SIZE,&size,&i);
            if (rt != i) exitstatus = 65402;
            i = 1;
            rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_PC_PACKING,pack,&i); pack[4] = '\0';
            if (rt != i) exitstatus = 65403;
            i = 1;
            rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_DATA_NBYTES,&byte,&i);
            if (rt != i) exitstatus = 65406;

            ++prnum;

            if (pmopt & 1) sprintf(str, "%s %d ", rt == 1 ? "  ":"!E", prnum);
            else           sprintf(str, "%s", rt == 1 ? "  ":"!E");

            if (pmopt & 2) {
                unsigned char    q[16];
                unsigned int     v;
                i = 16;
                nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_DATA_QUADRUPLET,q,&i);
                i = 1;
                nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_MISSING_VALUE,&v,&i);
                sprintf(tmp, "%c%c%c%c %08x" ,q[12],q[13],q[14],q[15], v);
            } else tmp[0] = '\0';

            printf("%sDATA %s:%s:%s:%s:%s:%s:%s(%d,%d)%6d%s\n", str, t3, abt, mmb, avt, pln, elm, pack, size[0], size[1], byte, tmp);
            if (verbs > 1) {
                int     itms = size[0] * size[1];
                int     sz = itms * (large == 1 ? 8 : 4);
                proc_read(types, bt, mmb, vt, pln, elm);
            }
            if (force != 0 && exitstatus != 0) return  rt;

            ++(*n);
            if (subc) {
                struct nsd_subc_s  subc1;
                subc1.nsd   = nsd;
                subc1.bt    = bt;
                subc1.types = types;
                rt = proc_subc(&subc1, NULL, mmb, vt, pln, elm, NULL);
            }
        }
    }

    return  rt;
}

int
proc_info(char *types, int bt, char *mmb, int vt)
{
    int     i, n, rt;
    char   *t1 = types;
    char   *t2 = types + 8;
    char   *t3 = types + 12;
    char    dnull[4];

    /* INFO */
    if ((rt = nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_INFO_NUM,dnull,&n,1)) == 1) {
        if (n > 0) {
            char   *group = calloc(n * 4 + 1, 1);
            if (group == NULL) { printf("!E  group = calloc(%d)\n", n * 4 + 1); exit(1); }
            rt = nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_INFO_LIST,dnull,group,n);
            printf("  info group = %d (%s) %s\n", n, group, rt == n ? "":"Error");
            for (i=0; i<n; i++) {
                char   *p = group + i*4;
                char   *info = NULL;
                int     byte;
                rt = nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_INFO_NBYTES,p,&byte,1);
                info = malloc(byte+1);
                if (info == NULL) { printf("!E  info = malloc(%d)\n", byte+1); exit(1); }
                rt = nusdas_info(t1,t2,t3,&bt,mmb,&vt,p,info,&byte,"GET");
                if (info != NULL) free(info);
            }
            if (group != NULL) free(group);
        }
    } else {
        printf("rt=%d: nusdas_inq_subcinfo(%s,%d,%s,%d,N_INFO_NUM)\n", rt, types, bt, mmb, vt);
    }

    return  0;
}


int
proc_type(int nsd, char *types, int *n)
{
    struct nsd_parm   csd;
    int     i, rt, sz;
    int     b, m, k;
    char   *t1 = types;
    char   *t2 = types + 8;
    char   *t3 = types + 12;
  
    rt = nsd_param(types, &csd, selbt1, selbt2);
    if (rt != 0) return rt;
    csd.nsd = nsd;

    prnum = 0;

    for (b=0; b<csd.nb; b++) {
        int     py, pm, pd, ph, pg;
        int     save_count = *n;
        int     bt = csd.bt[b];                                                 /* nsd1: basetime */

        if ((selbt2 != 0) && (selbt1 > bt)) continue;                           /* select basetime */
        if ((selbt2 != 0) && (selbt2 < bt)) continue;                           /* select basetime */

        if (selvt2 != 0) {                                                      /* select validtime, search in basetime */
            for (k=0; k<csd.nf; k++) {
                int     vt = time_add(bt, csd.ft[k], csd.fu);
                if ((selvt1 <= vt) && (vt <= selvt2)) break;
            }
            if (k >= csd.nf) continue;
        }

        nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, bt);
        printf("     basetime (%04d%02d%02d%02d%02d) found\n", py, pm, pd, ph, pg);

        for (m=0; m<csd.nm; m++) {
            char    mmb[5];

            strncpy(mmb, csd.mb + m * 4, 4); mmb[4] = '\0';

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

            for (k=0; k<csd.nf; k++) {
                int     vt = time_add(bt, csd.ft[k], csd.fu);                   /* nsd1: validtime */
                int     n_mb, n_vt, n_pl, n_el, n_mp;
                N_SI4         *tvt = NULL;
                char    *tmb = NULL,       *pln = NULL, *elm = NULL, *map = NULL;
                int     im, iv, ip, ie;

                if ((selvt2 != 0) && (selvt1 > vt)) continue;                   /* select validtime */
                if ((selvt2 != 0) && (selvt2 < vt)) continue;

                n_mb = n_vt = n_pl = n_el = 0;
                i = 1;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_MEMBER_NUM,&n_mb,&i) != i) continue;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_VALIDTIME_NUM,&n_vt,&i) != i) continue;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_PLANE_NUM,&n_pl,&i) != i) continue;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_ELEMENT_NUM,&n_el,&i) != i) continue;

                tmb = calloc(n_mb * 4 + 1, 1);
                tvt = malloc(n_vt * 4);
                pln = calloc(n_pl * 6 + 1, 1);
                elm = calloc(n_el * 6 + 1, 1);
                memset(elm, '?', n_el * 6);
                elm[n_el * 6] = '\0';

                if (tmb == NULL) { printf("!E  tmb = calloc(%d)\n", n_mb * 4 + 1); exit(1); }
                if (tvt == NULL) { printf("!E  tvt = malloc(%d)\n", n_vt * 4); exit(1); }
                if (pln == NULL) { printf("!E  pln = calloc(%d)\n", n_pl * 6 + 1); exit(1); }
                if (elm == NULL) { printf("!E  elm = calloc(%d)\n", n_el * 6 + 1); exit(1); }

                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_MEMBER_LIST,tmb,&n_mb) != n_mb) continue;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_VALIDTIME_LIST,tvt,&n_vt) != n_vt) continue;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_PLANE_LIST,pln,&n_pl) != n_pl) continue;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_ELEMENT_LIST,elm,&n_el) != n_el) continue;
                if (debug) {
                    printf("    nusdas_inq_cntl(N_ELEMENT_LIST) DATA file infometion\n");
                    printf("    %s\n",elm);
                }


                for (im=0; im<n_mb; im++) if (strncmp(tmb+im*4,mmb,4) == 0) break;
                if (im >= n_mb) goto cont;
                for (iv=0; iv<n_vt; iv++) if (tvt[iv] == vt) break;
                if (iv >= n_vt) goto cont;

                i = n_mb * n_vt * n_pl * n_el;
                map = malloc(i);
                if (map == NULL) { printf("!E  map = malloc(%d)\n", i); exit(1); }
                n_mp = nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_DATA_MAP,map,&i);
                if (debug) {
                    printf("n_mb,n_vt,n_pl,n_el = %d, %d, %d, %d \n",n_mb,n_vt,n_pl,n_el);
                    printf("datamap=%d:  (%d:%s:%d)\n", n_mp, bt, mmb, vt);
                    for(i=0; i<n_mp; i++) printf("%d",map[i]);
                    printf("\n");
                }

                i = (im * n_vt + iv) * (n_pl * n_el);
                proc_list(nsd, types, bt, mmb, vt, n_pl, pln, n_el, elm, map + i, n);
                if (subc) proc_info(types, bt, mmb, vt);

                if (map != NULL) free(map);
            cont:
                if (tmb != NULL) free(tmb);
                if (tvt != NULL) free(tvt);
                if (pln != NULL) free(pln);
                if (elm != NULL) free(elm);
                if (force != 0 && exitstatus != 0) return  rt;
            }
        }
        if (*n == save_count)
            printf("     basetime (%04d%02d%02d%02d%02d) data record nothing!!!.\n", py, pm, pd, ph, pg);
        else printf("     %d records in (%04d%02d%02d%02d%02d)\n\n", *n - save_count, py, pm, pd, ph, pg);
    }

    return  0;
}


static int
rd_nusdef_type(int nsd, char ***types)
{
    int     i, n, rt;
    char   *ary[1000], str[17];
    int     num = 0;
                                                                                                                                                             
    while (nusdas_scan_ds(str,str+8,str+12,&n) == 0) {
        if (n != nsd) continue;
        str[16] = '\0';
        if (debug) printf("NUSDAS%02d:%s \n",n,str);
        ary[num++] = strdup(str);
    }
    *types = (char **)malloc(num * sizeof(char *));
    if (*types == NULL) { printf("!E  *types = malloc(%d)\n", num * sizeof(char *)); exit(1); }
    memcpy(*types,ary,num * sizeof(char *));
    if (debug) printf("nusdas_scan_ds(%d) num = %d \n", nsd, num);
    return  num;
}


int
main(int argc, char **argv)
{
    int     i, rt, total, *rec;

    int     narg = 0;
    int     ntype, nsd;
    char  **types;

    myname = argv[0];
    nsd = -1;
    while (++narg < argc) {    /* 引数の解析 */
        int     y, m, d, h, g;
        char   *p = argv[narg];
        if (*p == '-') {
            char   *p1 = p;
            while(*(++p1)) {
                if (strncmp(p1,"D2",2) == 0) { debug = 2; break; }
                else if (strncmp(p1,"-ispcall",8) == 0) { ispec = 1; subc = max(1,subc); break; }
                else if (strncmp(p1,"-missing",8) == 0) { pmopt |= 2; break; }
                else if (strncmp(p1,"-nusbuf" ,7) == 0) { noclose = 0; break; }
                else if (strncmp(p1,"-noclose",8) == 0) { noclose = 1; break; }
                else if (strncmp(p1,"-rege"   ,5) == 0) { regexp  = 1; break; }
                else if (*p1 == 'D') debug = 1;
                else if (*p1 == '0') debug = 0;
                else if (*p1 == 'f') force = 1;
                else if (*p1 == 'X') large = 1;
                else if (*p1 == 'n') pmopt |= 1;
                else if (*p1 == 'q') pmopt |= 0x8000;
                else if (*p1 == 'h') usage(0,0,NULL);
                else if (*p1 == '?') usage(0,0,NULL);
                else if (*p1 == 'd') {
                    verbs = atoi(p1+1);
                    subc = max(1,verbs);
                    break;
                } else if (*p1 == 't') {
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    seltp = p1;
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
                } else if (*p1 == 'v') {  /* yyyymmddhhgg,yyyymmddhhgg */
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
            if      (nsd < 0) nsd = atoi(p);
            else {
                printf ("too many arguments. %s \n", p);
                exit(1);
            }
        }
    }

    if (nsd <= 0) usage(0,0,NULL);

    if (debug) {
        printf ("debug  = %d \n", debug);
        printf ("noclose= %d \n", noclose);
        printf ("subc   = %d \n", subc);
        printf ("nsd    = %d \n", nsd);
        printf ("select type (%s) \n", seltp);
        printf ("select basetime  = %d, %d \n", selbt1, selbt2);
        printf ("select validtime = %d, %d \n", selvt1, selvt2);
        printf ("select member     (%s)    \n", selmm);
        printf ("select plane      (%s)    \n", selpl);
        printf ("select element    (%s)    \n", selel);
    }

    nusdas_iocntl(N_IO_WARNING_OUT,debug);

    rt = nusdas_parameter_change(N_PC_ID_SET, &nsd);
    if (debug) printf("nusdas_parameter_change(N_PC_ID_SET,%d)=%d\n",nsd,rt);

    ntype = rd_nusdef_type(nsd,&types);

    rec = calloc(ntype,4);
    if (rec == NULL) { printf("!E  rec = calloc(%d)\n", ntype); exit(1); }
    if (pmopt & 0x8000) {
        for (i=0; i<ntype; i++) {
            struct nsd_parm   csd;
            int    rt = nsd_param(types[i], &csd, selbt1, selbt2);
            printf("    %s  (%d,%d,%d,%d,%d)\n", types[i], csd.nb, csd.nm, csd.nf, csd.np, csd.ne);
            if (subc) {
                int     j, py, pm, pd, ph, pg;
                if ((seltp != NULL) && (strncmp(types[i],seltp,strlen(seltp)) != 0)) continue;
                for (j=0; j<csd.nb; j++) {
                    if ((j % 5) == 0) printf("  ");
                    nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, csd.bt[j]);
                    printf("  %04d%02d%02d%02d%02d", py, pm, pd, ph, pg);
                    if (((j+1) % 5) == 0) printf("\n");
                }
                if (j % 5) printf("\n");
                printf("    member[%s]\n", csd.mb);
                printf("    valid:");
                for (j=0; j<csd.nf; j++) printf("  %d", csd.ft[j]); printf("\n");
                printf("    plene[%s]\n", csd.pl);
                printf("    element[%s]\n", csd.el);
            }
        }
    } else {

        if (noclose > 0) {
            nusdas_iocntl(N_IO_W_FCLOSE,N_OFF);
            nusdas_iocntl(N_IO_R_FCLOSE,N_OFF);
        }

        for (i=0; i<ntype; i++) {

            if ((seltp != NULL) && (strncmp(types[i],seltp,strlen(seltp)) != 0)) continue;

            printf("  %s : list\n", types[i]);
            rt = proc_type(nsd, types[i], rec+i);
            if (rt < 0) printf("!W  record = %d: %s \n", rec[i], types[i]);
            printf("\n");
        }

        nusdas_allfile_close(N_FOPEN_ALL);

        total = 0;
        for (i=0; i<ntype; i++) {
            if ((seltp != NULL) && (strncmp(types[i],seltp,strlen(seltp)) != 0)) continue;
            printf("    %s: record = %d \n", types[i], rec[i]);
            total += rec[i];
        }
        printf("    Total record = %d \n", total);
    }

    if (exitstatus == 0) return 0;

    printf("!Error code = %d \n", exitstatus);
    return  99;
}
