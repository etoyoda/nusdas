/*---------------------------------------------------------------------
nustdif.c
2005.6.1 Tabito Hara

info を出力
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

struct nusinfo_opts{
    int (*operator)(struct nus_dims_t *, struct nusinfo_opts *);
    int put_title;
};

int nusdims_info_bin(struct nus_dims_t *dp, struct nusinfo_opts *op);

/*--------------------------------------------------------------------------*/
	void
opt_clear(struct nusinfo_opts *op)
{
    op->operator = nusdims_info_bin; 
    op->put_title = 0;
}
/*--------------------------------------------------------------------------*/
	void
option(struct nusinfo_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
        case 'b':
            op->operator = nusdims_info_bin;
            break;
	default:
		break;
	}
}
/*--------------------------------------------------------------------------*/
	int
nusdims_info_bin(struct nus_dims_t *dp, struct nusinfo_opts *op)
/*

*/
{
    N_SI4 r, n = 1;
    char gname[5];
    unsigned char *info;
    int alloc_size;

    memcpy(gname, &dp->element, 4);
    gname[4] = '\0';

    alloc_size = 10000;
    info = (unsigned char*)malloc(alloc_size);
    if(info == NULL){
        eprintf(HTTP_ERR, "malloc error\n", r);
        return -10;
    }

    r = nusdas_info2(dp->type.type1, dp->type.type2, dp->type.type3,
                         &dp->base, dp->member, &dp->valid, &dp->valid2,
                         gname, (void*)info, (N_SI4*)&alloc_size, N_IO_GET);
    if (r < 0) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_info] %d\n", r);
        return r;
    }
    twrite(info, 1, r);
    free(info);
    return 0;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nusinfo_opts opt;
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


