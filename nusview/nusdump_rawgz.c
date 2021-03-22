#include "config.h"
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "nusdas.h"
#include "nusdim.h"
#include "textout.h"

struct nusdump_opt{
    N_SI4 sten[4];
    int gz_mode;
};

static void opt_clear(struct nusdump_opt *op){
    int ii;
    for(ii = 0; ii < 4; ii++){
        op->sten[ii] = -1;
    }
    op->gz_mode = 1;
}

static void option(struct nusdump_opt *op, const char *str)
{
    char *buf, *p;
    size_t len;
    char *p_sten[4];
    int cnt, ii;

    if (str == NULL)
        return;
    if (*str == '-')
        str++;
    switch (*str) {
    case 'r':
        op->gz_mode = 0;
        text_init("application/x_nusdas_rawdata");
        break;
    case 'R':
        str++;
        len = strlen(str);
        if((buf = (char*)malloc(len + 1)) == NULL){
            eprintf(HTTP_ERR, "malloc error, %d, %s\n", __FILE__, __LINE__);
            text_end();
            exit(1);
        }
        memcpy(buf, str, len);
        buf[len] = '\0';
        p_sten[0] = buf;
        p = buf;
        cnt = 1;
        while(*p){
            if(*p == '/'){
                *p = '\0';
                if(cnt == 4){
                    break;
                }
                p_sten[cnt++] = p + 1;
            }
            p++;
            
        }
        for(ii = 0; ii < 4; ii++){
            for(p = p_sten[ii]; *p; p++){
                if(*p != '-' && *p < '0' || *p > '9' ){
                    eprintf(HTTP_ERR, "Not digit character!");
                    free(buf);
                    text_end();
                    exit(1);
                }
            }
            op->sten[ii] = atoi(p_sten[ii]);
        }
        free(buf);
        break;
    default:
        break;
    }
}

static int get_data(struct nus_dims_t *dims, struct nusdump_opt *op)
{
    char pack[4];
    int gsize[2];
    int dnum;
    int alloc_size;
    int rt, ng, rawsize, cmpbufsize, cmpsize;
    unsigned char *buf, *cmpbuf;
    int ixst, ixen, jyst, jyen;

    dnum = 2;
    rt = nusdas_inq_data2(dims->type.type1, dims->type.type2, 
                           dims->type.type3, 
                           (N_SI4*)&dims->base, dims->member, 
                           (N_SI4*)&dims->valid, 
                           (N_SI4*)&dims->valid2, 
                           dims->plane, dims->plane2, 
                           dims->element, 
                           N_GRID_SIZE, gsize, (N_SI4*)&dnum);
    if(rt != dnum){
        eprintf(HTTP_ERR, "nusdas grid error(N_GRID_SIZE), rt = %d\n", rt);
        tprintf("X-Nusdas-Return-Code", "%d", rt);
        return -1;
    }
    ng = gsize[0] * gsize[1];
    dnum = 1;
    rt = nusdas_inq_data2(dims->type.type1, dims->type.type2, 
                           dims->type.type3, 
                           (N_SI4*)&dims->base, dims->member, 
                           (N_SI4*)&dims->valid, 
                           (N_SI4*)&dims->valid2, 
                           dims->plane, dims->plane2, 
                           dims->element, 
                           N_PC_PACKING, pack, (N_SI4*)&dnum);
    if(rt != dnum){
        eprintf(HTTP_ERR, "nusdas grid error(N_PC_PACKING), rt = %d\n", rt);
        tprintf("X-Nusdas-Return-Code", "%d", rt);
        return -1;
    }

    if(memcmp(pack, N_P_1PAC, 4) == 0 || memcmp(pack, N_P_I1, 4) == 0 ||
        memcmp(pack, N_P_RLEN, 4) == 0){
        /* 1byte case */
        alloc_size = ng + 100;
    }
    else if(memcmp(pack, N_P_2PAC, 4) == 0 || memcmp(pack, N_P_I2, 4) == 0
            || memcmp(pack, N_P_N1I2, 4) == 0 
            || memcmp(pack, N_P_2UPC, 4) == 0
            || memcmp(pack, N_P_2UPP, 4) == 0
            || memcmp(pack, N_P_2UPJ, 4) == 0){
        /* 2byte case */
        alloc_size = ng * 2  + 100;
    }
    else if(memcmp(pack, N_P_4PAC, 4) == 0 || memcmp(pack, N_P_R4, 4) == 0
        || memcmp(pack, N_P_I4, 4) == 0){
        /* 4byte case */
        alloc_size = ng * 4 + 100;
    }
    else if(memcmp(pack, N_P_R8, 4) == 0){
        /* 8byte case */
        alloc_size = ng * 8 + 100;
    }
    else{
        eprintf(HTTP_ERR, "Unknown packing parameter, %s\n", pack);
        return -2;
    }
    /* for mask */
    alloc_size += (ng - 1) / 8 + 1;
    if((buf = (unsigned char*)malloc(alloc_size)) == NULL){
        eprintf(HTTP_ERR, "malloc error, %s, %d\n", __FILE__, __LINE__);
        return -10;
    }
    dnum = alloc_size;
    if(op->sten[0] < 0 && op->sten[1] < 0 
       && op->sten[2] < 0 && op->sten[3] < 0){
        rt = nusdas_read2_raw(dims->type.type1, dims->type.type2, 
                              dims->type.type3, 
                              (N_SI4*)&dims->base, dims->member, 
                              (N_SI4*)&dims->valid, 
                              (N_SI4*)&dims->valid2, 
                              dims->plane, dims->plane2, 
                              dims->element, 
                              buf, (N_SI4*)&dnum);
        if(rt < 0){
            free(buf);
            eprintf(HTTP_ERR, "nusdas_read2_raw error, rt = %d\n", rt);
            tprintf("X-Nusdas-Return-Code", "%d", rt);
            return rt;
        }
    }
    else{
        rt = nusdas_cut2_raw(dims->type.type1, dims->type.type2, 
                             dims->type.type3, 
                             (N_SI4*)&dims->base, dims->member, 
                             (N_SI4*)&dims->valid, 
                             (N_SI4*)&dims->valid2, 
                             dims->plane, dims->plane2, 
                             dims->element, 
                             buf, (N_SI4*)&dnum, 
                             &(op->sten[0]), &(op->sten[1]), 
                             &(op->sten[2]), &(op->sten[3]));
        if(rt < 0){
            free(buf);
            eprintf(HTTP_ERR, "nusdas_cut2_raw error, rt = %d, %d, %d, %d, %d\n", rt, 
                    op->sten[0], op->sten[1], op->sten[2], op->sten[3]);
            tprintf("X-Nusdas-Return-Code", "%d", rt);
            return rt;
        }
    }
    
    rawsize = rt;

    if(op->sten[0] < 0){
        ixst = 1;
    }
    else{
        ixst = op->sten[0];
    }
    if(op->sten[1] < 0){
        ixen = gsize[0];
    }
    else{
        ixen = op->sten[1];
    }
    if(op->sten[2] < 0){
        jyst = 1;
    }
    else{
        jyst = op->sten[2];
    }
    if(op->sten[3] < 0){
        jyen = gsize[1];
    }
    else{
        jyen = op->sten[3];
    }

    tprintf("X-Data-Range", "%d,%d,%d,%d", ixst, ixen, jyst, jyen);
#ifdef USE_ZLIB
    if(op->gz_mode == 1){
        cmpbufsize = rawsize * 3;
        if((cmpbuf = (unsigned char*)malloc(cmpbufsize)) == NULL){
            eprintf(HTTP_ERR, "malloc error, %s, %d\n", __FILE__, __LINE__);
            free(buf);
            return -10;
        }
#if N_NUSDAS_VERSION >= 13
	rt = nusdas_gzip(buf, rawsize, cmpbuf, cmpbufsize);
#else
        rt = nusgz_compress(buf, rawsize, cmpbuf, cmpbufsize);
#endif
        if(rt < 0){
            eprintf(HTTP_ERR, "nusgz_compress error, rt = %d\n", rt);
            free(buf);
            free(cmpbuf);
            return rt;
        }
        cmpsize = rt;
        twrite(cmpbuf, cmpsize, 1);
        free(buf);
        free(cmpbuf);
        return cmpsize;
    }
    else{
        twrite(buf, rawsize, 1);
        free(buf);
        return rawsize;
    }
#else
    twrite(buf, rawsize, 1);
    free(buf);
    return rawsize;
#endif

}

    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
        struct nusdump_opt opt;
	int	argind;
	int	r;

	text_init("text/plain");
        opt_clear(&opt);
	if (argv[1] == NULL) {
		eprintf(HTTP_ERR, "usage: %s <path>\n\n", argv[0]);
		text_end();
		return 1;
	}
        
#ifdef USE_ZLIB
        text_init("application/x-nusdas_rawdata_gz");
#else
        text_init("application/x-nusdas_rawdata");
#endif
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
            r = get_data(&dims, &opt);
            if (r != 0) {
                r = 3;
                break;
            }
            r = 0;
	}
	text_end();
	return r;
}

