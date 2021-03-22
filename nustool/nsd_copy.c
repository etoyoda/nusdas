/*
#! cc -64 -O -o nsd_copy  %  obstation.c  nsd_param.c  nsd_subc.c \
          -I /shrtK2/npd/suuchi80/Module/Nustool/Src \
          -I /dvlK2/npd1/npd_pg/suuchi26/NuSDaS/Comm/Lib/Nwp/Inc13 \
          -L /dvlK2/npd1/npd_pg/suuchi26/NuSDaS/Comm/Lib/Nwp  -lnusdas13  -lnwp13  -lm


#! cc -64 -O % nsd_param.c nsd_subc.c obstation.c \
          -I /grpK/nwp/Open/Module/Comm/Lib/Nwp/Inc13 \
          -L /grpK/nwp/Open/Module/Comm/Lib/Nwp  -lnusdas13  -lnwp13  -lm
*/

#include   <stdio.h>
#include   <stdlib.h>
#include   <math.h>
#include   <string.h>
#include   <regex.h>
#include   "nsd_param.h"
#include   "nsd_subc.h"

#ifndef IG_CREATE_LAT
#include   "obstation.h"
#endif

#define    max(a,b)    ((a)>(b)?(a):(b))
#define    min(a,b)    ((a)<(b)?(a):(b))

char   *myname = "?";
int     exitstatus  = 0;
int     ispec  = 0;
int     debug  = 0;              /* -Dn                                   */
int     subc   = 0;              /* -d                                    */
int     verbs  = 0;              /* -d2                                   */
int     quiet  = 0;              /* --quiet                               */
int     qtime  = 0;              /* --qtime                               */
int     mask   = 0;              /* --mask                                */
int     n_nc   = 0;              /* --n_nc                                */
static int     noclose= 0;       /* --noclose                             */
static int     no_subc= 0;       /* --no-subc                             */
static int     over   = 0;       /* --overwrite | --no_overwrite          */
static int     selbt1 = 0;       /* -b yyyymmddhh,                        */
static int     selbt2 = 0;       /* -b           ,yyyymmddhh              */
static int     selvt1 = 0;       /* -v yyyymmddhh,                        */
static int     selvt2 = 0;       /* -v           ,yyyymmddhh              */
static int     beta   = 0;       /* -zB                                   */
static int     alpha  = 0;       /* -z ,A                                 */
static char   *seltpsrc[10];     /* -t t1t1t1t1t2t2t3t3                   */
static char   *seltpdst[10];     /*                    ,t1t1t1t1t2t2t3t3  */
static int     seltpnum = 0;     /*    type table number                  */
static int     regexp = 0;
static char   *selmm = NULL;     /* -m member                             */
static char   *selpl = NULL;     /* -p plane                              */
static char   *selel = NULL;     /* -e element                            */
static int     warnend = 0;      /* -w warnend                            */
static int     create_LAT = 0;
static int     create_LON = 0;
static int     create_HIGH = 0;

static char *
usage(int n, int argc, char *argv[])
{
    char   *p = strrchr(myname,'/');
    const static char   *msg = "\n\
    NuSDaS Copy  NRD1 --> NRD2 （NRD1 NRD2 は NUSDAS番号）。\n\
    -t...   : type1 type2 type3 に限定する。(_DCDSTSFOBSVAMD4)\n\
    -b...   : basetime に限定する。 (200704051200,200704051200)\n\
    -v...   : validtime に限定する。(200704051200,200704051200)\n\
    -p...   : plane に限定する。\n\
    -e...   : element に限定する。\n\
    -m...   : member に限定する。\n\
    -d      : SUBC等のコピーを表示。\n\
    -zB,A   : basetime を変更してコピー。basetime = INT((validtime + A) / B) * B\n\
    --overwrite : コピー元のデータレコード作成時刻が古くてもコピーする\n\
    --noclose   : NUSDAS read write で file close しない\n\
    --no-subc   : 書き込み時にSUBCの書き込みをしない\n\
    --mask  : コピーする時、mask bit を作成する \n\
    --n_nc  : UPCのデータは無変換でコピーする\n\
    --quiet : コピーした要素毎の表示をしない \n\
    --qtime : コピーした時刻を表示する \n\
    -W      : 警告(!W)発生時に処理を継続せずに異常終了させる \n\
    -D      : デバッグ出力\n\
    ( type を変更してコピーする -t t1t1t1t1t2t2t3t3,t1t1t1t1t2t2txtx ) \n\
\n";
    if (n >= argc) {
      printf("\n  usage: %s [option] NRD1  NRD2\n", (p == NULL ? myname : p+1));
      printf(msg);
      exit(argv ? 1 : 0);
    }
    return  argv[n];
}


static void
check_warning()
{
    if(warnend) exit(10);
}

int
proc_copy(struct nsd_parm csd1, struct nsd_parm csd2, int bt, char *mmb, int vt, int n_pl, char *plnp, int n_el, char *elmp, char *map, void *dat, int sz, int *copies)
{
    int     i, l, e, rt = 0;
    int     copy = 0, inflag = 0;
    int     py, pm, pd, ph, pg;
    char    abt[20], avt[20];
    char   *t1, *t2, *t3;
    int     all = n_pl * n_el;
    char   *cplist = malloc(all+1);

    if (cplist == NULL) { printf("!E  cplist = malloc(%d)\n", all+1); exit(1); }
    for (i=0; i<all; i++) cplist[i] = '-';
    cplist[all] = '\0';

    if (debug > 1) {
        printf("proc_copy: n_pl = %d (%s)\n", n_pl, plnp);
        printf("           n_el = %d (%s)\n", n_el, elmp);
        printf("       data map = ");
        for(i=0; i<all; i++) printf("%d", map[i]);
        printf("\n");
    }

    nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, bt);
    sprintf(abt, "%04d%02d%02d%02d%02d", py, pm, pd, ph, pg);
    nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, vt);
    sprintf(avt, "%04d%02d%02d%02d%02d", py, pm, pd, ph, pg);

    if (debug) printf("   nusdas_inq_cntl(%s:%s:%s:%s) \n", csd1.types, abt, mmb, avt);
    for (l=0; l<n_pl; l++) {
        char    pln[7], elm[7], pack[5];

        strncpy(pln, plnp + l * 6, 6); pln[6] = '\0';

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

        for (i=0; i<csd2.np; i++) if(strncmp(csd2.pl + i*6, pln, 6) == 0) break;
        if (i >= csd2.np) continue;

        for (e=0; e<n_el; e++) {
            int     len, size[2], flag;
            unsigned int    tm1, tm2;
            char   *dtyp, wkwk[5];
            int     pos = l * n_el + e;
            int     ncflag;

            if (map[pos] == 0) continue;

            strncpy(elm, elmp + e * 6, 6); elm[6] = '\0';

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

            for (i=0; i<csd2.ne; i++) if(strncmp(csd2.el + i*6, elm, 6) == 0) break;
            if (i >= csd2.ne) continue;

            rt = nusdas_parameter_change(N_PC_ID_SET, &csd1.nsd);
            if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csd1.nsd);
            t1 = csd1.types;
            t2 = csd1.types + 8;
            t3 = csd1.types + 12;

            i = 1;
            rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_DATA_EXIST,&flag,&i);
            if(rt == 1 && flag != 1) continue;

            i = 1;
            rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_RECORD_TIME,&tm1,&i);
            if (rt != i) tm1 = 0;

            i = 1;
            rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_MISSING_MODE,wkwk,&i); wkwk[4] = '\0';
            if (rt != i) exitstatus = 65403;
            else {
                ncflag = (strncmp(wkwk,"MASK",4) == 0) ? 0 : n_nc;
            }

            i = 2;
            rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_GRID_SIZE,&size,&i);
            if (rt != i) exitstatus = 65402;
            i = 1;
            rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_PC_PACKING,pack,&i); pack[4] = '\0';
            if (rt != i) exitstatus = 65403;

            if         (strncmp(pack,N_P_I1  ,4) == 0) {            /* "I1  " */
                dtyp = N_I1;
            } else if ((strncmp(pack,N_P_I2  ,4) == 0) ||           /* "I2  " */
                       (strncmp(pack,N_P_N1I2,4) == 0)    ) {       /* "N1I2" */
                dtyp = N_I2;
            } else if  (strncmp(pack,N_P_I4  ,4) == 0) {            /* "I4  " */
                dtyp = N_I4;
            } else if  (strncmp(pack,N_P_R4  ,4) == 0) {            /* "R4  " */
                dtyp = N_R4;
            } else if ((strncmp(pack,N_P_R8  ,4) == 0) ||           /* "R8  " */
                       (strncmp(pack,N_P_1PAC,4) == 0) ||           /* "1PAC" */
                       (strncmp(pack,N_P_4PAC,4) == 0)    ) {       /* "4PAC" */
                dtyp = N_R8;
            } else if ((strncmp(pack,N_P_2UPC,4) == 0) ||           /* "2UPC" */
                       (strncmp(pack,N_P_2PAC,4) == 0) ||           /* "2PAC" */
                       (strncmp(pack,N_P_2UPJ,4) == 0) ||           /* "2UPJ" */
                       (strncmp(pack,N_P_2UPP,4) == 0)    ) {       /* "2UPP" */
                dtyp = ncflag == 1 ? N_NC : N_R8;
            } else if  (strncmp(pack,N_P_RLEN,4) == 0) {            /* "RLEN" */
                dtyp = N_I4;
            } else {
                printf("!W  unknown packing (%s) %s:%s \n", pack, pln, elm);
                check_warning();
                continue;
            }

            i = size[0] * size[1];
            if ((len = nusdas_read(t1,t2,t3,&bt,mmb,&vt,pln,elm,dat,dtyp,&i)) > 0 && len <= sz) {    /*  NUSDAS_READ  */
                int     bt2 = (beta == 0) ? bt : (int)((vt + alpha) / beta) * beta;
                char    abt2[100];

                ++inflag;

                nwp_seq2ymdhm(&py, &pm, &pd, &ph, &pg, bt2);
                sprintf(abt2, "%04d%02d%02d%02d%02d", py, pm, pd, ph, pg);

                rt = nusdas_parameter_change(N_PC_ID_SET, &csd2.nsd);
                if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csd2.nsd);
                t1 = csd2.types;
                t2 = csd2.types + 8;
                t3 = csd2.types + 12;
                if ((over == 0) && (tm1 > 0)) {
                    i = 1;
                    rt = nusdas_inq_data(t1,t2,t3,&bt,mmb,&vt,pln,elm,N_DATA_EXIST,&flag,&i);
                    if(rt == 1 && flag == 1) {
                        i = 1;
                        if ((rt = nusdas_inq_data(t1,t2,t3,&bt2,mmb,&vt,pln,elm,N_RECORD_TIME,&tm2,&i)) == i) {
                            if (tm2 >= tm1) {
                                if (quiet == 0) printf("  skip! NUSDAS%02d newer than NUSDAS%02d %s:%s:%s:%s\n", csd2.nsd, csd1.nsd, csd1.types, avt, pln, elm);
                                continue;
                            }
                        } else {
                            static int n_n = 0;
                            if (debug) printf("rt=%d  nusdas_inq_data(%s:%s:%s:%s:%s:N_RECORD_TIME) %d\n", rt, csd2.types, mmb, avt, pln, elm, ++n_n);
                        }
                    }
                }

                rt = nusdas_parameter_change(N_PC_PACKING, pack);
                rt = nusdas_parameter_change(N_PC_SIZEX, size); 
                rt = nusdas_parameter_change(N_PC_SIZEY, size+1); 

                if (mask == 1) nusdas_set_mask(t1,t2,t3, dat, dtyp, len);
                rt = nusdas_write(t1,t2,t3,&bt2,mmb,&vt,pln,elm,dat,dtyp,&len);    /*  NUSDAS_WRITE  */
                if (debug) printf("rt=%d  nusdas_write()\n", rt);
                if (rt == len) {
                    struct nsd_subc_s  subc1, subc2;
                    subc1.nsd   = csd1.nsd;
                    subc1.bt    = bt;
                    subc1.types = csd1.types;
                    subc2.nsd   = csd2.nsd;
                    subc2.bt    = bt2;
                    subc2.types = csd2.types;

                    ++copy;
                    if (quiet == 0) printf("  %s%8d:%s:%s:%s:%s:%s:%s:%s(%d,%d)\n", (qtime == 0 ? "DATA" : "COPY")
                                               , rt, abt2, mmb, avt, pln, elm, pack, dtyp, size[0], size[1]);
#ifndef IG_CREATE_LAT
                    if (((create_LAT | create_LON | create_HIGH) != 0) && (strncmp(elm,"NUM   ",6) == 0) ) {
                        int     (*proc)(int, int, float *, float *, int *);

                        if      (strncmp(csd1.types,"_DCDSTSFOBSVAMD4",16) == 0) proc = amd_search;
                        else if (strncmp(csd1.types,"_DCDSTSFOBSVAMDS",16) == 0) proc = amd_search;
                        else if (strncmp(csd1.types,"_DCDSTSFOBSVOBPR",16) == 0) proc = obs_search;
                        else if (strncmp(csd1.types,"_DCDSTSFOBSVPUBR",16) == 0) proc = pub_search;
                        else if (strncmp(csd1.types,"_DCDSTSFOBSVAWSD",16) == 0) proc = aws_search;
                        else                                                proc = NULL;

                        if (proc != NULL) {
                            float   *la = malloc(len * sizeof(float) * 3);
                            float   *lo = la + len;
                            int     *hi = (int *)(lo + len);
                            int      cc = 0;

                            if (la == NULL) { printf("!E  la = malloc(%d)\n", len * sizeof(float) * 3); exit(1); }
                            if      (strncmp(dtyp,N_I1,4) == 0) for (i=0; i<len; i++) cc += proc(*((unsigned char *)dat + i),vt,la+i,lo+i,hi+i) == 0 ? 1: 0;
                            else if (strncmp(dtyp,N_I2,4) == 0) for (i=0; i<len; i++) cc += proc(*((unsigned short *)dat + i),vt,la+i,lo+i,hi+i) == 0 ? 1: 0;
                            else if (strncmp(dtyp,N_I4,4) == 0) for (i=0; i<len; i++) cc += proc(*((int *)dat + i),vt,la+i,lo+i,hi+i) == 0 ? 1: 0;
                            else if (strncmp(dtyp,N_R4,4) == 0) for (i=0; i<len; i++) cc += proc((int)*((float *)dat + i),vt,la+i,lo+i,hi+i) == 0 ? 1: 0;
                            else if (strncmp(dtyp,N_R8,4) == 0) for (i=0; i<len; i++) cc += proc((int)*((double *)dat + i),vt,la+i,lo+i,hi+i) == 0 ? 1: 0;
                            else {
                                printf("!E  unknown data type (%s) \n", dtyp);
                                exit(99);
                            }
                            if (cc > 0) {
                                nusdas_parameter_change(N_PC_PACKING, N_P_R4);
                                if (create_LAT  != 0) nusdas_write(t1,t2,t3,&bt2,mmb,&vt,pln,"LAT   ",la,N_R4,&len);
                                if (create_LON  != 0) nusdas_write(t1,t2,t3,&bt2,mmb,&vt,pln,"LON   ",lo,N_R4,&len);
                                nusdas_parameter_change(N_PC_PACKING, N_P_I2);
                                if (create_HIGH != 0) nusdas_write(t1,t2,t3,&bt2,mmb,&vt,pln,"HIGH  ",hi,N_I4,&len);
                            }
                            if (la != NULL) free(la);
                        }
                    }
#endif
                    if ( no_subc != 1 ){
                      rt = proc_subc(&subc1, &subc2, mmb, vt, pln, elm, "PUT");
                      if (rt != 0) printf("!E rt=%d nusdas_subc(%s:%s:%s:%s:%s:%s)\n", rt, csd2.types, abt2, mmb, avt, pln, elm);
                      if (rt != 0) exit(1);
                    }else{
                      printf("No SUBC write.\n");
                    }
                    cplist[pos] = 'o';
                } else {
                    cplist[pos] = 'x';
                    printf("!W %s%8d:%s:%s:%s:%s:%s:%s:%s(%d,%d)\n", (qtime == 0 ? "DATA" : "COPY")
                               , rt, abt2, mmb, avt, pln, elm, pack, dtyp, size[0], size[1]);
                    check_warning();
                }

            } else {
                printf("!W rt=%d nusdas_read(%s:%s:%s:%s:%s:%s:%s:%s)\n", len, csd1.types, abt, mmb, avt, pln, elm, pack, dtyp);
                check_warning();
            }
        }
    }

    if ((quiet == 1) && (inflag > 0)) {
        if (qtime == 0) printf("    %s", avt);
        else            printf("  COPY %s %s", abt, avt);
        if (all < 60) printf(" %s\n", cplist);
        else          printf("\n  %s\n", cplist);
    }
    if (cplist != NULL) free(cplist);

    if ((qtime == 1) && (inflag > 0)) printf("COPYTIME %s %s %s\n", csd1.types, abt, avt);

    *copies = copy;
    return  rt;
}

int
proc_info(struct nsd_parm csd1, struct nsd_parm csd2, int bt, char *mmb, int vt)
{
    int     i, n, rt;
    char   *t1, *t2, *t3;
    int     bt2 = (beta == 0) ? bt : (int)((vt + alpha) / beta) * beta;
    char    dnull[4];

    /* INFO */
    rt = nusdas_parameter_change(N_PC_ID_SET, &csd1.nsd);
    if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csd1.nsd);
    t1 = csd1.types;
    t2 = csd1.types + 8;
    t3 = csd1.types + 12;
    if ((rt = nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_INFO_NUM,dnull,&n,1)) == 1) {
        if (n > 0) {
            char   *group = calloc(n * 4 + 1, 1);
            if (group == NULL) { printf("!E  group = calloc(%d)\n", n * 4 + 1); exit(1); }
            if (nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_INFO_LIST,dnull,group,n) == n) {
                group[n * 4] = '\0';
                if (quiet == 0) printf("  info group copy %d (%s)\n", n, group);
                for (i=0; i<n; i++) {
                    char   *p = group + i*4;
                    char   *info = NULL;
                    int     byte;
                    if (nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_INFO_NBYTES,p,&byte,1) == 1) {
                        info = malloc(byte+1);
                        if (info == NULL) { printf("!E  info = malloc(%d)\n", byte+1); exit(1); }
                        if ((rt = nusdas_info(t1,t2,t3,&bt,mmb,&vt,p,info,&byte,"GET")) >= 0) {
                            rt = nusdas_parameter_change(N_PC_ID_SET, &csd2.nsd);
                            if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csd2.nsd);
                            t1 = csd2.types;
                            t2 = csd2.types + 8;
                            t3 = csd2.types + 12;
                            rt = nusdas_info(t1,t2,t3,&bt2,mmb,&vt,p,info,&byte,"PUT");
                            if (rt < 0) {
                              printf("!W  rt=%d: nusdas_info(%s,%d,%s,%d,%s,%d,PUT)\n", rt, csd2.types, bt, mmb, vt, p, byte);
                              check_warning();
                            }
                        } else {
                            printf("!W  nusdas_info(%s:%d:%s:%d:%s:%d) = %d\n", csd1.types, bt, mmb, vt, p, byte, rt);
                            check_warning();
                        }
                        if (info != NULL) free(info);
                    } else {
                        printf("!W  nusdas_inq_subcinfo(N_INFO_NBYTES:%s:%d:%s:%d:%s)\n", csd1.types, bt, mmb, vt, p);
                        check_warning();
                    }
                }
            } else {
                printf("!W  nusdas_inq_subcinfo(N_INFO_LIST:%s:%d:%s:%d:%s)\n", csd1.types, bt, mmb, vt, group);
                check_warning();
            }
            if (group != NULL) free(group);
        }
    } else {
        printf("rt=%d: nusdas_inq_subcinfo(%s,%d,%s,%d,N_INFO_NUM)\n", rt, csd1.types, bt, mmb, vt);
    }

    return  0;
}


int
proc_type(int nsd1, char *typ1, int nsd2, char *typ2, int *copies)
{
    struct nsd_parm  csd1, csd2;
    int     i, rt, sz, ncp, copy = 0;
    int     b, m, k;
    char   *t1, *t2, *t3;
    void   *dat = NULL;


    rt = nusdas_parameter_change(N_PC_ID_SET, &nsd2);
    if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, nsd2);
    rt = nsd_param(typ2, &csd2, selbt1, selbt2);
    if (rt != 0) return rt;
    csd2.nsd = nsd2;

    rt = nusdas_parameter_change(N_PC_ID_SET, &nsd1);
    if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, nsd1);
    rt = nsd_param(typ1, &csd1, selbt1, selbt2);
    if (rt != 0) return rt;
    csd1.nsd = nsd1;

#ifndef IG_CREATE_LAT
    if ((strncmp(csd1.types,"_DCDSTSFOBSVOBPR",16) == 0) ||
        (strncmp(csd1.types,"_DCDSTSFOBSVAMD4",16) == 0) ||
        (strncmp(csd1.types,"_DCDSTSFOBSVAMDS",16) == 0) ||
        (strncmp(csd1.types,"_DCDSTSFOBSVPUBR",16) == 0) ||
        (strncmp(csd1.types,"_DCDSTSFOBSVAWSD",16) == 0)   ){
        char   *p1, *p2;
        if (debug) printf("%s  ne=%d...(%s)\n", csd1.types, csd1.ne, csd1.el);
        if (debug) printf("%s  ne=%d...(%s)\n", csd1.types, csd2.ne, csd2.el);
        p1 = strstr(csd1.el,"LAT   ");
        p2 = strstr(csd2.el,"LAT   ");
        create_LAT = (p1 == NULL ? (p2 == NULL ? 0 : 1) : 0);
        p1 = strstr(csd1.el,"LON   ");
        p2 = strstr(csd2.el,"LON   ");
        create_LON = (p1 == NULL ? (p2 == NULL ? 0 : 1) : 0);
        p1 = strstr(csd1.el,"HIGH  ");
        p2 = strstr(csd2.el,"HIGH  ");
        create_HIGH = (p1 == NULL ? (p2 == NULL ? 0 : 1) : 0);
        if (debug) printf("create_LAT,LON,HIGH = %d, %d, %d \n", create_LAT, create_LON, create_HIGH);
    } else {
        create_LAT  = 0;
        create_LON  = 0;
        create_HIGH = 0;
    }
#endif

    sz = csd1.size[0] * csd1.size[1] + 20;
    if ((dat = malloc(sz * 8)) == NULL) {
        printf("!E  dat = malloc(%d) \n", sz * 8);
        exit(1);
    }

    for (b=0; b<csd1.nb; b++) {
        int     bt = csd1.bt[b];                                                /* nsd1: basetime */

        if ((selbt2 != 0) && (selbt1 > bt)) continue;                           /* select basetime */
        if ((selbt2 != 0) && (selbt2 < bt)) continue;                           /* select basetime */

        for (m=0; m<csd1.nm; m++) {
            char    mmb[5];

            strncpy(mmb, csd1.mb + m * 4, 4); mmb[4] = '\0';

            /* if ((selmm[0] != '\0') && (strncmp(mmb,selmm,strlen(selmm)) != 0)) continue; * select */
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

            for (i=0; i<csd2.nm; i++) if(strncmp(csd2.mb + i*4, mmb, 4) == 0) break;
            if (i >= csd2.nm) continue;

            for (k=0; k<csd1.nf; k++) {
                int     vt = time_add(bt, csd1.ft[k], csd2.fu);                 /* nsd1: validtime */
                int     n_mb, n_vt, n_pl, n_el, n_mp;
                N_SI4         *tvt = NULL;
                char    *tmb = NULL,       *pln = NULL, *elm = NULL, *map = NULL;
                int     im, iv;

                if ((selvt2 != 0) && (selvt1 > vt)) continue;                   /* select validtime */
                if ((selvt2 != 0) && (selvt2 < vt)) continue;

                rt = nusdas_parameter_change(N_PC_ID_SET, &csd1.nsd);
                if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csd1.nsd);
                t1 = csd1.types;
                t2 = csd1.types + 8;
                t3 = csd1.types + 12;

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

                if (tmb == NULL) { printf("!E  tmb = calloc(%d)\n", n_mb * 4 + 1); exit(1); }
                if (tvt == NULL) { printf("!E  tvt = malloc(%d)\n", n_vt * 4); exit(1); }
                if (pln == NULL) { printf("!E  pln = calloc(%d)\n", n_pl * 6 + 1); exit(1); }
                if (elm == NULL) { printf("!E  elm = calloc(%d)\n", n_el * 6 + 1); exit(1); }

                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_MEMBER_LIST,tmb,&n_mb) != n_mb) goto cont;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_VALIDTIME_LIST,tvt,&n_vt) != n_vt) goto cont;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_PLANE_LIST,pln,&n_pl) != n_pl) goto cont;
                if (nusdas_inq_cntl(t1,t2,t3,&bt,mmb,&vt,N_ELEMENT_LIST,elm,&n_el) != n_el) goto cont;


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

                proc_copy(csd1, csd2, bt, mmb, vt, n_pl, pln, n_el, elm, map + i, dat, sz, &ncp);
                copy += ncp;
                if (ncp > 0) proc_info(csd1, csd2, bt, mmb, vt);

                if (map != NULL) free(map);
            cont:
                if (tmb != NULL) free(tmb);
                if (tvt != NULL) free(tvt);
                if (pln != NULL) free(pln);
                if (elm != NULL) free(elm);
            }
        }
    }

    if (dat != NULL) free(dat);

    *copies = copy;
    return  0;
}


static int
rd_nusdef_type(int nsd, char ***types)
{
    int     n;
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
    int     i, j, rt, total, *copies;

    int     narg = 0;
    int     ntype, nsd1, nsd2;
    char  **types;

    myname = argv[0];
    nsd1 = -1;
    nsd2 = -1;
    while (++narg < argc) {    /* 引数の解析 */
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
                } else if (strncmp(p1,"-qtime",6) == 0) { qtime = 1; break; }
                else if (strncmp(p1,"-help",5) == 0) usage(0,0,NULL);
                else if (strncmp(p1,"-mask",5) == 0) { mask = 1; break; }
                else if (strncmp(p1,"-n_nc",5) == 0) { n_nc = 1; break; }
                else if (strncmp(p1,"-over",5) == 0) { over = 1; break; }
                else if (strncmp(p1,"-no_over",8) == 0) { over = 0; break; }
                else if (strncmp(p1,"-nusbuf" ,7) == 0) { noclose = 0; break; }
                else if (strncmp(p1,"-noclose",8) == 0) { noclose = 1; break; }
                else if (strncmp(p1,"-no-subc",8) == 0) { no_subc = 1; break; }
                else if (strncmp(p1,"-rege"   ,5) == 0) { regexp  = 1; break; }
                else if (*p1 == 'D') debug  = 1;
                else if (*p1 == '0') debug  = 0;
                else if (*p1 == 'W') warnend  = 1;
                else if (*p1 == 'h') usage(0,0,NULL);
                else if (*p1 == '?') usage(0,0,NULL);
                else if (*p1 == 'd') {
                    subc = max(1,atoi(p1+1));
                    verbs = subc > 1 ? 1 : 0;
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
                } else if (*p1 == 'z') {  /* beta,alpha */
                    int     a, b;
                    if (*(++p1) == '\0') p1 = usage(++narg, argc, argv);
                    i = sscanf(p1,"%d,%d",&b, &a);
                    if (i-- > 0) beta = b;
                    if (i-- > 0) alpha = a;
                    break;
                } else {
                    printf("!E  unknown option (%s)\n", p);
                    usage(0,0,argv);
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

    if (nsd2 <= 0) usage(0,0,argv);

    if (debug) {
        printf ("debug  = %d \n", debug);
        printf ("warnend= %d \n", warnend);
        printf ("nsd    = %d --> %d \n", nsd1, nsd2);
        printf ("overw  = %d \n", over);
        printf ("noclose= %d \n", noclose);
        printf ("no-subc= %d \n", no_subc);
        printf ("subc   = %d \n", subc);
        printf ("select type (%d)\n", seltpnum);
        for (i=0; i<seltpnum; i++) printf ("            %-16s : %s\n", seltpsrc[i], (seltpdst[i] == NULL) ? "" : seltpdst[i]);
        printf ("select basetime  = %d, %d \n", selbt1, selbt2);
        printf ("select validtime = %d, %d \n", selvt1, selvt2);
        printf ("select member     (%s)    \n", selmm);
        printf ("select plane      (%s)    \n", selpl);
        printf ("select element    (%s)    \n", selel);
        printf ("basetime B,A     = %d, %d \n", beta, alpha);
    }

    nusdas_iocntl(N_IO_WARNING_OUT,debug);

    if (noclose > 0) {
        nusdas_iocntl(N_IO_W_FCLOSE,N_OFF);
        nusdas_iocntl(N_IO_R_FCLOSE,N_OFF);
    }

    rt = nusdas_parameter_change(N_PC_ID_SET, &nsd1);
    if (debug) printf("nusdas_parameter_change(N_PC_ID_SET,%d)=%d\n",nsd1,rt);

    ntype = rd_nusdef_type(nsd1,&types);

    copies = calloc(ntype, sizeof(int));
    if (copies == NULL) { printf("!E  copies = calloc(%d)\n", ntype); exit(1); }
    for (i=0; i<ntype; i++) {
        char    dstyp[17];
        int     ncp, len = 0;

        strncpy(dstyp, types[i], 16);
        dstyp[16] = '\0';

        if (seltpnum > 0) {
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

        if (len > 0) {
            printf("  %s : copy to %s\n", types[i], dstyp);
        } else {
            printf("  %s : copy\n", types[i]);
        }

        rt = proc_type(nsd1, types[i], nsd2, dstyp, &ncp);
        copies[i] = ncp;
        if (rt >= 0) {
        } else {
            printf("!W  record = %d: %s \n", copies[i], types[i]);
            check_warning();
        }
        printf("\n");
    }

    nusdas_allfile_close(N_FOPEN_ALL);

    total = 0;
    for (i=0; i<ntype; i++) {
        char    dstyp[17];
        int     len = 0;

        strncpy(dstyp, types[i], 16);
        dstyp[16] = '\0';

        if (seltpnum > 0) {
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

        printf("    %s: copy records = %d \n", dstyp, copies[i]);
        total += copies[i];
    }

    printf("    Copy records, Total = %d \n\n", total);
    if (exitstatus == 0) {
        if (0 == total) {
            printf("!W  No record copied... \n");
            check_warning();
        }
        return 0;
    }

    printf("!Error code = %d \n", exitstatus);
    return  99;
}
