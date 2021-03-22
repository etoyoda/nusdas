/*---------------------------------------------------------------------
nuszhyb.c
2006.10 Tabito Hara

nusdas_subc_zhyb を出力
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

struct nuszhyb_opts{
    int (*operator)(struct nus_dims_t *, struct nuszhyb_opts *);
};

int nusdims_zhyb_bin(struct nus_dims_t *dp, struct nuszhyb_opts *op);

/*--------------------------------------------------------------------------*/
	void
opt_clear(struct nuszhyb_opts *op)
{
    op->operator = nusdims_zhyb_bin; 
}
/*--------------------------------------------------------------------------*/
	void
option(struct nuszhyb_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
        case 'b':
            op->operator = nusdims_zhyb_bin;
            break;
	default:
		break;
	}
}
/*--------------------------------------------------------------------------*/
	int
nusdims_zhyb_bin(struct nus_dims_t *dp, struct nuszhyb_opts *op)
/*
Format:
nz        int     4
ptrf      float   4
presrf    float   4
zrp       float   4 * nz
zrw       float   4 * nz
vctrans_p float   4 * nz
vctrans_w float   4 * nz
dvtrans_p float   4 * nz
dvtrans_w float   4 * nz
*/
{
    N_SI4 r, n = 1;
    N_SI4 nz;
    float ptrf, presrf;
    float *zrp, *zrw, *vctrans_p, *vctrans_w, *dvtrans_p, *dvtrans_w;

    r = nusdas_subc_eta_inq_nz2
        (dp->type.type1, dp->type.type2, dp->type.type3,
         &dp->base, dp->member, &dp->valid, &dp->valid2,
         "ZHYB", &nz);

    if (r <= 0) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_subc_eta_inq_nz] %d\n", r);
        return r;
    }
    tprintf("X-ZHYB-NZ", "%d\n", nz);

    if(
        (zrp = (float*)malloc(nz * sizeof(float))) == NULL ||
        (zrw = (float*)malloc(nz * sizeof(float))) == NULL ||
        (vctrans_p = (float*)malloc(nz * sizeof(float))) == NULL ||
        (vctrans_w = (float*)malloc(nz * sizeof(float))) == NULL ||
        (dvtrans_p = (float*)malloc(nz * sizeof(float))) == NULL ||
        (dvtrans_w = (float*)malloc(nz * sizeof(float))) == NULL){
        eprintf(HTTP_ERR, "malloc error at nusdims_zhyb_bin\n");
        return -10;
    }

    r = nusdas_subc_zhyb2
       (dp->type.type1, dp->type.type2, dp->type.type3,
        &dp->base, dp->member, &dp->valid, &dp->valid2,
	&nz, &ptrf, &presrf, zrp, zrw,
	vctrans_p, vctrans_w, dvtrans_p, dvtrans_w, N_IO_GET);

    if (r < 0) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_subc_zhyb] %d\n", r);
        free(zrp);
        free(zrw);
        free(vctrans_p);
        free(vctrans_w);
        free(dvtrans_p);
        free(dvtrans_w);
        return r;
    }

    twrite(&nz, sizeof(int), 1);
    twrite(&ptrf, sizeof(float), 1);
    twrite(&presrf, sizeof(float), 1);
    twrite(zrp, sizeof(float), nz);
    twrite(zrw, sizeof(float), nz);
    twrite(vctrans_p, sizeof(float), nz);
    twrite(vctrans_w, sizeof(float), nz);
    twrite(dvtrans_p, sizeof(float), nz);
    twrite(dvtrans_w, sizeof(float), nz);

    free(zrp);
    free(zrw);
    free(vctrans_p);
    free(vctrans_w);
    free(dvtrans_p);
    free(dvtrans_w);

    return 0;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nuszhyb_opts opt;
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


