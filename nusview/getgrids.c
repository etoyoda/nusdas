/* nuspnm.c
 * 2001-07-04 TOYODA Eizi
 */

#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <nusdas.h>
#include "nusdim.h"
#include "textout.h"
#include "nwpl_mapproj_df.h"

#if 0
static char rcsid[] = "$Id: getgrids.c,v 1.3 2007-06-22 07:11:42 suuchi04 Exp $";
#endif

int
get_missing_info(struct nus_dims_t *dp,
		const char *utype __attribute__((unused)),
		struct grids_t *gp)
{
        N_SI4	n;
        int r;
	char    miss_mode[5];

	n = 1;
	strncpy(miss_mode, "    ", 4);
	r = nusdas_inq_data(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		N_MISSING_MODE, miss_mode, &n);
	if (r <= 0) {
		eprintf(HTTP_ERR, "nusdas_inq_data(N_MISSING_MODE): %d\n", r);
		return r;
	}
	miss_mode[4] = '\0';
	tprintf("X-missing-mode", "%s", miss_mode);

	if      (strcmp(miss_mode, "NONE") == 0)
	  gp->miss_mode = NONE;
	else if (strcmp(miss_mode, "UDFV") == 0)
	  gp->miss_mode = UDFV;
	else if (strcmp(miss_mode, "MASK") == 0)
	  gp->miss_mode = MASK;
	else
	  gp->miss_mode = ERROR;

	gp->ucmissval = N_MV_UI1;
	gp->imissval  = N_MV_SI4;
	gp->fmissval  = N_MV_R4;
	gp->dmissval  = N_MV_R8;

#if 0
	if (strcmp(miss_mode, "UDFV") == 0) {
	  void *ptr;

	if      (gp->ucbuf != NULL) ptr = &(gp->ucmissval);
	else if (gp->ibuf  != NULL) ptr = &(gp->imissval);
	else if (gp->fbuf  != NULL) ptr = &(gp->fmissval);
	else if (gp->dbuf  != NULL) ptr = &(gp->dmissval);
	  else {
	    eprintf(HTTP_ERR, "get_missing_info: invalid utype: %s\n", utype);
	    return -3;
	  }
  
	  n = 1;
	  r = nusdas_inq_data(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		N_MISSING_VALUE, ptr, &n);
	  if (r <= 0) {
	    eprintf(HTTP_ERR, "nusdas_inq_data(N_MISSING_VALUE): %d\n", r);
	    return r;
	  }
	}
#endif
	return r;
}

/* dp で指定される NuSDaS 面をゲット */

	int
nusdims_get_grids(struct nus_dims_t *dp, struct grids_t *gp)
{
	N_SI4	n;
	int	i, j, r;

#define NUSDAS_ENOMEM -32

	gp->xysize[0] = 0;
	gp->xysize[1] = 0;
	gp->ibuf = NULL, gp->fbuf = NULL, gp->dbuf = NULL;

        if(memcmp(dp->element, "LAT   ", 6) == 0 
           || memcmp(dp->element, "LON   ", 6) == 0){
            char *elemlist;
            N_SI4 nelem;

            n = 1;
            r = nusdas_inq_cntl(
                dp->type.type1, dp->type.type2, dp->type.type3, 
                &(dp->base), dp->member, &(dp->valid), N_ELEMENT_NUM, 
                &nelem, &n);
            if(r != n){
                eprintf(HTTP_ERR, "nusdas_inq_cntl error(N_ELEMENT_NUM)\n, %d",
                        r);
                return r;
            }
            elemlist = (char*)malloc(nelem * 6);
            n = nelem;
            r = nusdas_inq_cntl(
                dp->type.type1, dp->type.type2, dp->type.type3, 
                &(dp->base), dp->member, &(dp->valid), N_ELEMENT_LIST, 
                elemlist, &n);
            if(r !=  n){
                free(elemlist);
                eprintf(HTTP_ERR, "nusdas_inq_cntl error(N_ELEMENT_LIST)\n, %d",
                        r);
                return r;
            }
            for(i = 0; i < nelem; i++){
                if(memcmp(elemlist + i * 6, dp->element, 6) == 0){
                    break;
                }
            }
            free(elemlist);
            if(i == nelem){
                char proj[4], mean[4];
                N_SI4 gsize[2];
                float ginfo[14];
                float *x, *y, *lat, *lon, slat1, slat2;
                r = nusdas_grid(
                    dp->type.type1, dp->type.type2, dp->type.type3, 
                    &(dp->base), dp->member, &(dp->valid), proj, gsize, 
                    ginfo, mean, N_IO_GET);
                if(r != 0){
                    eprintf(HTTP_ERR, "nusdas_grid error\n, %d", r);
                    return r;
                }
                n = gsize[0] * gsize[1];
                gp->xysize[0] = gsize[0];
                gp->xysize[1] = gsize[1];
                x = (float*)malloc(n * sizeof(float));
                y = (float*)malloc(n * sizeof(float));
                lat = (float*)malloc(n * sizeof(float));
                lon = (float*)malloc(n * sizeof(float));
                r = gsize[0] * gsize[1];
                if(x == NULL || y == NULL || lat == NULL || lon == NULL){
                    r = NUSDAS_ENOMEM;
                    goto End;
                }
                for(j = 0; j < gsize[1]; j++){
                    for(i = 0; i < gsize[0]; i++){
                        x[j * gsize[0] + i] = (float)(i + 1);
                        y[j * gsize[0] + i] = (float)(j + 1);
                    }
                }
                if(memcmp(proj, "LM", 2) == 0){
                    if(abs(ginfo[4] - ginfo[5]) > 1.e-3){
                        eprintf(HTTP_ERR, "distx != disty, %f, %f\n", 
                                ginfo[4], ginfo[5]);
                        r = -1;
                        goto End;
                    }
                    if(abs(ginfo[7] - ginfo[9]) > 1.e-3){
                        eprintf(HTTP_ERR, "lon1 != lon2, %f, %f\n", 
                                ginfo[7], ginfo[9]);
                        r = -1;
                    }
                    if(abs(ginfo[6]) < abs(ginfo[8])){
                        slat1 = ginfo[6];
                        slat2 = ginfo[8];
                    }
                    else{
                        slat1 = ginfo[8];
                        slat2 = ginfo[6];
                    }
                    NWP_lambert2sphere_F(x, y, n, slat1, slat2, ginfo[7], 
                                         ginfo[2], ginfo[3], 
                                         ginfo[0], ginfo[1], ginfo[4],
                                         lat, lon);
                }
                else if(memcmp(proj, "ME", 2) == 0){
                    if(abs(ginfo[4] - ginfo[5]) > 1.e-3){
                        eprintf(HTTP_ERR, "distx != disty, %f, %f\n", 
                                ginfo[4], ginfo[5]);
                        r = -1;
                        goto End;
                    }
                    NWP_mercator2sphere_F(x, y, n, ginfo[6], 
                                          ginfo[2], ginfo[3], 
                                          ginfo[0], ginfo[1],
                                          ginfo[4], lat, lon);
                }
                else if(memcmp(proj, "PS", 2) == 0){
                    if(abs(ginfo[4] - ginfo[5]) > 1.e-3){
                        eprintf(HTTP_ERR, "distx != disty, %f, %f\n", 
                                ginfo[4], ginfo[5]);
                        r = -1;
                        goto End;
                    }
                    NWP_polar2sphere_F(x, y, n, ginfo[6], ginfo[7], 
                                          ginfo[2], ginfo[3], 
                                          ginfo[0], ginfo[1],
                                          ginfo[4], lat, lon);
                }
                else if(memcmp(proj, "LL", 2) == 0){
                    for(i = 0; i < n; i++){
                        lon[i] = ginfo[3] + (x[i] - ginfo[0]) * ginfo[4];
                        if (lon[i] > 180.0) {
                            lon[i] -= 360.0;
                        }
                        lat[i] = ginfo[2] + (ginfo[1] - y[i]) * ginfo[5];
                    }
                }
                else{
                    eprintf(HTTP_ERR, "%4.4s is not supported.\n", proj);
                    r = -1;
                    goto End;
                }
                if ((gp->fbuf = malloc(n * sizeof(float))) == NULL)
                        return NUSDAS_ENOMEM;
                if(memcmp(dp->element, "LAT   ", 6) == 0){
                    memcpy(gp->fbuf, lat, n * sizeof(float));
                }
                else if(memcmp(dp->element, "LON   ", 6) == 0){
                    memcpy(gp->fbuf, lon, n * sizeof(float));
                }
            End:
                if(x != NULL) free(x);
                if(y != NULL) free(y);
                if(lat != NULL) free(lat);
                if(lon != NULL) free(lon);
                return r;
            }
        }


	n = 2;
	r = nusdas_inq_data(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		N_GRID_SIZE, gp->xysize, &n);
	if (r <= 0) {
		eprintf(HTTP_ERR, "nusdas_inq_data(N_GRID_SIZE): %d\n", r);
		return r;
	}
	n = gp->xysize[0] * gp->xysize[1];


/* try_float32: */
	/* 4バイト実数で読んでみる */
	if ((gp->fbuf = malloc(n * sizeof(float))) == NULL)
		return NUSDAS_ENOMEM;
	memset(gp->fbuf, 0, n * sizeof(float));
	r = nusdas_read(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		gp->fbuf, N_R4, &n);
	if (r == -5)
		goto try_int32;
	else if (r < -2)
		eprintf(HTTP_ERR, "nusdas_read.float32 %d\n", r);

	if ((i = get_missing_info(dp, N_R4, gp)) < 0)
	  return i;

	return r;

try_int32:
	/* 4バイト整数で読んでみる */
	gp->ibuf = (int *)gp->fbuf;
	gp->fbuf = NULL;
	r = nusdas_read(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		gp->ibuf, N_I4, &n);
	if (r == -5)
		goto try_float64;
	if (r <= 0)
		eprintf(HTTP_ERR, "nusdas_read.int32 %d\n", r);

	if ((i = get_missing_info(dp, N_I4, gp)) < 0)
	  return i;

	return r;

try_float64:
	/* 8バイト実数で読んでみる */
	free(gp->ibuf);
	if ((gp->dbuf = malloc(n * sizeof(double))) == NULL)
		return NUSDAS_ENOMEM;
	memset(gp->dbuf, 0, n * sizeof(double));
	r = nusdas_read(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		gp->dbuf, N_R8, &n);
	if (r <= 0) {
		eprintf(HTTP_ERR, "nusdas_read.float64 %d\n", r);
		free(gp->dbuf);
		return r;
	}

	if ((i = get_missing_info(dp, N_R8, gp)) < 0)
	  return i;

	return r;
}

	int
nusdims_get_grids_uint8(struct nus_dims_t *dp, struct grids_t *gp)
{
	N_SI4	n;
	int	i, r;

	gp->xysize[0] = 0;
	gp->xysize[1] = 0;
	gp->ucbuf = NULL, gp->ibuf = NULL, gp->fbuf = NULL, gp->dbuf = NULL;

	n = 2;
        r = nusdas_inq_data2(dp->type.type1, dp->type.type2, dp->type.type3,
                             &(dp->base), dp->member, &(dp->valid), &(dp->valid2), 
                             dp->plane, dp->plane2, dp->element, N_GRID_SIZE, 
                             gp->xysize, &n);
	if (r < 0) {
		eprintf(HTTP_ERR, "nusdas_inq_data: %d\n", r);
		return r;
	}
	n = gp->xysize[0] * gp->xysize[1];

#define NUSDAS_ENOMEM -32

	if ((gp->ucbuf = malloc(n * sizeof(char))) == NULL)
		return NUSDAS_ENOMEM;
	memset(gp->ucbuf, 0, n * sizeof(char));
	r = nusdas_read(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		gp->ucbuf, N_I1, &n);
	if (r <= 0)
		eprintf(HTTP_ERR, "nusdas_read.I1 %d\n", r);

	if ((i = get_missing_info(dp, N_I1, gp)) < 0)
	  return i;

	return r;
}
/*--------------------------------------------------------------------*/
	int
nusdims_get_grids_int32(struct nus_dims_t *dp, struct grids_t *gp)
{
	N_SI4	n;
	int	i, r;

	gp->xysize[0] = 0;
	gp->xysize[1] = 0;
	gp->ucbuf = NULL, gp->ibuf = NULL, gp->fbuf = NULL, gp->dbuf = NULL;

	n = 2;
        r = nusdas_inq_data2(dp->type.type1, dp->type.type2, dp->type.type3,
                             &(dp->base), dp->member, &(dp->valid), &(dp->valid2), 
                             dp->plane, dp->plane2, dp->element, N_GRID_SIZE, 
                             gp->xysize, &n);
	if (r < 0) {
		eprintf(HTTP_ERR, "nusdas_inq_data: %d\n", r);
		return r;
	}
	n = gp->xysize[0] * gp->xysize[1];

#define NUSDAS_ENOMEM -32

	if ((gp->ibuf = malloc(n * sizeof(int))) == NULL)
		return NUSDAS_ENOMEM;
	memset(gp->ibuf, 0, n * sizeof(int));
	r = nusdas_read(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		gp->ibuf, N_I4, &n);
	if (r <= 0)
		eprintf(HTTP_ERR, "nusdas_read.I4 %d\n", r);


	if ((i = get_missing_info(dp, N_I4, gp)) < 0)
	  return i;

	return r;
}
/*-----------------------------------------------------------------------*/
	int
nusdims_get_grids_double64(struct nus_dims_t *dp, struct grids_t *gp)
{
	N_SI4	n;
	int	i, r;

	gp->xysize[0] = 0;
	gp->xysize[1] = 0;
	gp->ucbuf = NULL, gp->ibuf = NULL, gp->fbuf = NULL, gp->dbuf = NULL;

	n = 2;
        r = nusdas_inq_data2(dp->type.type1, dp->type.type2, dp->type.type3,
                             &(dp->base), dp->member, &(dp->valid), &(dp->valid2), 
                             dp->plane, dp->plane2, dp->element, N_GRID_SIZE, 
                             gp->xysize, &n);
	if (r < 0) {
		eprintf(HTTP_ERR, "nusdas_inq_data: %d\n", r);
		return r;
	}
	n = gp->xysize[0] * gp->xysize[1];

#define NUSDAS_ENOMEM -32

	if ((gp->dbuf = malloc(n * sizeof(double))) == NULL)
		return NUSDAS_ENOMEM;
	memset(gp->dbuf, 0, n * sizeof(double));
	r = nusdas_read(dp->type.type1, dp->type.type2, dp->type.type3,
		&(dp->base), dp->member, &(dp->valid), dp->plane, dp->element,
		gp->dbuf, N_R8, &n);
	if (r <= 0)
		eprintf(HTTP_ERR, "nusdas_read.R8 %d\n", r);

	if ((i = get_missing_info(dp, N_R8, gp)) < 0)
	  return i;
	return r;
}
/*-------------------------------------------------------------------------*/
