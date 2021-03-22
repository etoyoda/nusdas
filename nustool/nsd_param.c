#include   <stdio.h>
#include   <stdlib.h>
#include   <string.h>

#include   "nwpl_capi.h"
#include   "nusdas.h"
#include   "nsd_param.h"

extern int debug;

#ifdef NNNNNNNN
static void str_free(char **p) { while(*p) free(*p++); }
static char **str_split(char *str, int len, int num)
{
    int     i;
    char  **p = (char **)malloc(sizeof(char *) * (num+1));
    for (i=0; i<num; i++) {
        char    wk[100];
        memcpy(wk, str+i*len, len);
        wk[len] = '\0';
        p[i] = strdup(wk);
    }
    p[num] = NULL;
    return  p;
}
#endif

void
print_map(int m, int k, int nsd, int n_mb, int n_vt, int n_pl, int n_el, int flag_size, char *tb_mb, int *tb_vt, char *tb_pl, char *tb_el, char *flag)
{
    int     i, im, iv, ip, ie, skip = 0;
    char  **pary, mmb[5];
    strncpy(mmb, tb_mb + m * 4, 4); mmb[4] = '\0';

    printf("------ flag array list ------\n");
    printf("NUSDAS%02d:n_mb,n_vt,n_pl,n_el=%d,%d,%d,%d, flag_size=%d ", nsd, n_mb, n_vt, n_pl, n_el, flag_size);
    i = 0;
    for(im=0; im<n_mb; im++) {
        for(iv=0; iv<n_vt; iv++) {
            for(ip=0; ip<n_pl; ip++) {
                if (skip == 1) {
                    for(ie=0; ie<n_el; ie++) if(flag[i+ie] != 0) skip = 0;
                }
                if (skip == 1) {
                    i += n_el;
                } else {
                    skip = 1;
                    printf("\n  INDX[%2d,%2d,%2d]", im, iv, ip);
                    for(ie=0; ie<n_el; ie++) {
                        if (flag[i] != 0) skip = 0;
                        printf(" %d", flag[i]);
                        if (++i >= flag_size) break;
                    }
                }
                if (i >= flag_size) break;
            }
            if (i >= flag_size) break;
        }
        if (i >= flag_size) break;
    }
    printf("\n      flag [n_mb=%d, n_vt=%d, n_pl=%d]\n", n_mb, n_vt, n_pl);

    printf("N_MEMBER  LIST(%s)\n",tb_mb);
    printf("N_PLANE   LIST(%s)\n",tb_pl);
    printf("N_ELEMENT LIST(%s)\n",tb_el);
    for (i=0; i<n_vt; i+=1) {
        printf("%4d: %d\n", i, tb_vt[i]);
    }
    printf("(%s) m = %d / %d - (%s)\n", m < n_mb ? "OK" : "NG", m, n_mb, mmb);       /* csd に mmb の有無 */
    printf("(%s) k = %d / %d ... %d\n", k < n_vt ? "OK" : "NG", k, n_vt, tb_vt[k]);  /* csd に vt  の有無 */
    printf("\n");
}


int
nsd_param(char *type, struct nsd_parm *prm, int selbt1, int selbt2)
{
    char   *t1 = type;
    char   *t2 = type + 8;
    char   *t3 = type + 12;
    N_SI4   i, n, rt, *wk;
    char   *wc, unit[8];
    void   *p;

    /*------------------------------------*
     *  basetime[], member[], kt[]
     *------------------------------------*/

    i = 1;
    if ((selbt1 != 0) && ( selbt1 == selbt2)){
        rt=1;
    }else{
        rt = nusdas_inq_nrdbtime(t1, t2, t3, &n, &i, debug);
    }
    if (debug) printf("nusdas_inq_nrdbtime() rt = %d \n", rt);
    if (rt < 0) return -987;

    if (rt > 0) {
        wk = malloc(rt * sizeof(N_SI4));
        if ((selbt1 != 0) && (selbt1 == selbt2)){
            *wk = selbt1 ;
            n = 1;
        }else{
            n = nusdas_inq_nrdbtime(t1, t2, t3, wk, &rt, debug);
            if (debug) printf("nusdas_inq_nrdbtime() n,rt = %d, %d \n", n, rt);
        }
        prm->bt = wk;
        prm->nb = n;
    } else {
        prm->nb = 0;
    }

    i = 1;
    rt = nusdas_inq_def(t1,t2,t3, N_MEMBER_NUM, &n, &i);
    if (rt != 1) return -988;
    p = calloc(n * 4 + 1, 1);
    rt = nusdas_inq_def(t1,t2,t3, N_MEMBER_LIST, p, &n);
    if (rt != n) return -989;
    prm->nm = n;
    prm->mb = p;

    i = 1;
    rt = nusdas_inq_def(t1,t2,t3, N_VALIDTIME_NUM, &n, &i);
    if (rt != 1) return -991;
    p = malloc(n * sizeof(N_SI4));
    rt = nusdas_inq_def(t1,t2,t3, N_VALIDTIME_LIST, p, &n);
    if (rt != n) return -992;
    prm->nf = n;
    prm->ft = p;

    i = 1;
    rt = nusdas_inq_def(t1,t2,t3, N_VALIDTIME_UNIT, unit, &i);
    if (rt != 1) return -993;
    unit[4] = '\0';
    prm->fu = str2sym4(unit);
    /* unitが異常ならtime_add(0,0,unit)が-1になることを利用したエラーチェック */
    if (0 > time_add(0, 0, prm->fu)) {
        fprintf(stderr, "!E unknown  N_VALIDTIME_UNIT (%s)\n", unit);
        return -993;
    }


    /*-------------------------------------------------------*
     *   plane[], element[], elementmap[], grid_size[]
     *-------------------------------------------------------*/
    i = 1;
    rt = nusdas_inq_def(t1,t2,t3, N_PLANE_NUM, &n, &i);
    if (rt != 1) return -994;
    p = calloc(n * 6 + 1, 1);
    rt = nusdas_inq_def(t1,t2,t3, N_PLANE_LIST, p, &n);
    if (rt != n) return -995;
    prm->np = n;
    prm->pl = p;
    if ((i = strlen(p) / 6) != n) {
        printf("!W  nusdas.def で %s=%d と %s (%d?) があっていないかも\n",
               "plane", n, "plane1", i);
    }

    i = 1;
    rt = nusdas_inq_def(t1,t2,t3, N_ELEMENT_NUM, &n, &i);
    if (rt != 1) return -996;
    p = calloc(n * 6 + 1, 1);
    rt = nusdas_inq_def(t1,t2,t3, N_ELEMENT_LIST, p, &n);
    if (rt != n) return -997;
    prm->ne = n;
    prm->el = p;
    if ((i = strlen(p) / 6) != n) {
        printf("!W  nusdas.def で %s=%d と %s (%d?) があっていないかも\n",
               "element", n, "elementmap", i);
    }

    i = prm->nm * prm->nf * prm->np * prm->ne;
    wc = malloc(i);
    n = nusdas_inq_def(t1,t2,t3, N_ELEMENT_MAP, wc, &i);
    if (n < 1) return -998;
    prm->nw = n;
    prm->ew = wc;
    prm->er = malloc(n);
    wc = NULL;

    i = 2;
    rt = nusdas_inq_def(t1,t2,t3, N_GRID_SIZE, prm->size, &i);
    if (rt != 2) return -999;

    if (debug == 2) {
        print_map(0, 0, -1, prm->nb, prm->nf, prm->np, prm->ne, prm->nw, prm->mb, prm->ft, prm->pl, prm->el, prm->ew);
    } else if (debug) {
        printf("\n");
        printf("********** nusdas_def parameter list **********\n");
        printf("basetime num = %d \n", prm->nb);
        for (i=0; i<prm->nb; i++) printf(" %d", prm->bt[i]);
        printf("\nforecast time num = %d \n", prm->nf);
        for (i=0; i<prm->nf; i++) printf(" %d", prm->ft[i]);
        printf("\nmember num = %d \n", prm->nm);
        prm->mb[prm->nm * 4] = '\0';
        printf("%s;\n", prm->mb);
        printf("plane num = %d \n", prm->np);
        prm->pl[prm->np * 6] = '\0';
        printf("%s;\n", prm->pl);
        printf("element num = %d \n", prm->ne);
        prm->el[prm->ne * 6] = '\0';
        printf("%s;\n", prm->el);
        printf("element map num = %d \n", prm->nw);
        if (debug > 2) {
            for (i=0; i<prm->nw; i++) printf("%d", prm->ew[i]);
        }
        printf("\ngrid size = (%d, %d) \n", prm->size[0], prm->size[1]);
        printf("***********************************************\n");
    }

    prm->types = type;

    return  0;
}
