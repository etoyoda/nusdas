/*---------------------------------------------------------------------
nussigm.c
2003.7.27 Tabito Hara

sigma と eta の補助管理部を出力
デフォルトは,sigma を binary で出力
オプション
-s : sigma を出力(デフォルト)
-e : eta を出力
-b : binary で出力(デフォルト)
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "stringplus.h"
#include <nusdas.h>
#include "textout.h"
#include "nusdim.h"

#define PLANE_SIGMA 0
#define PLANE_ETA 1

struct nussigm_opts{
    int (*operator)(struct nus_dims_t *, struct nussigm_opts *);
    int plane_name;
    int put_title;
};

int nusdims_sigm_bin(struct nus_dims_t *dp, struct nussigm_opts *op);

/*--------------------------------------------------------------------------*/
	void
opt_clear(struct nussigm_opts *op)
{
    op->operator = nusdims_sigm_bin; 
    op->plane_name = PLANE_SIGMA;
    op->put_title = 0;
}
/*--------------------------------------------------------------------------*/
	void
option(struct nussigm_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
        case 'b':
            op->operator = nusdims_sigm_bin;
            break;
        case 's':
            op->plane_name = PLANE_SIGMA;
            break;
        case 'e':
            op->plane_name = PLANE_ETA;
            break;
	default:
		break;
	}
}
/*--------------------------------------------------------------------------*/
	int
nusdims_sigm_bin(struct nus_dims_t *dp, struct nussigm_opts *op)
/*
フォーマット:
A(k)  float  n_lv+1
B(k)  float  n_lv+1
C(k)  float  1
(n_lv は面の数)
計 4*(2*(n_lv+1)+1) byte
*/
{
    N_SI4 r, n = 1;
    N_SI4 n_lv;
    float *array_a, *array_b, *array_c;
    int rt_val;
    char gname[4];

    /*
    r = nusdas_inq_cntl2(dp->type.type1, dp->type.type2, dp->type.type3,
                         &dp->base, dp->member, &dp->valid, &dp->valid2,
                         N_PLANE_NUM, &n_lv, &n);
    */
    
    if(op->plane_name == PLANE_SIGMA){
        memcpy(gname, "SIGM", 4);
    }
    else if(op->plane_name == PLANE_ETA){
        memcpy(gname, "ETA ", 4);
    }
    r = nusdas_subc_eta_inq_nz2
        (dp->type.type1, dp->type.type2, dp->type.type3,
         &dp->base, dp->member, &dp->valid, &dp->valid2,
         gname, &n_lv);
    
    if (r <= 0) {
        float zero = 0;
        tprintf("X-PLANE-NUM","0");
        twrite(&zero, sizeof(float), 1);
        twrite(&zero, sizeof(float), 1);
        twrite(&zero, sizeof(float), 1);
        return 0;
        /*
        eprintf(HTTP_ERR, "nusdas error [nusdas_subc_eta_inq_nz] %d\n", r);
        return r;
        */
    }
    tprintf("X-PLANE-NUM","%d", n_lv);

    array_a = (float*)malloc((n_lv+1)*sizeof(float));
    array_b = (float*)malloc((n_lv+1)*sizeof(float));
    array_c = (float*)malloc(1*sizeof(float));
    if(array_a == NULL || array_b == NULL || array_c == NULL){
        eprintf(HTTP_ERR, "malloc error\n");
        rt_val = -10;
        goto End;
    }
    if(op->plane_name == PLANE_SIGMA){
        r = nusdas_subc_sigm2(dp->type.type1, dp->type.type2, dp->type.type3,
                              &dp->base, dp->member, &dp->valid, &dp->valid2,
                              &n_lv, array_a, array_b, array_c, N_IO_GET);
        if( r < 0){
            memcpy(array_a, '\0', (n_lv+1)*sizeof(float));
            memcpy(array_b, '\0', (n_lv+1)*sizeof(float));
            memcpy(array_c, '\0', 1*sizeof(float));
            rt_val = 0;
            /*
            eprintf(HTTP_ERR, "nusdas error [nusdas_subc_sigm2] %d\n", r);
            rt_val = r; 
            goto End;
            */
        }
    }
    else if(op->plane_name == PLANE_ETA){
        r = nusdas_subc_eta2(dp->type.type1, dp->type.type2, dp->type.type3,
                              &dp->base, dp->member, &dp->valid, &dp->valid2,
                              &n_lv, array_a, array_b, array_c, N_IO_GET);
        if( r < 0){
            memcpy(array_a, '\0', (n_lv+1)*sizeof(float));
            memcpy(array_b, '\0', (n_lv+1)*sizeof(float));
            memcpy(array_c, '\0', 1*sizeof(float));
            rt_val = 0;
            /*
            eprintf(HTTP_ERR, "nusdas error [nusdas_subc_eta2] %d\n", r);
            rt_val = r; 
            goto End;
            */
        }
    }
    else{
        rt_val = -1;
        goto End;
    }
    twrite(array_a, sizeof(float), n_lv + 1);
    twrite(array_b, sizeof(float), n_lv + 1);
    twrite(array_c, sizeof(float), 1);
    rt_val = 0;
End:
    free(array_a);
    free(array_b);
    free(array_c);
    return rt_val;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nussigm_opts opt;
	int	argind;
	int	r;

	if (argv[1] == NULL) {
		eprintf(HTTP_ERR, "usage: %s <path>\n", argv[0]);
		text_end();
		return 1;
	}
	text_init("text/html");
        opt_clear(&opt);
	for (argind = 1; argv[argind] != NULL; argind++) {
		if (argv[argind][0] == '-') {
			option(&opt, argv[argind]);
			continue;
                }
		r = path_to_nusdims(argv[argind], &dims);
		if (!(r & NUSDIM_VALID)) {
			eprintf(HTTP_ERR, "validtime not specified\n");
			tprintf("X-resolved-path", "0x%x %s\n", r, nusdims_to_path(&dims));
			r = 2;
			break;
		}
		r = opt.operator(&dims, &opt);
                tprintf("X-Nusdas-Return-Code", "%d", r);
		if (r != 0) {
			r = 3;
			break;
		}
		r = 0;
	}
	text_end();
	return r;
}


