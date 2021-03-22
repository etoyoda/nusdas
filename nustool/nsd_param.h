#include   "nwpl_capi.h"
#include   "nusdas.h"

/* NuSDaS非公開関数を使うための黒魔法 */
#include   "config.h"
#include   "internal_types.h"
#define    NEED_STR2SYM4
#include   "sys_sym.h"
#include   "sys_time.h"

struct nsd_parm {
        int     nsd;
        N_SI4   nb, *bt;             /* basetime */
        N_SI4   nf, *ft;             /* fcsttime */
        sym4_t  fu;                  /* fcstunit */
        N_SI4   nm,  np,  ne,  nw, nr, size[2];
        char   *mb, *pl, *el;        /* member,plane,element (nothing NULL) */
        char                  *ew;   /* element write-map (1 byte integer)  */
        char                  *er;   /* element  read-map (1 byte integer)  */
        char   *types;
    };

void
print_map(int m, int k, int nsd, int n_mb, int n_vt, int n_pl, int n_el, int flag_size, char *tb_mb, int *tb_vt, char *tb_pl, char *tb_el, char *flag);
