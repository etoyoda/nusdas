#include   <stdio.h>
#include   <string.h>
#include   <stdlib.h>
#include   <time.h>

#include   "nusdas.h"
#include   "nwpl_capi.h"
#include   "obstation.h"

#ifdef TEST
    int             debug = 1;
#else
    extern  int     debug;
#endif


static int  num_amd, num_obs, num_pub, num_aws;
static struct  station_s  *amd = NULL;
static struct  station_s  *obs = NULL;
static struct  station_s  *pub = NULL;
static struct  station_s  *aws = NULL;

static int cmp_stn(const void *p1, const void *p2)
{
    struct  station_s  *s1 = (struct station_s *)p1;
    struct  station_s  *s2 = (struct station_s *)p2;
    return s1->no - s2->no;
}

static int f_open(int n, char *fnames[], FILE **pfp)
{
    int     i;
    char   *fname, str[256];
    for (i=0; i<n; i++) {
        fname = fnames[i];
        if ((*pfp = fopen(fname,"r")) != NULL) break;
    }
    if (*pfp == NULL) {
        printf("!W  can not open: %s\n", fname);
        return -1;  /* can not open */
    }
    n = 0;
    while(fgets(str,255,*pfp) != NULL) ++n;
    rewind(*pfp);
    if (debug) printf(" %d(lines) %s \n", n, fname);
    return  n;
}

static struct stn_joho_s *joho_search(int sq, struct stn_joho_s *joho)
{
    if ((sq >= joho->bgn) && (sq <= joho->fin)) return  joho;
    if (joho->next != NULL) return joho_search(sq, joho->next);
    return  NULL;
}

static int rd_amd()
{
    static char  *tbl[] = {"/nwp/Rtn/Const/Pre/Dcd/amddic.txt"
                          ,"AMDDIC_TXT"
                          };
    FILE   *fp;
    int     n;
    char    str[256];
    struct  station_s  *amdp = NULL;

    if (amd != NULL) return 0;
    if ((n = f_open(2,tbl,&fp)) < 0) return -1;
    if ((amd = calloc(n,sizeof(struct station_s))) == NULL) return -3;
    n = 0;
    while (fgets(str,255,fp) != NULL) {
        int    id, high, y, m, d, h, g, bgn, fin;
        float  lat, lap, lon, lop;
        struct  stn_joho_s  *joho;

        str[90] = '\0';
        sscanf (str+65, "%4d%2d%2d%2d%2d", &y, &m, &d, &h, &g);
        bgn = nwp_ymdhm2seq(y,m,d,h,g);
        sscanf (str+78, "%4d%2d%2d%2d%2d", &y, &m, &d, &h, &g);
        fin = nwp_ymdhm2seq(y,m,d,h,g);
        str[28] = '\0'; if (sscanf(str+24,"%d",&high) < 1) high = -1;
        str[23] = '\0'; if (sscanf(str+19,"%f" ,&lop) < 1) lop = 0;
        str[19] = '\0'; if (sscanf(str+15,"%3f",&lon) < 1) lon = -1;
        str[14] = '\0'; if (sscanf(str+10,"%f" ,&lap) < 1) lap = 0;
        str[10] = '\0'; if (sscanf(str+7, "%2f",&lat) < 1) lat = -1;
        str[6]  = '\0'; if (sscanf(str+1, "%5d",&id)  < 1) id = -1;
        if ((amdp == NULL) || (id != amdp->no)) {          /* 地点を追加 */
            amdp = amd + n++;
            amdp->no = id;
            amdp->joho = malloc(sizeof(struct stn_joho_s));
            joho = amdp->joho;
        } else if (id == amdp->no) {                       /* 地点に追加 */
            joho = amdp->joho;
            while (joho->next != NULL) joho = joho->next;
            joho->next = malloc(sizeof(struct stn_joho_s));
            joho = joho->next;
        }
        joho->bgn  = bgn;
        joho->fin  = fin;
        joho->high = high;
        joho->lat  = lat + lap / 60;
        joho->lon  = lon + lop / 60;
        joho->next = NULL;
    }
    return  n;
}

int
amd_search(int no, int sq, float *lat, float *lon, int *high)
{
    struct  station_s   *p;
    struct  stn_joho_s  *joho;
    static  int   amd_flag = 0;
    if (amd == NULL) {
        if (amd_flag != 0) return  *high = *lat = *lon = -1;
        amd_flag = 1;
        if ((num_amd = rd_amd()) <= 0) return  *high = *lat = *lon = -1;
    }

    p = bsearch(&no, amd, num_amd, sizeof(struct station_s), cmp_stn);
    if (p == NULL) return  *high = *lat = *lon = -1;
    joho = joho_search(sq, p->joho);
    if (joho == NULL) return  *high = *lat = *lon = -1;
    *lat  = joho->lat;
    *lon  = joho->lon;
    *high = joho->high;
    return  0;
}


static int rd_obs()
{
    static char  *tbl[] = {"/nwp/Rtn/Const/Vsrf/Dcd/predic.txt"
                          ,"PREDIC_TXT"
                          };
    FILE   *fp;
    int     n;
    char    str[256];
    struct  station_s  *obsp = NULL;

    if (obs != NULL) return 0;
    if ((n = f_open(2,tbl,&fp)) < 0) return -1;
    if ((obs = calloc(n,sizeof(struct station_s))) == NULL) return -3;
    n = 0;
    while (fgets(str,255,fp) != NULL) {
        int    id, high, y, m, d, h, g, bgn, fin;
        float  lat, lap, lon, lop;
        struct  stn_joho_s  *joho;

        str[80] = '\0';
        sscanf (str+62, "%4d%2d%2d", &y, &m, &d); h = g = 0;
        bgn = nwp_ymdhm2seq(y,m,d,h,g);
        sscanf (str+71, "%4d%2d%2d", &y, &m, &d); h = g = 0;
        fin = nwp_ymdhm2seq(y,m,d,h,g);
        str[29] = '\0'; if (sscanf(str+25,"%d",&high) < 1) high = -1;
        str[24] = '\0'; if (sscanf(str+20,"%f" ,&lop) < 1) lop = 0;
        str[20] = '\0'; if (sscanf(str+16,"%3f",&lon) < 1) lon = -1;
        str[15] = '\0'; if (sscanf(str+11,"%f" ,&lap) < 1) lap = 0;
        str[11] = '\0'; if (sscanf(str+8, "%2f",&lat) < 1) lat = -1;
        str[7]  = '\0'; if (sscanf(str+0, "%7d",&id)  < 1) id = -1;
        if ((obsp == NULL) || (id != obsp->no)) {          /* 地点を追加 */
            obsp = obs + n++;
            obsp->no = id;
            obsp->joho = malloc(sizeof(struct stn_joho_s));
            joho = obsp->joho;
        } else if (id == obsp->no) {                       /* 地点に追加 */
            joho = obsp->joho;
            while (joho->next != NULL) joho = joho->next;
            joho->next = malloc(sizeof(struct stn_joho_s));
            joho = joho->next;
        }
        joho->bgn  = bgn;
        joho->fin  = fin;
        joho->high = high;
        joho->lat  = lat + lap / 60;
        joho->lon  = lon + lop / 60;
        joho->next = NULL;
    }
    return  n;
}

int
obs_search(int no, int sq, float *lat, float *lon, int *high)
{
    struct  station_s   *p;
    struct  stn_joho_s  *joho;
    static  int   obs_flag = 0;
    if (obs == NULL) {
        if (obs_flag != 0) return  *high = *lat = *lon = -1;
        obs_flag = 1;
        if ((num_obs = rd_obs()) <= 0) return  *high = *lat = *lon = -1;
    }
    p = bsearch(&no, obs, num_obs, sizeof(struct station_s), cmp_stn);
    if (p == NULL) return  *high = *lat = *lon = -1;
    joho = joho_search(sq, p->joho);
    if (joho == NULL) return  *high = *lat = *lon = -1;
    *lat  = joho->lat;
    *lon  = joho->lon;
    *high = joho->high;
    return  0;
}


static int rd_aws()
{
    static char  *tbl[] = {"/nwp/Rtn/Const/Vsrf/Dcd/awsdic.txt"
                          ,"AWSDIC_TXT"
                          };
    FILE   *fp;
    int     n;
    char    str[256];
    struct  station_s  *awsp = NULL;

    if (aws != NULL) return 0;
    if ((n = f_open(2,tbl,&fp)) < 0) return -1;
    if ((aws = calloc(n,sizeof(struct station_s))) == NULL) return -3;
    n = 0;
    while (fgets(str,255,fp) != NULL) {
        int    id, high, y, m, d, h, g, bgn, fin;
        float  lat, lap, lon, lop;
        struct  stn_joho_s  *joho;

        str[90] = '\0';
        sscanf (str+65, "%4d%2d%2d%2d%2d", &y, &m, &d, &h, &g);
        bgn = nwp_ymdhm2seq(y,m,d,h,g);
        sscanf (str+78, "%4d%2d%2d%2d%2d", &y, &m, &d, &h, &g);
        fin = nwp_ymdhm2seq(y,m,d,h,g);
        str[28] = '\0'; if (sscanf(str+24,"%d",&high) < 1) high = -1;
        lop = 0;
        str[23] = '\0'; if (sscanf(str+17,"%f",&lon) < 1) lon = -1;
        lap = 0;
        str[14] = '\0'; if (sscanf(str+9, "%f",&lat) < 1) lat = -1;
        str[6]  = '\0'; if (sscanf(str+1, "%5d",&id)  < 1) id = -1;
        if ((awsp == NULL) || (id != awsp->no)) {          /* 地点を追加 */
            awsp = aws + n++;
            awsp->no = id;
            awsp->joho = malloc(sizeof(struct stn_joho_s));
            joho = awsp->joho;
        } else if (id == awsp->no) {                       /* 地点に追加 */
            joho = awsp->joho;
            while (joho->next != NULL) joho = joho->next;
            joho->next = malloc(sizeof(struct stn_joho_s));
            joho = joho->next;
        }
        joho->bgn  = bgn;
        joho->fin  = fin;
        joho->high = high;
        joho->lat  = lat + lap / 60;
        joho->lon  = lon + lop / 60;
        joho->next = NULL;
    }
    return  n;
}

int
aws_search(int no, int sq, float *lat, float *lon, int *high)
{
    struct  station_s   *p;
    struct  stn_joho_s  *joho;
    static  int   aws_flag = 0;
    if (aws == NULL) {
        if (aws_flag != 0) return  *high = *lat = *lon = -1;
        aws_flag = 1;
        if ((num_aws = rd_aws()) <= 0) return  *high = *lat = *lon = -1;
    }

    p = bsearch(&no, aws, num_aws, sizeof(struct station_s), cmp_stn);
    if (p == NULL) return  *high = *lat = *lon = -1;
    joho = joho_search(sq, p->joho);
    if (joho == NULL) return  *high = *lat = *lon = -1;
    *lat  = joho->lat;
    *lon  = joho->lon;
    *high = joho->high;
    return  0;
}


static int rd_pub()
{
    static char  *tbl[] = {"/nwp/Rtn/Const/Vsrf/Dcd/pubdic.txt"
                          ,"PUBDIC_TXT"
                          };
    FILE   *fp;
    int     n;
    char    str[256];
    struct  station_s  *pubp = NULL;

    if (pub != NULL) return 0;
    if ((n = f_open(2,tbl,&fp)) < 0) return -1;
    if ((pub = calloc(n,sizeof(struct station_s))) == NULL) return -3;
    n = 0;
    while (fgets(str,255,fp) != NULL) {
        int    id, high, y, m, d, h, g, bgn, fin;
        float  lat, lap, lon, lop;
        struct  stn_joho_s  *joho;

        str[90] = '\0';
        sscanf (str+65, "%4d%2d%2d%2d%2d", &y, &m, &d, &h, &g);
        bgn = nwp_ymdhm2seq(y,m,d,h,g);
        sscanf (str+78, "%4d%2d%2d%2d%2d", &y, &m, &d, &h, &g);
        fin = nwp_ymdhm2seq(y,m,d,h,g);
        str[28] = '\0'; if (sscanf(str+24,"%d",&high) < 1) high = -1;
        str[23] = '\0'; if (sscanf(str+19,"%f" ,&lop) < 1) lop = 0;
        str[19] = '\0'; if (sscanf(str+15,"%3f",&lon) < 1) lon = -1;
        str[14] = '\0'; if (sscanf(str+10,"%f" ,&lap) < 1) lap = 0;
        str[10] = '\0'; if (sscanf(str+7, "%2f",&lat) < 1) lat = -1;
        str[6]  = '\0'; if (sscanf(str+0, "%6d",&id)  < 1) id = -1;
        if ((pubp == NULL) || (id != pubp->no)) {          /* 地点を追加 */
            pubp = pub + n++;
            pubp->no = id;
            pubp->joho = malloc(sizeof(struct stn_joho_s));
            joho = pubp->joho;
        } else if (id == pubp->no) {                       /* 地点に追加 */
            joho = pubp->joho;
            while (joho->next != NULL) joho = joho->next;
            joho->next = malloc(sizeof(struct stn_joho_s));
            joho = joho->next;
        }
        joho->bgn  = bgn;
        joho->fin  = fin;
        joho->high = high;
        joho->lat  = lat + lap / 60;
        joho->lon  = lon + lop / 60;
        joho->next = NULL;
    }
    return  n;
}

int
pub_search(int no, int sq, float *lat, float *lon, int *high)
{
    struct  station_s   *p;
    struct  stn_joho_s  *joho;
    static  int   pub_flag = 0;
    if (pub == NULL) {
        if (pub_flag != 0) return  *high = *lat = *lon = -1;
        pub_flag = 1;
        if ((num_pub = rd_pub()) <= 0) return  *high = *lat = *lon = -1;
    }
    p = bsearch(&no, pub, num_pub, sizeof(struct station_s), cmp_stn);
    if (p == NULL) return  *high = *lat = *lon = -1;
    joho = joho_search(sq, p->joho);
    if (joho == NULL) return  *high = *lat = *lon = -1;
    *lat  = joho->lat;
    *lon  = joho->lon;
    *high = joho->high;
    return  0;
}


#ifdef TEST
int pr_stn(int no, struct stn_joho_s *joho)
{
    static int n = 0;
    if (joho == NULL) return printf("nothing(%d)\n", no);
    printf("%-5d  %5d %6.3f %7.3f %4d %d %d \n", ++n,
            no, joho->lat, joho->lon, joho->high, joho->bgn, joho->fin);
    if (joho->next != NULL) pr_stn(no, joho->next);
    return  0;
}

int main(int argc, char **argv)
{
    char   *myname = *argv++;
    int     i, no, high, sq;
    float   lat, lon;
    struct  station_s  *p;

    no = atoi(*argv++);
    sq = atoi(*argv++);
    if (sq == 0) sq = 108600480;

/*
    for (i=0; i<num_amd; i++) pr_stn(amd[i].no, amd[i].joho);
    p = bsearch(&no, amd, num_amd, sizeof(struct station_s), cmp_stn);
    i = p - amd;
    pr_stn(amd[i].no, amd[i].joho);
    pr_stn(no, joho_search(108600480,amd[i].joho));
    pr_stn(no, joho_search(105190560,amd[i].joho));
    amd_search(no, sq, &lat, &lon, &high);
    printf("    %d (%d) lat,lon,high = %6.3f, %7.3f, %d \n", no, sq, lat, lon, high);

    for (i=0; i<num_obs; i++) pr_stn(obs[i].no, obs[i].joho);
    obs_search(no, sq, &lat, &lon, &high);
    printf("    %d (%d) lat,lon,high = %6.3f, %7.3f, %d \n", no, sq, lat, lon, high);

    pub_search(no, sq, &lat, &lon, &high);
    printf("    %d (%d) lat,lon,high = %6.3f, %7.3f, %d \n", no, sq, lat, lon, high);
    for (i=0; i<num_pub; i++) pr_stn(pub[i].no, pub[i].joho);

    for (i=0; i<num_aws; i++) pr_stn(aws[i].no, aws[i].joho);
*/
    aws_search(no, sq, &lat, &lon, &high);
    printf("    %d (%d) lat,lon,high = %6.3f, %7.3f, %d \n", no, sq, lat, lon, high);

    return  0;
}
#endif
