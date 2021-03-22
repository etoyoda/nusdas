/*---------------------------------------------------------------------
nusrgau.c
2006.10 Tabito Hara

nusdas_subc_rgau を出力
デフォルトは, bin で出力
オプション
-t : bin で出力(デフォルト)
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "stringplus.h"
#include <nusdas.h>
#include "textout.h"
#include "nusdim.h"

struct nusrgau_opts{
    int (*operator)(struct nus_dims_t *, struct nusrgau_opts *);
};

int nusdims_rgau_bin(struct nus_dims_t *dp, struct nusrgau_opts *op);

/*--------------------------------------------------------------------------*/
	void
opt_clear(struct nusrgau_opts *op)
{
    op->operator = nusdims_rgau_bin; 
}
/*--------------------------------------------------------------------------*/
	void
option(struct nusrgau_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
        case 'b':
            op->operator = nusdims_rgau_bin;
            break;
	default:
		break;
	}
}
/*--------------------------------------------------------------------------*/
	int
nusdims_rgau_bin(struct nus_dims_t *dp, struct nusrgau_opts *op)
/*
Format:
j         int     4
j_start   int     4
j_n       int     4
i         int     4 * j_n
i_start   int     4 * j_n
i_n       int     4 * j_n
lat       float   4 * j_n
*/
{
    N_SI4 r, n = 1;
    N_SI4 j, j_start, j_n;
    N_SI4 *i, *i_start, *i_n;
    float *lat;

    r = nusdas_subc_rgau_inq_jn2
        (dp->type.type1, dp->type.type2, dp->type.type3,
         &dp->base, dp->member, &dp->valid, &dp->valid2,
         &j_n);

    if (r <= 0) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_subc_rgau_inq_jn] %d\n", r);
        return r;
    }
    tprintf("X-RGAU-JN", "%d\n", j_n);

    if(
        (i = (N_SI4*)malloc(j_n * sizeof(N_SI4))) == NULL ||
        (i_start = (N_SI4*)malloc(j_n * sizeof(N_SI4))) == NULL ||
        (i_n = (N_SI4*)malloc(j_n * sizeof(N_SI4))) == NULL ||
        (lat = (float*)malloc(j_n * sizeof(float))) == NULL){
        eprintf(HTTP_ERR, "malloc error at nusdims_rgau_bin\n");
        return -10;
    }

    r = nusdas_subc_rgau2
        (dp->type.type1, dp->type.type2, dp->type.type3,
         &dp->base, dp->member, &dp->valid, &dp->valid2,
         &j, &j_start, &j_n,
         i, i_start, i_n,
         lat, N_IO_GET);

    if (r < 0) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_subc_rgau] %d\n", r);
        free(i);
        free(i_start);
        free(i_n);
        free(lat);
        return r;
    }

    twrite(&j, sizeof(N_SI4), 1);
    twrite(&j_start, sizeof(N_SI4), 1);
    twrite(&j_n, sizeof(N_SI4), 1);
    twrite(i, sizeof(N_SI4), j_n);
    twrite(i_start, sizeof(N_SI4), j_n);
    twrite(i_n, sizeof(N_SI4), j_n);
    twrite(lat, sizeof(float), j_n);

    free(i);
    free(i_start);
    free(i_n);
    free(lat);
    return 0;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nusrgau_opts opt;
	int	argind;
	int	r;

	if (argv[1] == NULL) {
		eprintf(HTTP_ERR, "usage: %s <path>\n", argv[0]);
		text_end();
		return 1;
	}
	text_init("text/plain");
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


