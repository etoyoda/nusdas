/*---------------------------------------------------------------------
nussubcinfo.c
2007.03 Tabito Hara

nusdas_subcinfo ╓Р╫пно
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "stringplus.h"
#include "nusdas.h"
#include "textout.h"
#include "nusdim.h"

#define SUBC_MODE 0
#define INFO_MODE 1

struct nussubcinfo_opts{ 
    int  si_mode;  /* SUBC or INFO */   
    char recname[5];
    int (*operator)(struct nus_dims_t *, struct nussubcinfo_opts *);
};

int nusdims_subcinfo_bin(struct nus_dims_t *dp, struct nussubcinfo_opts *op);

/*--------------------------------------------------------------------------*/
	void
opt_clear(struct nussubcinfo_opts *op)
{
    op->operator = nusdims_subcinfo_bin; 
    memset(op->recname, 0x20, 4);
    op->recname[4] = 0;
    op->si_mode = SUBC_MODE;
}
/*--------------------------------------------------------------------------*/
	void
option(struct nussubcinfo_opts *op, const char *str)
{
	int ip;

	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
        case 'b':
            op->operator = nusdims_subcinfo_bin;
            break;
	case 'r':
		str++;
		ip = 0;
		while(*str != 0x20 && *str && ip < 4){
			op->recname[ip++] = *str++;
		}
		break;
	case 's':
		op->si_mode = SUBC_MODE;
		break;
	case 'i':
		op->si_mode = INFO_MODE;
		break;
	default:
		break;
	}
}
/*--------------------------------------------------------------------------*/
	int
nusdims_subcinfo_bin(struct nus_dims_t *dp, struct nussubcinfo_opts *op)
{
    N_SI4 r, n = 1;
    N_SI4 nbytes, siz;
    char *buf;
    N_SI4 query_nbytes, query_content;

    if(op->si_mode == SUBC_MODE){
	    query_nbytes = N_SUBC_NBYTES;
	    query_content = N_SUBC_CONTENT;
    }else if(op->si_mode == INFO_MODE){
	    query_nbytes = N_INFO_NBYTES;
	    query_content = N_INFO_CONTENT;
    }
    siz = (N_SI4)sizeof(nbytes);
    r = nusdas_inq_subcinfo(dp->type.type1, dp->type.type2, dp->type.type3, 
			    &dp->base, dp->member, &dp->valid, 
			    query_nbytes, op->recname, &nbytes, siz);
    if(r < 0){
	    eprintf(HTTP_ERR, "nusdas_inq_subcinfo error(NBYTES): %d\n", r);	    
	    return r;
    }
    if((buf = (char*)malloc(nbytes)) == NULL){
	    eprintf(HTTP_ERR, "malloc failed: %s, %d\n", __FILE__, __LINE__);
	    return -10;
    }
	    

    r = nusdas_inq_subcinfo(dp->type.type1, dp->type.type2, dp->type.type3, 
			    &dp->base, dp->member, &dp->valid, 
			    query_content, op->recname, 
			    buf, nbytes);
    if(r < 0){
	    eprintf(HTTP_ERR, "nusdas_inq_subcinfo error(CONTENT): %d\n", r);	    
	    return r;
    }
    tprintf("X-SUBC_NBYTES", "%d\n", nbytes);

    twrite(buf, 1, nbytes);
    free(buf);

    return 0;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nussubcinfo_opts opt;
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


