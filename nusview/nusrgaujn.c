/*---------------------------------------------------------------------
nusrgaujn.c
2006.10 Tabito Hara

nusdas_subc_rgau_inq_jn を出力
デフォルトは, text で出力
オプション
-t : text で出力(デフォルト)
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "stringplus.h"
#include <nusdas.h>
#include "textout.h"
#include "nusdim.h"

struct nusrgaujn_opts{
    int (*operator)(struct nus_dims_t *, struct nusrgaujn_opts *);
};

int nusdims_rgaujn_txt(struct nus_dims_t *dp, struct nusrgaujn_opts *op);

/*--------------------------------------------------------------------------*/
	void
opt_clear(struct nusrgaujn_opts *op)
{
    op->operator = nusdims_rgaujn_txt; 
}
/*--------------------------------------------------------------------------*/
	void
option(struct nusrgaujn_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
        case 't':
            op->operator = nusdims_rgaujn_txt;
            break;
	default:
		break;
	}
}
/*--------------------------------------------------------------------------*/
	int
nusdims_rgaujn_txt(struct nus_dims_t *dp, struct nusrgaujn_opts *op)
/*

*/
{
    N_SI4 r, n = 1;
    N_SI4 j_n;

    r = nusdas_subc_rgau_inq_jn2
        (dp->type.type1, dp->type.type2, dp->type.type3,
         &dp->base, dp->member, &dp->valid, &dp->valid2,
         &j_n);

    if (r <= 0) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_subc_rgau_inq_jn] %d\n", r);
        return r;
    }
    tprintf(0, "%d\n", j_n);
    return 0;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nusrgaujn_opts opt;
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


