/* nusdump.c
 * 2001-08-31 TOYODA Eizi
 */

#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "nusdas.h"
#include "nusdim.h"
#include "textout.h"

#if 0
static char rcsid[] = "$Id: nusdump.c,v 1.7 2009-11-18 04:30:59 suuchi31 Exp $";
#endif

struct nusdump_opts {
	int (*handler)(struct grids_t *gp, struct nusdump_opts *op);
        int (*grid_getter)(struct nus_dims_t *dp, struct grids_t *gp);
	int	pnm;
        size_t  numdata;
        int sten[4];  /* 0: ixst, 1: ixen, 2: jyst, 3:jyen */
};

static
int set_sten(struct grids_t *gp, struct nusdump_opts *op, 
              int *ixst, int *ixen, int *jyst, int * jyen);

        int
header_x_missing(struct grids_t *gp)
{
  char fmt[80];

  if (gp->miss_mode != UDFV && gp->miss_mode != MASK)
    return 0;
  if (gp->ucbuf != NULL) {
    unsigned char missval = gp->ucmissval;
    strcpy(fmt, "%d");
    if (missval == UCHAR_MAX)
      strcat(fmt, " (UCHAR_MAX)");
    tprintf("X-missing-value", fmt, missval);
  } else if (gp->ibuf != NULL) {
    int missval = gp->imissval;
    strcpy(fmt, "%d");
    if (missval == INT_MAX)
      strcat(fmt, " (INT_MAX)");
    else if (missval == INT_MIN)
      strcat(fmt, " (INT_MIN)");
    tprintf("X-missing-value", fmt, missval);
  } else if (gp->fbuf != NULL) {
    float missval = gp->fmissval;
    strcpy(fmt, "%.7g");
    if (missval == FLT_MAX)
      strcat(fmt, " (FLT_MAX)");
    else if (missval == FLT_MIN)
      strcat(fmt, " (FLT_MIN)");
    tprintf("X-missing-value", fmt, missval);
  } else if (gp->dbuf != NULL) {
    double missval = gp->dmissval;
    strcpy(fmt, "%.16g");
    if (missval == DBL_MAX)
      strcat(fmt, " (DBL_MAX)");
    else if (missval == DBL_MIN)
      strcat(fmt, " (DBL_MIN)");
    tprintf("X-missing-value", fmt, missval);
  } else {
    return 0;
  }
  return 1;
}


	int
grids_dump_native(struct grids_t *gp, struct nusdump_opts *op)
{
	size_t		elemsiz;
	void		*buffer;
	if (gp->ibuf != NULL) {
		tset_content_type("application/x-int32-stream");
		elemsiz = sizeof(N_SI4);
		buffer = gp->ibuf;
	} else if (gp->fbuf != NULL) {
		tset_content_type("application/x-float32-stream");
		elemsiz = sizeof(float);
		buffer = gp->fbuf;
	} else if (gp->dbuf != NULL) {
		tset_content_type("application/x-float64-stream");
		elemsiz = sizeof(double);
		buffer = gp->dbuf;
	} else {
		eprintf(HTTP_ERR, "error: null pointer buffer");
		return -1;
	}
	header_x_missing(gp);
        if(op->sten[0] < 0 && op->sten[1] < 0 
           && op->sten[2] < 0 && op->sten[3] < 0){
            twrite(buffer, elemsiz, op->numdata);
        }
        else{
            int jy;
            int ixst, ixen, jyst, jyen;
            if(set_sten(gp, op, &ixst, &ixen, &jyst, &jyen) < 0){
                return -8;
            }
            for(jy = jyst - 1; jy < jyen; jy++){
                twrite((char*)buffer + elemsiz * (jy * gp->xysize[0] + ixst - 1), 
                       elemsiz, ixen - ixst + 1);
            }

        }
	return 0;
}

	int
grids_convert_float32(struct grids_t *gp)
{
	unsigned long siz, ofs;

	if (gp->fbuf != NULL)
		return 0;
	siz = (unsigned long)gp->xysize[0] * (unsigned long)gp->xysize[1];
	gp->fbuf = malloc(siz * sizeof(float));
	if (gp->ibuf != NULL) {
		tprintf("X-notice", "converted from int32 to float32");
		for (ofs = 0; ofs < siz; ofs++) {
			gp->fbuf[ofs] = gp->ibuf[ofs];
		}
		gp->fmissval = gp->imissval;
		free(gp->ibuf);
		gp->ibuf = NULL;
	} else if (gp->dbuf != NULL) {
		tprintf("X-notice", "converted from float64 to float32");
		for (ofs = 0; ofs < siz; ofs++) {
			gp->fbuf[ofs] = gp->dbuf[ofs];
		}
		gp->fmissval = gp->dmissval;
		free(gp->dbuf);
		gp->dbuf = NULL;
	} else {
		return -1;
	}
	return 0;
}

	int
grids_dump_float32(struct grids_t *gp, struct nusdump_opts *op)
{
	int	r;
        size_t     elemsiz;

        elemsiz = sizeof(float);
	r = grids_convert_float32(gp);
	if (r)
		return r;
	tset_content_type("application/x-float32-stream");
	header_x_missing(gp);
        if(op->sten[0] < 0 && op->sten[1] < 0 
           && op->sten[2] < 0 && op->sten[3] < 0){
            twrite(gp->fbuf, elemsiz, op->numdata);
        }
        else{
            int jy;
            int ixst, ixen, jyst, jyen;
            if(set_sten(gp, op, &ixst, &ixen, &jyst, &jyen) < 0){
                return -8;
            }
            for(jy = jyst - 1; jy < jyen; jy++){
                twrite(gp->fbuf + (jy * gp->xysize[0] + ixst - 1), 
                       elemsiz, ixen - ixst + 1);
            }

        }
	return 0;
}

int
grids_dump_uint8(struct grids_t *gp, struct nusdump_opts *op)
{
    size_t elemsiz;

    elemsiz = sizeof(char);

    if(gp->ucbuf == NULL){
        eprintf(HTTP_ERR, "error: null pointer buffer <ucbuf>");
        return -1;
    }
    tset_content_type("application/x-uint8-stream");
    header_x_missing(gp);
    if(op->sten[0] < 0 && op->sten[1] < 0 
       && op->sten[2] < 0 && op->sten[3] < 0){
        twrite(gp->ucbuf, elemsiz, op->numdata);
    }
    else{
        int jy;
        int ixst, ixen, jyst, jyen;
        if(set_sten(gp, op, &ixst, &ixen, &jyst, &jyen) < 0){
            return -8;
        }
        for(jy = jyst - 1; jy < jyen; jy++){
            twrite(gp->ucbuf + (jy * gp->xysize[0] + ixst - 1), 
                   elemsiz, ixen - ixst + 1);
        }
        
    }
    return 0;
}

int
grids_dump_rlen8(struct grids_t *gp,
		struct nusdump_opts *op __attribute__((unused)))
{
    unsigned long siz;
    int cmp_size;
    int maxv;
    unsigned char *tmp_alloc;
    int nbit = 8;

    struct sized_uchar{
        unsigned char *data;
        int alloc_size;
    } sd;

    if(gp->ucbuf == NULL){
        eprintf(HTTP_ERR, "error: null pointer buffer <ucbuf>");
        return -1;
    }
    siz = (unsigned long)gp->xysize[0] * (unsigned long)gp->xysize[1];
    tset_content_type("application/x-rlen8-stream");
    
    sd.alloc_size = 300000;
    sd.data = (unsigned char*)malloc(sd.alloc_size * sizeof(sd.data));
    if(sd.data == NULL){
        return -32;
    }

    maxv = -1;
    while(1){
        cmp_size = n_encode_rlen_8bit_I1(gp->ucbuf, sd.data, siz, 
                                      sd.alloc_size,&maxv);
        if(cmp_size == -1){
            sd.alloc_size += 100000;
            tmp_alloc = (unsigned char*)malloc(sd.alloc_size*sizeof(sd.data));
            if(tmp_alloc == NULL){
                free(sd.data);
                return -32;
            }
            else{
                sd.data = tmp_alloc;
            }
        }
        else if(cmp_size == -2){
            eprintf(HTTP_ERR, "error: some data is out of range ");
            free(sd.data);
            return -2;
        }
        else{
            break;
        }
    }
    twrite(&nbit, sizeof(int), 1);
    twrite(&maxv, sizeof(int), 1);
    twrite(&cmp_size, sizeof(int), 1);
    twrite(sd.data, sizeof(char), cmp_size);
    free(sd.data);
    return 0;
}

int
grids_dump_int32(struct grids_t *gp, struct nusdump_opts *op)
{
    size_t elemsiz;

    elemsiz = sizeof(int);
    if(gp->ibuf == NULL){
        eprintf(HTTP_ERR, "error: null pointer buffer <ibuf>");
        return -1;
    }
    tset_content_type("application/x-int32-stream");
    header_x_missing(gp);
    if(op->sten[0] < 0 && op->sten[1] < 0 
       && op->sten[2] < 0 && op->sten[3] < 0){
        twrite(gp->ibuf, elemsiz, op->numdata);
    }
    else{
        int jy;
        int ixst, ixen, jyst, jyen;
        if(set_sten(gp, op, &ixst, &ixen, &jyst, &jyen) < 0){
            return -8;
        }

        for(jy = jyst - 1; jy < jyen; jy++){
            twrite(gp->ibuf + (jy * gp->xysize[0] + ixst - 1), 
                   elemsiz, ixen - ixst + 1);
        }
        
    }
    return 0;
}

int
grids_dump_double64(struct grids_t *gp, struct nusdump_opts *op)
{
    size_t elemsiz;

    elemsiz = sizeof(double);

    if(gp->dbuf == NULL){
        eprintf(HTTP_ERR, "error: null pointer buffer <ibuf>");
        return -1;
    }
    tset_content_type("application/x-double64-stream");
    header_x_missing(gp);
    if(op->sten[0] < 0 && op->sten[1] < 0 
       && op->sten[2] < 0 && op->sten[3] < 0){
        twrite(gp->dbuf, elemsiz, op->numdata);
    }
    else{
        int jy;
        int ixst, ixen, jyst, jyen;
        if(set_sten(gp, op, &ixst, &ixen, &jyst, &jyen) < 0){
            return -8;
        }

        for(jy = jyst - 1; jy < jyen; jy++){
            twrite(gp->dbuf + (jy * gp->xysize[0] + ixst - 1), 
                   elemsiz, ixen - ixst + 1);
        }
        
    }
    return 0;
}



	int
grids_dump_pnm(struct grids_t *gp, struct nusdump_opts *op)
{
	int	r, miss_ari = (gp->miss_mode == UDFV || gp->miss_mode == MASK);
	unsigned long i, n;
	float	min, max, scale, miss;
	char *buffer;
        int ix, jy;
        int ixst, ixen, jyst, jyen;
        int cut_xysize[2];
        int index;

	r = grids_convert_float32(gp);
	if (r)
		return r;
	if ( gp->xysize[0] == 1 || gp->xysize[1] == 1) return -1;

        if(set_sten(gp, op, &ixst, &ixen, &jyst, &jyen) < 0){
            return -8;
        }

	max = -1.0e10f;
	min = 1.0e10f;
        for(jy = jyst - 1; jy < jyen; jy++){
            for(ix = ixst - 1; ix < ixen; ix++){
                i = jy * gp->xysize[0] + ix;
                if (miss_ari && gp->fmissval == gp->fbuf[i])
                    continue;
		if (gp->fbuf[i] < min) min = gp->fbuf[i];
		if (gp->fbuf[i] > max) max = gp->fbuf[i];
            }
	}
	tprintf("X-value-max", "%g", max);
	tprintf("X-value-min", "%g", min);

	if (miss_ari) {
	  miss = min - max;
          for(jy = jyst - 1; jy < jyen; jy++){
              for(ix = ixst - 1; ix < ixen; ix++){
                  i = jy * gp->xysize[0] + ix;
                  if (gp->fmissval == gp->fbuf[i])
                      gp->fbuf[i] = miss;
              }
          }
	  tprintf("X-missing-value", "%g", miss);
	  min = miss;
	}
	scale = max - min;
	if (scale == 0.0f)
		scale = 1.0f;
	else
		scale = scale / 255.0f;
	tprintf("X-gradation-step", "%g", scale);
        cut_xysize[0] = ixen - ixst + 1;
        cut_xysize[1] = jyen - jyst + 1;
        n = (ixen - ixst + 1) * (jyen - jyst + 1);

	if (op->pnm == 6) {
		tset_content_type("image/x-portable-pixmap; opt=binary");

		buffer = malloc(n * 3);
		tprintf(0, "P6\n%d %d\n255 255 255\n", cut_xysize[0], cut_xysize[1]);
                index = 0;
                for(jy = jyst - 1; jy < jyen; jy++){
                    for(ix = ixst - 1; ix < ixen; ix++){
                        i = jy * gp->xysize[0] + ix;

			buffer[index * 3 + 0] = (unsigned char)(256 - (gp->fbuf[i] - min) / scale);
			buffer[index * 3 + 1] = (unsigned char)((gp->fbuf[i] - min) / scale);
			buffer[index * 3 + 2] = (unsigned char)((gp->fbuf[i] - min) / scale);
                        index++;
                    }
		}
		twritemem(buffer, 1, n * 3);
	} else if (op->pnm == 5) {
		tset_content_type("image/x-portable-graymap");
		buffer = malloc(n);
		tprintf(0, "P5\n%d %d\n255\n", cut_xysize[0], cut_xysize[1]);
                index = 0;
                for(jy = jyst - 1; jy < jyen; jy++){
                    for(ix = ixst - 1; ix < ixen; ix++){
                        i = jy * gp->xysize[0] + ix;
			buffer[index] = (unsigned char)((gp->fbuf[i] - min) / scale);
                        index++;
                    }
		}
		twritemem(buffer, 1, n);
	} else {
		tset_content_type("image/x-portable-graymap; opt=text");

		tprintf(0, "P2\n%d %d\n255\n", cut_xysize[0], cut_xysize[1]);
                for(jy = jyst - 1; jy < jyen; jy++){
                    for(ix = ixst - 1; ix < ixen; ix++){
                        i = jy * gp->xysize[0] + ix;
			tprintf(0, "%d\n", (unsigned char)((gp->fbuf[i] - min) / scale));
                    }
		}
	}
	return 0;
}

	int
grids_dump_printf(struct grids_t *gp, struct nusdump_opts *op)
{
	int	i, j, ofs;
	size_t  cnt = 0;
        int ixst, ixen, jyst, jyen;

	tset_content_type("text/plain");

	header_x_missing(gp);

        if(set_sten(gp, op, &ixst, &ixen, &jyst, &jyen) < 0){
            return -8;
        }
        
	for (j = jyst - 1; j < jyen; j++) {
		for (i = ixst - 1; i < ixen; i++) {
			ofs = i + gp->xysize[0] * j;
			if (gp->ibuf) {
				tprintf(0, " %d ", gp->ibuf[ofs]);
			} else if (gp->fbuf) {
				tprintf(0, " %.7g", gp->fbuf[ofs]);
			} else if (gp->dbuf) {
				tprintf(0, " %.16g", gp->dbuf[ofs]);
			} else {
				return -1;
			}
			if (++cnt >= op->numdata) {
			  tprintf(0, "\n");
			  return 0;
			}
		}
		tprintf(0, "\n");
	}
	return 0;
}

	void
opt_clear(struct nusdump_opts *op)
{
        int ii;
	op->handler = grids_dump_native;
        op->grid_getter = nusdims_get_grids;
	op->pnm = 0;
        for(ii = 0; ii < 4; ii++){
            op->sten[ii] = -1;
        }

}

	void
option(struct nusdump_opts *op, const char *str)
{
        char *p_sten[4], *p;
        int cnt, len, ii;
        char *buf;

	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
	case 'r':
		if (str[1]) {
			nusdas_addroot(str+1);
		}
		break;
	case 't':
		if (str[1] == 'f') {
			op->handler = grids_dump_float32;
		} else if (str[1] == 'p') {
			op->handler = grids_dump_printf;
		} else if (str[1] == 'u') { /* unsigned char */
                    op->handler = grids_dump_uint8;
                    op->grid_getter = nusdims_get_grids_uint8;
                } else if (str[1] == 'i') { /* int */
                    op->handler = grids_dump_int32;
                    op->grid_getter = nusdims_get_grids_int32;
                } else if (str[1] == 'r') { /* rlen 8bit */
                    op->handler = grids_dump_rlen8;
                    op->grid_getter = nusdims_get_grids_uint8;
                } else if (str[1] == 'd'){ /* double */
                    op->handler = grids_dump_double64;
                    op->grid_getter = nusdims_get_grids_double64;
                }
		break;
	case 'p':
	case 'P':
		if (str[1] == 'G') {
			op->handler = grids_dump_pnm;
			op->pnm = 2;
		} else if (str[1] == 'g') {
			op->handler = grids_dump_pnm;
			op->pnm = 5;
		} else if (str[1] == 'p') {
			op->handler = grids_dump_pnm;
			op->pnm = 6;
		}
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
                    if (*p != '-' && (*p < '0' || *p > '9')) {
                        eprintf(HTTP_ERR, "Not digit character!");
                        free(buf);
                        text_end();
                        exit(1);
                    }
                }
                op->sten[ii] = atoi(p_sten[ii]);
            }
            free(buf);
            if(op->sten[0] * op->sten[1] < 0 
               || op->sten[2] * op->sten[3] < 0){
                eprintf(HTTP_ERR, "nusdump range error!");                
                text_end();
                exit(1);
            }
            break;
	default:
		break;
	}
}

static
int set_sten(struct grids_t *gp, struct nusdump_opts *op, 
              int *ixst, int *ixen, int *jyst, int * jyen)
{
    int itmp;

    if(op->sten[0] > gp->xysize[0] || op->sten[1] > gp->xysize[0]
       || op->sten[2] > gp->xysize[1] || op->sten[3] > gp->xysize[1]
       || op->sten[0] == 0 || op->sten[1] == 0 
       || op->sten[2] == 0 || op->sten[3] == 0){
        eprintf(HTTP_ERR, "Invalid Argument for range to cut, %d, %d, %d, %d\n", 
                op->sten[0], op->sten[1], op->sten[2], op->sten[3]);
        return -8;
    }

    if(op->sten[0] < 0){
        *ixst = 1;
    }
    else{
        *ixst = op->sten[0];
    }
    if(op->sten[1] < 0){
        *ixen = gp->xysize[0];
    }
    else{
        *ixen = op->sten[1];
    }
    if(op->sten[2] < 0){
        *jyst = 1;
    }
    else{
        *jyst = op->sten[2];
    }
    if(op->sten[3] < 0){
        *jyen = gp->xysize[1];
    }
    else{
        *jyen = op->sten[3];
    }

    if(*ixen < *ixst){
        itmp = *ixst;
        *ixst = *ixen;
        *ixen = itmp;
    }
    if(*jyen < *jyst){
        itmp = *jyst;
        *jyst = *jyen;
        *jyen = itmp;
    }
    return 0;

}

/* dp で指定される NuSDaS 面をダンプする */

	int
nusdims_dump(struct nus_dims_t *dp, struct nusdump_opts *op)
{
	struct grids_t g;
	int	r;
        int ixst, ixen, jyst, jyen, datanum;

	g.ucbuf = NULL, g.ibuf = NULL, g.fbuf = NULL, g.dbuf = NULL;
	r = op->grid_getter(dp, &g);
	if (r < -2)
		eprintf(HTTP_ERR, "nusdims_get_grids %d\n", r);
	else
	  switch (r) {
	  case -2:
	    eprintf(HTTP_RESP_Not_Found,
		    "Requested data is NOT REGISTERED.\n");
	    break;
	  case -1:
	    eprintf(HTTP_RESP_Not_Found,
		    "Requested data is NOT REGISTERED or NOT RECORDED YET.\n");
	    break;
	  case 0:
	    eprintf(HTTP_RESP_Not_Found,
		    "Requested data is NOT RECORDED YET.\n");
	    r = -1;
	    break;
	  }
        tprintf("X-Nusdas-Return-Code", "%d", r);
	if (r < 0)
		return r;
        
	op->numdata = r;
        if(set_sten(&g, op, &ixst, &ixen, &jyst, &jyen)){
            return -8;
        }
        if(op->sten[0] < 0 && op->sten[1] < 0 &&
           op->sten[2] < 0 && op->sten[3] < 0){
            tprintf("X-Data-Num", "%d", r);
        }
        else{
            datanum = (ixen - ixst + 1) * (jyen - jyst + 1);
            tprintf("X-Data-Num", "%d", datanum);
        }
        tprintf("X-Data-Range", "%d,%d,%d,%d", ixst, ixen, jyst, jyen);
	r = op->handler(&g, op);
	if (r < 0)
		eprintf(HTTP_ERR, "nusdims_dump %d\n", r);
	return r;
}

    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nusdump_opts opt;
	int	argind;
	int	r;

	text_init("text/plain");
	if (argc < 2) {
		eprintf(HTTP_ERR, "usage: %s <path>\n\n", argv[0]);
		text_end();
		return 1;
	}
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
		r = nusdims_dump(&dims, &opt);
		if (r != 0) {
			r = 3;
			break;
		}
		r = 0;
	}
	text_end();
	return r;
}
