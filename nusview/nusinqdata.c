/* nusinqdata.c */
/* 2005/6/6 tabito */
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <strings.h>
#include <limits.h>
#include "stringplus.h"
#include <nusdas.h>
#include "textout.h"
#include "nusdim.h"


static char rcsid[] = "$Id: nusinqdata.c,v 1.2 2007-03-09 10:54:22 suuchi04 Exp $";

struct nusdata_opts{
    int (*operator)(struct nus_dims_t *dp, struct nusdata_opts *op);
    int put_title;
    int is_radar;
};

int nusdims_data_rd(struct nus_dims_t *dp, struct nusdata_opts *op);



	void
opt_clear(struct nusdata_opts *op)
{
    op->operator =nusdims_data_rd; 
    op->put_title = 0;
}

	void
option(struct nusdata_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
	case 't':
             if (str[1] == 'r') {
                tset_content_type("text/x-rd");
                op->operator =nusdims_data_rd;  
            }
            break;
        case 'l':
            op->put_title = 1;
	default:
		break;
	}
}

	int
nusdims_data_rd(struct nus_dims_t *dp, struct nusdata_opts *op)
{
    int	    r;
    int     array_num;
    int     gsize[2];
    char    pack[4];
    char    miss[4];
    int     rectime;
    int     ii;
    
    /* GRID_SIZE */
    array_num = 2;
    r = nusdas_inq_data2(dp->type.type1, dp->type.type2, dp->type.type3,
                         &dp->base, dp->member, &dp->valid, &dp->valid2,
			 dp->plane, dp->plane2, dp->element, 
                         N_GRID_SIZE, (void*)&gsize, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_data2(N_GRID_SIZE)] %d, %d\n", r, array_num);
        return r;
    }

    /* PACKING */
    array_num = 1;
    r = nusdas_inq_data2(dp->type.type1, dp->type.type2, dp->type.type3,
                         &dp->base, dp->member, &dp->valid, &dp->valid2,
			 dp->plane, dp->plane2, dp->element, 
                         N_PC_PACKING, (void*)pack, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_data2(N_PC_PACKING)] %d, %d\n", r, array_num);
        return r;
    }

    /* MISSING_MODE */
    array_num = 1;
    r = nusdas_inq_data2(dp->type.type1, dp->type.type2, dp->type.type3,
                         &dp->base, dp->member, &dp->valid, &dp->valid2,
			 dp->plane, dp->plane2, dp->element, 
                         N_MISSING_MODE, (void*)miss, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_data2(N_MISSING_MODE)] %d, %d\n", r, array_num);
        return r;
    }

    /* RECORD TIME */
    array_num = 1;
    r = nusdas_inq_data2(dp->type.type1, dp->type.type2, dp->type.type3,
                         &dp->base, dp->member, &dp->valid, &dp->valid2,
			 dp->plane, dp->plane2, dp->element, 
                         N_RECORD_TIME, (void*)&rectime, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_data2(N_RECORD_TIME)] %d, %d\n", r, array_num);
        return r;
    }
    
    /*  caution !!
        add 1 space after comma
    */

    tprintf(0,"=begin\n");
    if(op->put_title == 1){
        tprintf(0,"=data for %s\n", nusdims_to_path(dp));
    }
    tprintf(0,"==data info\n");
    tprintf(0,"=end\n");
    tprintf(0,"=begin RT\n");
    /* GRID_SIZE */
    tprintf(0, "number of x grids, %d\n", gsize[0]);
    tprintf(0, "number of y grids, %d\n", gsize[1]);
    /* PACKING */
    tprintf(0, "packing, %4.4s\n", pack);
    /* MISSING */
    tprintf(0, "missing mode, %4.4s\n", miss);
    /* RECORD TIME */
    tprintf(0, "record time, %d\n", rectime);    

    tprintf(0,"=end RT\n");
    tprintf(0,"\r\n");
    
    return 0;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nusdata_opts opt;
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
		if (!(r & NUSDIM_ELEMENT)) {
			eprintf(HTTP_ERR, "element not specified\n");
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
