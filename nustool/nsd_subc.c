#include   <stdio.h>
#include   <stdlib.h>
#include   <math.h>
#include   <string.h>
#include   "nwpl_capi.h"
#include   "nusdas.h"
#include   "nsd_subc.h"

#define    max(a,b)    ((a)>(b)?(a):(b))
#define    min(a,b)    ((a)<(b)?(a):(b))

extern int  debug;
extern int  verbs;
extern int  subc;
extern int  ispec;



static int proc_subc_rgau(struct nsd_subc_s *csdx, struct nsd_subc_s *csdy, char *mmb, int vt, char *pln, char *elm, char *group, char *mode)
{
    char   *t1, *t2, *t3;
    int     i, bt, rt, jn;
    int     j1, js1, jn1, j2, js2, jn2;
    int    *bi1 = NULL, *bis1, *bin1;
    int    *bi2 = NULL, *bis2, *bin2;
    float  *lat1 = NULL, *lat2;

    rt = nusdas_parameter_change(N_PC_ID_SET, &csdx->nsd);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdx->nsd);
    t1 = csdx->types;
    t2 = csdx->types + 8;
    t3 = csdx->types + 12;
    bt = csdx->bt;

    if ((rt = nusdas_subc_rgau_inq_jn(t1,t2,t3,&bt,mmb,&vt,&jn)) > 0) {
        bi1  = malloc(3 * jn * sizeof(int));   bis1 = bi1 + jn;  bin1 = bis1 + jn;
        bi2  = malloc(3 * jn * sizeof(int));   bis2 = bi2 + jn;  bin2 = bis2 + jn;
        lat1 = malloc(2 * jn * sizeof(float)); lat2 = lat1 + jn;
        if (bi1 == NULL) { printf("!E  bi1 = malloc(%d)\n", 3 * jn * sizeof(int)); exit(1); }
        if (bi2 == NULL) { printf("!E  bi2 = malloc(%d)\n", 3 * jn * sizeof(int)); exit(1); }
        if (lat1 == NULL) { printf("!E  lat1 = malloc(%d)\n", 2 * jn * sizeof(float)); exit(1); }
    } else {
        printf("!E rt=%d nusdas_subc_rgau_inq_jn(%s,%d,%s,%d) j_n=%d\n", rt,t1,bt,mmb,vt,jn);
        return  -1;
    }

    jn1 = jn;
    rt = nusdas_subc_rgau(t1,t2,t3,&bt,mmb,&vt,&j1,&js1,&jn1,bi1,bis1,bin1,lat1,"GET");
    if (debug) printf("   rt=%d nusdas_subc_rgau(%s,%d,%s,%d,%s,j=%d,j_n=%d)\n", rt,t1,bt,mmb,vt,group,j1,jn1);
    if (rt == 0) {
        if (csdy == NULL) {   /* nsd_list */
            printf("  SUBC %s(j,js,jn=%d,%d,%d)\n", group, j1, js1, jn1);
            for (i=0; i<jn; i++) printf("         %d: %d, %d, %d, %f\n", i+1, bi1[i], bis1[i], bin1[i], lat1[i]);
        } else {
            int     bt2 = csdy->bt;
            rt = nusdas_parameter_change(N_PC_ID_SET, &csdy->nsd);
            if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdy->nsd);
            t1 = csdy->types;
            t2 = csdy->types + 8;
            t3 = csdy->types + 12;
            if (*mode == 'P') {                /* nsd_copy */
                rt = nusdas_subc_rgau(t1,t2,t3,&bt,mmb,&vt,&j1,&js1,&jn1,bi1,bis1,bin1,lat1,"PUT");
                if (rt != 0) printf("!E rt=%d nusdas_subc_rgau(%s,%d,%s,%d,%s,PUT)\n", rt, t1, bt2, mmb, vt, group);
            } else {                /* nsd_xcmp */
                jn2 = jn;
                rt = nusdas_subc_rgau(t1,t2,t3,&bt2,mmb,&vt,&j2,&js2,&jn2,bi2,bis2,bin2,lat2,"GET");
                if (rt != 0) printf("!E rt=%d nusdas_subc_rgau(%s,%d,%s,%d,j2=%d,j_n=%d)\n", rt, t1, bt2, mmb, vt, j2, jn2);
                else if (debug) printf("   rt=%d nusdas_subc_rgau(%s,%d,%s,%d,j2=%d,j_n=%d)\n", rt, t1, bt2, mmb, vt, j2, jn2);
                if ((j1 != j2) || (js1 != js2) || (jn1 != jn2) ||
                    (memcmp(bi1,bi2,3*jn*sizeof(int)) != 0) || (memcmp(lat1,lat2,jn*sizeof(float)) != 0)) {
                    printf("DIFF SUBC %s \n", group);
                    if (verbs) {
                        printf("  SUBC %s \n", group);
                        printf("  NUSDAS%02d < %d, %d, %d \n", csdx->nsd, j1, js1, jn1);
                        printf("  NUSDAS%02d > %d, %d, %d \n", csdy->nsd, j2, js2, jn2);
                        for (i=0; i<jn; i++) {
                            printf("         %d < %d, %d, %d, %f\n", i+1, bi1[i], bis1[i], bin1[i], lat1[i]);
                            printf("         %d > %d, %d, %d, %f\n", i+1, bi2[i], bis2[i], bin2[i], lat2[i]);
                        }
                    }
                } else if (verbs) printf("   EQ SUBC %s(j,js,jn=%d,%d,%d)\n", group, j1, js1, jn1);
            }
        }
    } else {
        printf("!E  rt=%d nusdas_subc_rgau(%s,%d,%s,%d,%s,j=%d,j_n=%d)\n", rt,t1,bt,mmb,vt,group,j1,jn1);
    }

    if (bi1 != NULL) free(bi1);
    if (bi2 != NULL) free(bi2);
    if (lat1 != NULL) free(lat1);
    return  rt;
}

static int proc_subc_delt(struct nsd_subc_s *csdx, struct nsd_subc_s *csdy, char *mmb, int vt, char *pln, char *elm, char *group, char *mode)
{
    char   *t1, *t2, *t3;
    float   bf[1], bf2[1];
    int     i, bt, rt;

    rt = nusdas_parameter_change(N_PC_ID_SET, &csdx->nsd);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdx->nsd);
    t1 = csdx->types;
    t2 = csdx->types + 8;
    t3 = csdx->types + 12;
    bt = csdx->bt;

    rt = nusdas_subc_delt(t1,t2,t3,&bt,mmb,&vt,bf,"GET");
    if (debug) printf("   rt=%d nusdas_subc_delt(%s,%d,%s,%d,%s,%s,%s)\n", rt,t1,bt,mmb,vt,pln,elm,group);
    if (rt == 0) {
        if (csdy == NULL) {   /* nsd_list */
            printf("  SUBC %s(%f)\n", group, bf[0]);
        } else {                /* nsd_copy */
            int     bt2;
            float  *p = (*mode == 'G') ? bf2 : bf;

            rt = nusdas_parameter_change(N_PC_ID_SET, &csdy->nsd);
            if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdy->nsd);
            t1 = csdy->types;
            t2 = csdy->types + 8;
            t3 = csdy->types + 12;
            bt2 = csdy->bt;
            rt = nusdas_subc_delt(t1,t2,t3,&bt2,mmb,&vt,p,mode);
            if (rt != 0) printf("!E rt=%d nusdas_subc_delt(%s,%d,%s,%d,%s,%s,%s,PUT)\n", rt, t1, bt2, mmb, vt, pln, elm, group);
            else if (debug) printf("   rt=%d nusdas_subc_delt(%s,%d,%s,%d,%s,%s,%s,PUT)\n", rt, t1, bt2, mmb, vt, pln, elm, group);
            if (*mode == 'G') {
                if (bf[0] != bf2[0]) printf("DIFF SUBC %s \n", group);
                if (verbs) {
                    if (bf[0] != bf2[0]) {
                        printf("  NUSDAS%02d(SUBC): %d \n", csdx->nsd, bf[0]);
                        printf("  NUSDAS%02d(SUBC): %d \n", csdy->nsd, bf2[0]);
                    } else
                        printf("  EQ SUBC %s: %d \n", group, bf[0]);
                }
            }
        }
    } else {
        printf("!W  error nusdas_subc_delt(%s,%d,%s,%s,%s,GET) = %d \n", t1, vt, pln, elm, group, rt);
    }
    return  rt;
}

static int proc_subc_tdif(struct nsd_subc_s *csdx, struct nsd_subc_s *csdy, char *mmb, int vt, char *pln, char *elm, char *group, char *mode)
{
    char   *t1, *t2, *t3;
    N_SI4   bf[2], bf2[2];
    int     i, bt, rt;

    rt = nusdas_parameter_change(N_PC_ID_SET, &csdx->nsd);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdx->nsd);
    t1 = csdx->types;
    t2 = csdx->types + 8;
    t3 = csdx->types + 12;
    bt = csdx->bt;

    rt = nusdas_subc_tdif(t1,t2,t3,&bt,mmb,&vt,bf,bf+1,"GET");
    if (debug) printf("   rt=%d nusdas_subc_tdif(%s,%d,%s,%d,%s,%s,%s)\n", rt,t1,bt,mmb,vt,pln,elm,group);
    if (rt == 0) {
        if (csdy == NULL) {   /* nsd_list */
            printf("  SUBC %s(%d, %d)\n", group, bf[0], bf[1]);
        } else {                /* nsd_copy */
            int     bt2;
            N_SI4  *p = (*mode == 'G') ? bf2 : bf;

            rt = nusdas_parameter_change(N_PC_ID_SET, &csdy->nsd);
            if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdy->nsd);
            t1 = csdy->types;  /* bug-fix : 2009-6-17 */
            t2 = csdy->types + 8;
            t3 = csdy->types + 12;
            bt2 = csdy->bt;
            rt = nusdas_subc_tdif(t1,t2,t3,&bt2,mmb,&vt,p,p+1,mode);
            if (rt != 0) printf("!E rt=%d nusdas_subc_tdif(%s,%d,%s,%d,%s,%s,%s,PUT)\n", rt, t1, bt2, mmb, vt, pln, elm, group);
            else if (debug) printf("   rt=%d nusdas_subc_tdif(%s,%d,%s,%d,%s,%s,%s,PUT)\n", rt, t1, bt2, mmb, vt, pln, elm, group);
            if (*mode == 'G') {
                if ((i = memcmp(bf,bf2,2 * sizeof(N_SI4))) != 0) printf("DIFF SUBC %s \n", group);
                if (verbs) {
                    if (i != 0) {
                        printf("  NUSDAS%02d(SUBC): %d, %d \n", csdx->nsd, bf[0], bf[1]);
                        printf("  NUSDAS%02d(SUBC): %d, %d \n", csdy->nsd, bf2[0], bf2[1]);
                    } else
                        printf("  EQ SUBC %s: %d, %d \n", group, bf[0], bf[1]);
                }
            }
        }
    } else {
        printf("!W  error nusdas_subc_tdif(%s,%d,%s,%s,%s,GET) = %d \n", t1, vt, pln, elm, group, rt);
    }
    return  rt;
}

static int proc_subc_eta(struct nsd_subc_s *csdx, struct nsd_subc_s *csdy, char *mmb, int vt, char *pln, char *elm, char *group, char *mode)
{
    char   *t1, *t2, *t3;
    float  *a1 = NULL, *b1 = NULL, c1;
    float  *a2, *b2, c2;
    int     bt, nlev, rt, sz;

    rt = nusdas_parameter_change(N_PC_ID_SET, &csdx->nsd);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdx->nsd);
    t1 = csdx->types;
    t2 = csdx->types + 8;
    t3 = csdx->types + 12;
    bt = csdx->bt;
            
    rt = nusdas_subc_eta_inq_nz(t1,t2,t3,&bt,mmb,&vt,group,&nlev);
    if (rt <= 0) {
        printf("%d NUSDAS%02d:SUBC nusdas_subc_eta_inq_nz(%-16s,%-4s,%-4s) \n", rt, csdx->nsd, t1, mmb, group);
        return  0;
    }

    sz = (nlev + 1) * sizeof(float);
    a1 = (float *)malloc(2 * sz);
    b1 = (float *)malloc(2 * sz);
    if (a1 == NULL) { printf("!E  a1 = malloc(%d)\n", 2 * sz); exit(1); }
    if (b1 == NULL) { printf("!E  b1 = malloc(%d)\n", 2 * sz); exit(1); }
    rt = nusdas_subc_eta(t1,t2,t3,&bt,mmb,&vt,&nlev,a1,b1,&c1,"GET");

    if (debug) printf("   rt=%d NUSDAS%02d:nusdas_subc_eta(%s,%d,%s,%d,%s,%s,%s)\n", rt,csdx->nsd,t1,bt,mmb,vt,pln,elm,group);
    if (rt == 0) {
        int     i, bt2;
        if (csdy == NULL) {   /* nsd_list */
            printf("  SUBC %4s(%f, %f, %f) n_level=%d\n", group, a1[0], b1[0], c1, nlev);
            if (subc > 1) for (i=1; i<=nlev; i++) printf("       %4s %f, %f \n", group, a1[i], b1[i]);
        } else {                /* nsd_copy */
            i = nusdas_parameter_change(N_PC_ID_SET, &csdy->nsd);
            if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", i, csdy->nsd);
            t1 = csdy->types;
            t2 = csdy->types + 8;
            t3 = csdy->types + 12;
            bt2 = csdy->bt;
            if (*mode == 'G') {
                a2 = a1 + nlev + 1;
                b2 = b1 + nlev + 1;
                if ((rt = nusdas_subc_eta(t1,t2,t3,&bt2,mmb,&vt,&nlev,a2,b2,&c2,"GET")) == 0) {
                    if ((memcmp(a1,a2,sz) != 0) ||
                        (memcmp(b1,b2,sz) != 0) ||
                        (c1 != c2)                 ) printf("DIFF SUBC %s\n", group);
                    if (verbs) {
                        printf("  NUSDAS%02d,%02d(SUBC) %s: c=%f, %f \n", group, csdx->nsd, csdy->nsd, c1, c2);
                        for (i=0; i<=nlev; i++) printf("  %d: a=%f,%f  b=%f,%f \n", i, a1[i], a2[i], b1[i], b2[i]);
                    }
                }
            } else {
                rt = nusdas_subc_eta(t1,t2,t3,&bt2,mmb,&vt,&nlev,a1,b1,&c1,"PUT");
            }
            if (rt != 0) printf("!E rt=%d NUSDAS%02d:nusdas_subc_eta(%s,%d,%s,%d,%s,%s,%s,%s)\n", rt, csdy->nsd, t1, bt2, mmb, vt, pln, elm, group, mode);
            else if (debug) printf("   rt=%d NUSDAS%02d:nusdas_subc_eta(%s,%d,%s,%d,%s,%s,%s,%s)\n", rt, csdy->nsd, t1, bt2, mmb, vt, pln, elm, group, mode);
        }
    } else {
        printf("!W  error nusdas_subc_eta(%s,%d,%s,%s,%s,GET) = %d \n", t1, vt, pln, elm, group, rt);
    }
    if (b1 != NULL) free(b1);
    if (a1 != NULL) free(a1);
    return  rt;
}

static int proc_subc_zhyb(struct nsd_subc_s *csdx, struct nsd_subc_s *csdy, char *mmb, int vt, char *pln, char *elm, char *group, char *mode)
{
    char   *t1, *t2, *t3;
    int nz1, nz2;
    float ptrf1, presrf1, *zrp1 = NULL, *zrw1, *vctrans_p1, *vctrans_w1, *dvtrans_p1, *dvtrans_w1; 
    float ptrf2, presrf2, *zrp2 = NULL, *zrw2, *vctrans_p2, *vctrans_w2, *dvtrans_p2, *dvtrans_w2; 
    int     bt, nlev, rt, sz;

    rt = nusdas_parameter_change(N_PC_ID_SET, &csdx->nsd);
    if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdx->nsd);
    t1 = csdx->types;
    t2 = csdx->types + 8;
    t3 = csdx->types + 12;
    bt = csdx->bt;

    rt = nusdas_subc_eta_inq_nz(t1,t2,t3,&bt,mmb,&vt,group,&nlev);
    if (rt <= 0) {
    printf("%d NUSDAS%02d:SUBC nusdas_subc_eta_inq_nz(%-16s,%-4s,%-4s) \n", rt, csdx->nsd, csdx->types, mmb, group);
        return  0;
    }

    sz = 6 * nlev * sizeof(float);
    zrp1 = (float *)malloc(sz);
    if (zrp1 == NULL) { printf("!E  zrp1 = malloc(%d)\n", sz); exit(1); }
    zrw1       = zrp1 + nlev;
    vctrans_p1 = zrp1 + nlev * 2;
    vctrans_w1 = zrp1 + nlev * 3;
    dvtrans_p1 = zrp1 + nlev * 4;
    dvtrans_w1 = zrp1 + nlev * 5; 

    rt = nusdas_subc_zhyb(t1,t2,t3,&bt,mmb,&vt,&nlev,&ptrf1,&presrf1,zrp1,zrw1,vctrans_p1,vctrans_w1,dvtrans_p1,dvtrans_w1,"GET");

    if (debug) printf("   rt=%d NUSDAS%02d:nusdas_subc_zhyb(%s,%d,%s,%d,%s,%s,%s)\n", rt,csdx->nsd,csdx->types,bt,mmb,vt,pln,elm,group);
    if (rt == 0) {
        int     i, bt2;
        if (csdy == NULL) {   /* nsd_list */
            printf("  SUBC %4s(%f, %f) n_level=%d\n", group, ptrf1, presrf1, nlev);
            if (subc > 1) {
                for (i=0; i<=nlev; i++) {
                  printf("       %4s %f, %f, %f, %f, %f, %f\n", group, zrp1[i], zrw1[i], vctrans_p1[i], vctrans_w1[i], dvtrans_p1[i], dvtrans_w1[i]);
                }
            }
        } else {                /* nsd_copy */
            i = nusdas_parameter_change(N_PC_ID_SET, &csdy->nsd);
            if (debug) printf("rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", i, csdy->nsd);
            t1 = csdy->types;
            t2 = csdy->types + 8;
            t3 = csdy->types + 12;
            bt2 = csdy->bt;
            if (*mode == 'G') {
                zrp2 = (float *)malloc(sz);
                if (zrp2 == NULL) { printf("!E  zrp2 = malloc(%d)\n", sz); exit(1); }
                zrw2       = zrp2 + nlev;
                vctrans_p2 = zrp2 + nlev * 2;
                vctrans_w2 = zrp2 + nlev * 3;
                dvtrans_p2 = zrp2 + nlev * 4;
                dvtrans_w2 = zrp2 + nlev * 5;
                if ((rt = nusdas_subc_zhyb(t1,t2,t3,&bt,mmb,&vt,&nlev,&ptrf2,&presrf2,zrp2,zrw2,vctrans_p2,vctrans_w2,dvtrans_p2,dvtrans_w2,"GET")) == 0) {
                    if ((ptrf1 != ptrf2) || (presrf1 != presrf2) || (memcmp(zrp1,zrp2,sz) != 0))
                        printf("DIFF SUBC %s\n", group);
                    if (verbs) {
                        printf("  NUSDAS%02d,%02d(SUBC) %s: ptrf=%f, %f, presrf=%f, %f \n", csdx->nsd, csdy->nsd, group, ptrf1, ptrf2, presrf1, presrf2);
                        for (i=0; i<nlev; i++) {
                            printf("  %d: zrp=%f,%f zrw=%f,%f vctrans_p=%f,%f vctrans_w=%f,%f dvtrans_p=%f,%f dvtrans_w=%f,%f\n",
                                i, zrp1[i], zrp2[i], zrw1[i], zrw2[i], vctrans_p1[i], vctrans_p2[i], vctrans_w1[i], vctrans_w2[i],
                                dvtrans_p1[i], dvtrans_p2[i], dvtrans_w1[i], dvtrans_w2[i]);
                        }
                    }
                }
            } else {
                rt = nusdas_subc_zhyb(t1,t2,t3,&bt2,mmb,&vt,&nlev,&ptrf1,&presrf1,zrp1,zrw1,vctrans_p1,vctrans_w1,dvtrans_p1,dvtrans_w1,"PUT");
            }
            if (rt != 0) printf("!E rt=%d NUSDAS%02d:nusdas_subc_zhyb(%s,%d,%s,%d,%s,%s,%s,%s)\n", rt, csdy->nsd, csdy->types, bt2, mmb, vt, pln, elm, group, mode);
            else if (debug) printf("   rt=%d NUSDAS%02d:nusdas_subc_zhyb(%s,%d,%s,%d,%s,%s,%s,%s)\n", rt, csdy->nsd, csdy->types, bt2, mmb, vt, pln, elm, group, mode);
        }
    } else {
        printf("!W  error nusdas_subc_zhyb(%s,%d,%s,%s,%s,GET) = %d \n", csdx->types, vt, pln, elm, group, rt);
    }
    if (zrp1 != NULL) free(zrp1);
    if (zrp2 != NULL) free(zrp2);
    return  rt;
}

static int proc_subc_srf(struct nsd_subc_s *csdx, struct nsd_subc_s *csdy, char *mmb, int vt, char *pln, char *elm, char *group, char *mode)
{
    char   *t1, *t2, *t3;
    N_SI4   bf[128], bf2[128], sz = 0;
    int     i, bt, rt;

    rt = nusdas_parameter_change(N_PC_ID_SET, &csdx->nsd);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdx->nsd);
    t1 = csdx->types;
    t2 = csdx->types + 8;
    t3 = csdx->types + 12;
    bt = csdx->bt;
    
    memset(bf,0,sizeof(bf));
    rt = nusdas_subc_srf(t1,t2,t3,&bt,mmb,&vt,pln,elm,group,bf,"GET");
    if (debug) printf("   rt=%d NUSDAS%02d:nusdas_subc_srf(%s,%d,%s,%d,%s,%s,%s)\n", rt,csdx->nsd,t1,bt,mmb,vt,pln,elm,group);
    if (rt == 0) {

        if      (strcmp("RADR",group) == 0) sz = 1;
        else if (strcmp("THUN",group) == 0) sz = 1;
        else if (strcmp("RADS",group) == 0) sz = 6;
        else if (strcmp("DPRD",group) == 0) sz = 8;
        else if (strcmp("ISPC",group) == 0) sz = 32;

        if (csdy == NULL) {   /* nsd_list */
            endian_swab4(bf,1);
        /*  endian_swab4(bf,128);
            endian_swab4(bf+1,1);
            endian_swab4(bf+4,1);
            endian_swab4(bf+5,1);  */
            if        (sz == 1) { printf("  SUBC %s(%d)\n", group, bf[0]);
            } else if (sz == 6) {
                printf("  SUBC %s(%08x, %08x, %08x, %08x, %08x, %08x)\n", group, bf[0], bf[1], bf[2], bf[3], bf[4], bf[5]);
            } else if (sz == 8) {
                printf("  SUBC %s(%08x, %d, %d, %d, %d, %d, %d, %08x)\n", group, bf[0], bf[1], bf[2], bf[3], bf[4], bf[5], bf[6], bf[7]);
            } else if (sz == 32) {
                char    str[100];
                strncpy(str, (char *)bf, 4); str[4] = '\0';
                printf("  SUBC %s:%s %08x %08x-%08x %08x,%08x %08x-%08x\n", group, str, bf[1], bf[2], bf[3], bf[4], bf[5], bf[6], bf[7]);
                if (subc > 1) {
                    printf("            %08x%08x %08x %d %d %08x%08x %d \n", bf[8], bf[9], bf[10], bf[11], bf[12], bf[13], bf[14], bf[15]);
                    printf("            %d %d %d %d %d %d %x %x \n", bf[16], bf[17], bf[18], bf[19], bf[20], bf[21], bf[22], bf[23]);
                    strncpy(str, (char *)(bf+24), 32); str[32] = '\0';
                    printf("            %s \n", str);
                }
                if (ispec == 1) {
                /*  endian_swab2(bf+32,(128-32)*2);  */
                    for (i=8; i<128; i+=8) {
                        printf("   %3d: %08x %08x %08x %08x %08x %08x %08x %08x \n", i*4,
                                            bf[i+0], bf[i+1], bf[i+2], bf[i+3], bf[i+4], bf[i+5], bf[i+6], bf[i+7]);
                    }
                }
            }
        } else {                /* nsd_copy,nsd_xcmp */
            int     bt2;
            N_SI4  *p = (*mode == 'G') ? bf2 : bf;

            memset(bf2,0,sizeof(bf2));
            rt = nusdas_parameter_change(N_PC_ID_SET, &csdy->nsd);
            if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdy->nsd);
            t1 = csdy->types;  /* bug-fix : 2009-6-17 */
            t2 = csdy->types + 8;
            t3 = csdy->types + 12;
            bt2  = csdy->bt;
            rt = nusdas_subc_srf(t1,t2,t3,&bt2,mmb,&vt,pln,elm,group,p,mode);
            if (rt != 0) printf("!E rt=%d NUSDAS%02d:nusdas_subc_srf(%s,%d,%s,%d,%s,%s,%s,%s)\n", rt, csdy->nsd, t1, bt2, mmb, vt, pln, elm, group, mode);
            else if (debug) printf("   rt=%d NUSDAS%02d:nusdas_subc_srf(%s,%d,%s,%d,%s,%s,%s,%s)\n", rt, csdy->nsd, t1, bt2, mmb, vt, pln, elm, group, mode);
            if (*mode == 'G') {
                int     nsz = (ispec == 1) ? 128 : sz;
                if (memcmp(bf,bf2,nsz * sizeof(int))) {
                /*  endian_swab4(bf,128);
                    endian_swab4(bf+1,1);
                    endian_swab4(bf+4,1);
                    endian_swab4(bf+5,1);
                    endian_swab2(bf+32,(128-32)*2);  */
                /*  endian_swab4(bf2,128);
                    endian_swab4(bf2+1,1);
                    endian_swab4(bf2+4,1);
                    endian_swab4(bf2+5,1);
                    endian_swab2(bf2+32,(128-32)*2);  */
                    printf("DIFF SUBC %s\n", group);
                    if (verbs) {
                        int     i, j;
                        printf("     SUBC %s\n", group);
                        for (j=0; j<nsz; j+=8) {
                            int     n = min(nsz - j,8);
                            printf("  NUSDAS%02d:", csdx->nsd); for (i=0; i<n; i++) printf(" %08x", bf[j+i]); printf("\n");
                            for (i=0; i<n; i++) if (bf[j+i] != bf2[j+i]) break;
                            if (i < n) {
                            printf("  NUSDAS%02d:", csdy->nsd); for (i=0; i<n; i++) printf(" %08x", bf2[j+i]); printf("\n");
                            printf("           ");
                            for (i=0; i<n; i++) printf(" %8s", bf[j+i] == bf2[j+i] ? "--------" : "^^^^^^^^");
                            printf("\n");
                            }
                        }
                    }
                    rt = 99;
                } else {
                /*  endian_swab4(bf2,128);
                    endian_swab4(bf2+1,1);
                    endian_swab4(bf2+4,1);
                    endian_swab4(bf2+5,1);  */
                    if (verbs) printf("  EQ SUBC %s(%08x,%08x,%08x,%08x,%08x,%08x) %d(byte)\n",
                                                 group, bf2[0], bf2[1], bf2[2], bf2[3], bf2[4], bf2[5], nsz*4);
                }
            }
        }
    } else {
        printf("!W  error nusdas_subc_srf(%s,%d,%s,%s,%s,GET) = %d \n", t1, vt, pln, elm, group, rt);
    }
    return  rt;
}

int
proc_subc(struct nsd_subc_s *csdx, struct nsd_subc_s *csdy, char *mmb, int vt, char *pln, char *elm, char *mode)
{
    int     i, n, bt, rt, exitstatus = 0;
    char   *t1, *t2, *t3;
    char    dnull[4];


    /* SUBC */
    rt = nusdas_parameter_change(N_PC_ID_SET, &csdx->nsd);
    if (debug) printf("  rt=%d  nusdas_parameter_change(N_PC_ID_SET, %d)\n", rt, csdx->nsd);
    t1 = csdx->types;
    t2 = csdx->types + 8;
    t3 = csdx->types + 12;
    bt = csdx->bt;

    if ((rt = nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_SUBC_NUM,dnull,&n,1)) == 1) {
        if (n > 0) {
            char   *groups = calloc(n * 4 + 1, 1);
            if (groups == NULL) { printf("!E  groups = malloc(%d)\n", n * 4 + 1); exit(1); }
            rt = nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_SUBC_LIST,dnull,groups,n);
            if (debug) printf("  subc groups = %d (%s) %s\n", n, groups, rt == n ? "":"Error");
            for (i=0; i<n; i++) {
                int     byte;
                char    group[5];
                int     (*proc)();

                strncpy(group, groups + i*4, 4); group[4] = '\0';
                if (debug) {
                    rt = nusdas_inq_subcinfo(t1,t2,t3,&bt,mmb,&vt,N_SUBC_NBYTES,group,&byte,1);
                    printf("   rt=%d NUSDAS%02d:nusdas_inq_subcinfo(%s,%d,%s,%d,N_SUBC_NBYTES,%s) byte=%d\n", rt, csdx->nsd, t1, bt, mmb, vt, group, byte);
                }

                if      (strcmp("RADR",group) == 0) proc = proc_subc_srf;
                else if (strcmp("RADS",group) == 0) proc = proc_subc_srf;
                else if (strcmp("DPRD",group) == 0) proc = proc_subc_srf;
                else if (strcmp("ISPC",group) == 0) proc = proc_subc_srf;
                else if (strcmp("THUN",group) == 0) proc = proc_subc_srf;
                else if (strcmp("DELT",group) == 0) proc = proc_subc_delt;
                else if (strcmp("RGAU",group) == 0) proc = proc_subc_rgau;
                else if (strcmp("TDIF",group) == 0) proc = proc_subc_tdif;
                else if (strcmp("ETA ",group) == 0) proc = proc_subc_eta;
                else if (strcmp("ZHYB",group) == 0) proc = proc_subc_zhyb;
                else                                proc = NULL;

                if (proc != NULL) {
                    rt = proc(csdx,csdy,mmb,vt,pln,elm,group,mode);
                    if (rt != 0) exitstatus = rt;
                } else     printf(" SUBC unknown? %s \n", group);
            }
            if (groups != NULL) free(groups);
        }
    } else {
        printf("  rt=%d nusdas_inq_subcinfo(%s,%d,%s,%d,N_SUBC_NUM) n=%d\n", rt, t1, bt, mmb, vt, n);
        rt = -1;
    }

    return  exitstatus;
}
