/* nuscntl.c */
/* 2005/11/7 tabito */
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <strings.h>
#include <limits.h>
#include "stringplus.h"
#include <nusdas.h>
#include "textout.h"
#include "nusdim.h"


enum {N_RADR, N_RADS, N_ISPC, N_DPRD, N_THUN};

#define DEFAULT_GNAME N_RADR

static char rcsid[] = "$Id: nussubc_srf.c,v 1.3 2007-02-23 05:39:56 suuchi43 Exp $";


struct nussubc_srf_opts{
    int (*operator)(struct nus_dims_t *dp, struct nussubc_srf_opts *op);
    int gname;
};

int
nusdims_subc_srf_int(struct nus_dims_t *dp, struct nussubc_srf_opts *op);


	void
opt_clear(struct nussubc_srf_opts *op)
{
    op->operator = nusdims_subc_srf_int;
    op->gname = DEFAULT_GNAME;
}

	void
option(struct nussubc_srf_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
	case 'g':
            str++;
            if(memcmp(str, "RADR", 4) == 0){
                op->gname = N_RADR;
            }
            else if(memcmp(str, "RADS", 4) == 0){
                op->gname = N_RADS;
            }
            else if(memcmp(str, "ISPC", 4) == 0){
                op->gname = N_ISPC;
            }
            else if(memcmp(str, "DPRD", 4) == 0){
                op->gname = N_DPRD;
            }
            else if(memcmp(str, "THUN", 4) == 0){
                op->gname = N_THUN;
            }
            break;
	default:
		break;
	}
}

	int
nusdims_subc_srf_int(struct nus_dims_t *dp, struct nussubc_srf_opts *op)
{
    int	rt;
    char gname[4];
    int *data;
    int unit;

    switch (op->gname){
    case N_RADR:
        memcpy(gname, "RADR", 4);
        unit = 1;
        break;

    case N_RADS:
        memcpy(gname, "RADS", 4);
        unit = 6;
        break;

    case N_ISPC:
        memcpy(gname, "ISPC", 4);
        unit = 128;
        break;

    case N_DPRD:
        memcpy(gname, "DPRD", 4);
        unit = 8;
        break;

    case N_THUN:
        memcpy(gname, "THUN", 4);
        unit = 1;
        break;
    default:
        break;
        
    }

    if((data = (int*)malloc(unit * sizeof(int))) == NULL){
        eprintf(HTTP_ERR, "malloc error: %s, %d\n", __FILE__, __LINE__);
        return -10;
    }

    rt = nusdas_subc_srf2(dp->type.type1, dp->type.type2, dp->type.type3, 
                          &(dp->base), dp->member, 
                          &(dp->valid), &(dp->valid2), 
                          dp->plane, dp->plane2, dp->element, gname, 
                          (N_SI4*)data, N_IO_GET);
    if(rt == 0){
	text_init("application/x-int32-stream");
        endian_swab4(data, unit);
        twrite(data, sizeof(int), unit);
    }
    else{
        eprintf(HTTP_ERR, "nusdas_subc_srf2 error! rt = %d\n", rt);
    }
    free(data);
    return rt;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nussubc_srf_opts opt;
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
