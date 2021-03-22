/*---------------------------------------------------------------------
nustdif.c
2003.7.27 Tabito Hara

tdif を出力
デフォルトは,binary で出力
オプション
-b : binary で出力(デフォルト)
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "stringplus.h"
#include <nusdas.h>
#include "textout.h"
#include "nusdim.h"

struct nustdif_opts{
    int (*operator)(struct nus_dims_t *, struct nustdif_opts *);
    int put_title;
};

int nusdims_tdif_bin(struct nus_dims_t *dp, struct nustdif_opts *op);

/*--------------------------------------------------------------------------*/
	void
opt_clear(struct nustdif_opts *op)
{
    op->operator = nusdims_tdif_bin; 
    op->put_title = 0;
}
/*--------------------------------------------------------------------------*/
	void
option(struct nustdif_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
        case 'b':
            op->operator = nusdims_tdif_bin;
            break;
	default:
		break;
	}
}
/*--------------------------------------------------------------------------*/
	int
nusdims_tdif_bin(struct nus_dims_t *dp, struct nustdif_opts *op)
/*
フォーマット
対象時間からのずれ  long int   1
積算秒              long int   1
計 8バイト
*/
{
    N_SI4 r, n = 1;
    N_SI4 diff_time;
    N_SI4 total_sec;

    r = nusdas_subc_tdif2(dp->type.type1, dp->type.type2, dp->type.type3,
                         &dp->base, dp->member, &dp->valid, &dp->valid2,
                         &diff_time, &total_sec, N_IO_GET);
    if (r < 0) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_subc_tdif2] %d\n", r);
        return r;
    }
    twrite(&diff_time, sizeof(N_SI4), 1);
    twrite(&total_sec, sizeof(N_SI4), 1);
    return 0;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nustdif_opts opt;
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


