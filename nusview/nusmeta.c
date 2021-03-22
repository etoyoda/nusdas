/* nusmeta.c */
/* 2003/5/22 tabito added */
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <strings.h>
#include <limits.h>
#include "stringplus.h"
#include <nusdas.h>
#include "textout.h"
#include "nusdim.h"
#include "nwpl_capi.h"


struct nusmeta_opts{
    int (*operator)(struct nus_dims_t *dp, struct nusmeta_opts *op);
    int put_title;
    int is_radar;
    int sten[4];  /* 0: ixst, 1: ixen, 2: jyst, 3:jyen */    
};

int nusdims_meta(struct nus_dims_t *dp, struct nusmeta_opts *op);
int nusdims_meta_rd(struct nus_dims_t *dp, struct nusmeta_opts *op);
int nusdims_meta_tsv(struct nus_dims_t *dp, struct nusmeta_opts *op);
int nusdims_meta_bin(struct nus_dims_t *dp, struct nusmeta_opts *op);

static
int get_meta(struct nus_dims_t *dp, struct nusmeta_opts *op, 
             char *proj, N_SI4 *gridsize, float *grid, 
             char *pvalp, float *f_idx);

void
check_radar(char *proj, struct nus_dims_t *dp, N_SI4 *gridsize, float grid[][2]);


	void
opt_clear(struct nusmeta_opts *op)
{
    int ii;

    op->operator =nusdims_meta; 
    op->put_title = 0;
    for(ii = 0; ii < 4; ii++){
        op->sten[ii] = -1;
    }
}

	void
option(struct nusmeta_opts *op, const char *str)
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
            nusdas_addroot(str + 1);
            break;
	case 't':
            if (str[1] == '\0') {
                tset_content_type("text/plain");
            } 
            else if (str[1] == 'r') {
                tset_content_type("text/x-rd");
                op->operator =nusdims_meta_rd;  
            }
            else if (str[1] == 't') {
                tset_content_type("text/tab-separated-values");
                op->operator =nusdims_meta_tsv;  
            }
            break;
        case 'b':
            op->operator = nusdims_meta_bin;
            break;
        case 'l':
            op->put_title = 1;
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
            for (ii = 0; ii < 4; ii++){
                for (p = p_sten[ii]; *p; p++){
                    if (*p != '-' && (*p < '0' || *p > '9' )) {
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
int get_meta(struct nus_dims_t *dp, struct nusmeta_opts *op, 
             char *proj, N_SI4 *gridsize, float *grid0, 
             char *pvalp, float *f_idx)
{
    int	r, n = 2;
    int sten[4];
    float grid[7][2];
    int i, j, itmp;

    f_idx[0] = 1.0;
    f_idx[1] = 1.0;
    
    r = nusdas_grid2(dp->type.type1, dp->type.type2, dp->type.type3,
                     &dp->base, dp->member, &dp->valid, &dp->valid2,
                     proj, gridsize, grid0, pvalp, N_IO_GET);
    if (r < 0) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_grid2] %d\n", r);
        return r;
    }
    if(memcmp(dp->plane, "none  ", 6) != 0 && memcmp(dp->plane2, "none  ", 6) != 0
       && memcmp(dp->element, "none__", 6) != 0){
        r = nusdas_inq_data2(dp->type.type1, dp->type.type2, dp->type.type3,
                             &dp->base, dp->member, &dp->valid, &dp->valid2,
                             dp->plane, dp->plane2, dp->element,
                             N_GRID_SIZE, gridsize, (N_SI4*)&n);
        if (r < -2) {
            eprintf(HTTP_ERR, "nusdas error [nusdas_inq_data2] %d\n", r);
            return r;
        } else
            switch(r) {
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
                break;
            }
    }

    for(i = 0; i < 7; i++){
        for(j = 0; j < 2; j++){
            grid[i][j] = grid0[i * 2 + j];
        }
    }
    check_radar(proj, dp, gridsize, grid);

    if(op->sten[0] > gridsize[0] || op->sten[1] > gridsize[0]
       || op->sten[2] > gridsize[1] || op->sten[3] > gridsize[1]
       || op->sten[0] == 0 || op->sten[1] == 0 
       || op->sten[2] == 0 || op->sten[3] == 0){
        eprintf(HTTP_ERR, "Invalid region specified. %d, %d, %d, %d\n", 
                op->sten[0], op->sten[1], op->sten[2], op->sten[3]);
        return -8;
    }

    if(op->sten[0] < 0){
        sten[0] = 1;
    }
    else{
        sten[0] = op->sten[0];
    }
    if(op->sten[1] < 0){
        sten[1] = gridsize[0];
    }
    else{
        sten[1] = op->sten[1];
    }
    if(op->sten[2] < 0){
        sten[2] = 1;
    }
    else{
        sten[2] = op->sten[2];
    }
    if(op->sten[3] < 0){
        sten[3] = gridsize[1];
    }
    else{
        sten[3] = op->sten[3];
    }
    if(sten[1] < sten[0]){
        itmp = sten[0];
        sten[0] = sten[1];
        sten[1] = itmp;
    }
    if(sten[3] < sten[2]){
        itmp = sten[2];
        sten[2] = sten[3];
        sten[3] = itmp;
    }

    gridsize[0] = sten[1] - sten[0] + 1;
    gridsize[1] = sten[3] - sten[2] + 1;
    
    grid0[0] = grid0[0] - (float)(sten[0] - 1);
    grid0[1] = grid0[1] - (float)(sten[2] - 1);

    return 0;

}

	int
nusdims_meta(struct nus_dims_t *dp, struct nusmeta_opts *op)
{
	int	r;
	char	proj[4], pvalp[4];
	N_SI4	gridsize[2];
	float	grid[7][2], grid0[14];
	float   f_idx[2];
	int     i, j;

        r = get_meta(dp, op, proj, gridsize, grid0, pvalp, f_idx);
        if(r < 0){
            return r;
        }
        for(i = 0; i < 7; i++){
            for(j = 0; j < 2; j++){
                grid[i][j] = grid0[i * 2 + j];
            }
        }

	tagopen("html", NULL);
	tagopen("head", NULL);
	tagopen("title", NULL);
	tprintf(NULL, "meta for %s", nusdims_to_path(dp));
	tagclose("title");	
	tagclose("head");
	tputs("\r\n", NULL);

	tagopen("body", NULL);
	tagopen("table", "border", NULL);

#define TABLE_ROW(cname, desc, format, value) \
	tagopen("tr", NULL); \
	tagopen("tr", NULL); \
	tagopen("td", "name", cname, NULL); \
	tputs(desc, NULL); \
	tagclose("td"); \
	tagopen("td", NULL); \
	tprintf(NULL, format, value); \
	tagclose("td"); \
	tagclose("tr"); tputs("\r\n", NULL);

	TABLE_ROW("proj_type", "projection type\t", "%4.4s", proj);
	TABLE_ROW("xs", "number of x grids\t", "%d", gridsize[0]);
	TABLE_ROW("ys", "number of y grids\t", "%d", gridsize[1]);
	TABLE_ROW("x0", "base point x\t", "%g", grid[0][0]);
	TABLE_ROW("y0", "base point y\t", "%g", grid[0][1]);
	TABLE_ROW("lat_0", "base point lat\t", "%.5f", grid[1][0]);
	TABLE_ROW("lon_0", "base point lon\t", "%.5f", grid[1][1]);
	TABLE_ROW("xd", "grid interval x\t", "%g", grid[2][0]);
	TABLE_ROW("yd", "grid interval y\t", "%g", grid[2][1]);
	TABLE_ROW("lat_b1", "standard lat 1\t", "%.5f", grid[3][0]);
	TABLE_ROW("lon_b1", "standard lon 1\t", "%.5f", grid[3][1]);
	TABLE_ROW("lat_b2", "standard lat 2\t", "%.5f", grid[4][0]);
	TABLE_ROW("lon_b2", "standard lon 2\t", "%.5f", grid[4][1]);
	TABLE_ROW("lat_1", "latitude 1\t", "%.5f", grid[5][0]);
	TABLE_ROW("lon_1", "longitude 1\t", "%.5f", grid[5][1]);
	TABLE_ROW("lat_2", "latitude 2\t", "%.5f", grid[6][0]);
	TABLE_ROW("lon_2", "longitude 2\t", "%.5f", grid[6][1]);
	TABLE_ROW("pval", "representation\t", "%4.4s", pvalp);
	TABLE_ROW("first_x", "first index x\t", "%g", f_idx[0]);
	TABLE_ROW("first_y", "first index y\t", "%g", f_idx[1]);

	tagclose("table");
	tagclose("body");
	tagclose("html");
	tputs("\r\n", NULL);
	return 0;
}

/*-------------------------------------------------------------------------*/
	int
nusdims_meta_rd(struct nus_dims_t *dp, struct nusmeta_opts *op)
{
	int	r;
	char	proj[4], pvalp[4];
	N_SI4	gridsize[2];
	float	grid[7][2], grid0[14];
	float   f_idx[2];
	int     i, j;

        r = get_meta(dp, op, proj, gridsize, grid0, pvalp, f_idx);
        if(r < 0){
            return r;
        }
        for(i = 0; i < 7; i++){
            for(j = 0; j < 2; j++){
                grid[i][j] = grid0[i * 2 + j];
            }
        }

        tprintf(0,"=begin\n");
        if(op->put_title == 1){
            tprintf(0,"=meta for %s\n", nusdims_to_path(dp));
        }
        tprintf(0,"==grid info\n");
        tprintf(0,"=end\n");
        tprintf(0,"=begin RT\n");
	if (strncmp(proj, "    ", 4) == 0)
		strcpy(proj, "????");
	tprintf(0, "projection type, %4.4s\n", proj);
	tprintf(0, "number of x grids, %d\n", gridsize[0]);
	tprintf(0, "number of y grids, %d\n", gridsize[1]);
	tprintf(0, "base point x, %g\n", grid[0][0]);
	tprintf(0, "base point y, %g\n", grid[0][1]);
	tprintf(0, "base point lat, %.5f\n", grid[1][0]);
	tprintf(0, "base point lon, %.5f\n", grid[1][1]);
	tprintf(0, "grid interval x, %g\n", grid[2][0]);
	tprintf(0, "grid interval y, %g\n", grid[2][1]);
	tprintf(0, "standard lat 1, %.5f\n", grid[3][0]);
	tprintf(0, "standard lon 1, %.5f\n", grid[3][1]);
	tprintf(0, "standard lat 2, %.5f\n", grid[4][0]);
	tprintf(0, "standard lon 2, %.5f\n", grid[4][1]);
	tprintf(0, "latitude 1, %.5f\n", grid[5][0]);
	tprintf(0, "longitude 1, %.5f\n", grid[5][1]);
	tprintf(0, "latitude 2, %.5f\n", grid[6][0]);
	tprintf(0, "longitude 2, %.5f\n", grid[6][1]);
	if (strncmp(pvalp, "    ", 4) == 0)
		strcpy(pvalp, "????");
	tprintf(0, "representation, %4.4s\n", pvalp);
	tprintf(0, "first index x, %g\n", f_idx[0]);
	tprintf(0, "first index y, %g\n", f_idx[1]);
        tprintf(0,"=end RT\n");

	return 0;
}
/*-------------------------------------------------------------------------*/
	int
nusdims_meta_tsv(struct nus_dims_t *dp, struct nusmeta_opts *op)
{
	int	r;
	char	proj[4], pvalp[4];
	N_SI4	gridsize[2];
	float	grid[7][2], grid0[14];
	float   f_idx[2];
	int     i, j;

        r = get_meta(dp, op, proj, gridsize, grid0, pvalp, f_idx);
        if(r < 0){
            return r;
        }
        for(i = 0; i < 7; i++){
            for(j = 0; j < 2; j++){
                grid[i][j] = grid0[i * 2 + j];
            }
        }

	if (strncmp(proj, "    ", 4) == 0)
		strcpy(proj, "????");
	tprintf(0, "projection type\t%4.4s\n", proj);
	tprintf(0, "number of x grids\t%d\n", gridsize[0]);
	tprintf(0, "number of y grids\t%d\n", gridsize[1]);
	tprintf(0, "base point x\t%g\n", grid[0][0]);
	tprintf(0, "base point y\t%g\n", grid[0][1]);
	tprintf(0, "base point lat\t%.5f\n", grid[1][0]);
	tprintf(0, "base point lon\t%.5f\n", grid[1][1]);
	tprintf(0, "grid interval x\t%g\n", grid[2][0]);
	tprintf(0, "grid interval y\t%g\n", grid[2][1]);
	tprintf(0, "standard lat 1\t%.5f\n", grid[3][0]);
	tprintf(0, "standard lon 1\t%.5f\n", grid[3][1]);
	tprintf(0, "standard lat 2\t%.5f\n", grid[4][0]);
	tprintf(0, "standard lon 2\t%.5f\n", grid[4][1]);
	tprintf(0, "latitude 1\t%.5f\n", grid[5][0]);
	tprintf(0, "longitude 1\t%.5f\n", grid[5][1]);
	tprintf(0, "latitude 2\t%.5f\n", grid[6][0]);
	tprintf(0, "longitude 2\t%.5f\n", grid[6][1]);
	if (strncmp(pvalp, "    ", 4) == 0)
		strcpy(pvalp, "????");
	tprintf(0, "representation\t%4.4s\n", pvalp);
	tprintf(0, "first index x\t%g\n", f_idx[0]);
	tprintf(0, "first index y\t%g\n", f_idx[1]);
	return 0;
}
/*-------------------------------------------------------------------------*/
	int
nusdims_meta_bin(struct nus_dims_t *dp, struct nusmeta_opts *op)
/*
format:
投影法            char      4 
格子配列の大きさ  long int  4*2
基準点の座標      float     4*2
基準点の緯度経度  float     4*2
格子間隔          float     4*2
標準緯度経度      float     4*2
第2標準緯度経度   float     4*2
緯度経度1         float     4*2
緯度経度2         float     4*2
格子点の意味      char      4
f_idx             float     4*2
計80byte
 */
{
    int	        r;
    char	proj[4], pvalp[4];
    N_SI4	gridsize[2];
    float	grid[7][2], grid0[14];
    float       f_idx[2];
    int         i, j;

    r = get_meta(dp, op, proj, gridsize, grid0, pvalp, f_idx);
    if(r < 0){
        return r;
    }
    for(i = 0; i < 7; i++){
        for(j = 0; j < 2; j++){
            grid[i][j] = grid0[i * 2 + j];
        }
    }

    twrite(proj, 1, 4);
    twrite(gridsize, 4, 2);
    twrite(grid, 4, 14);
    twrite(pvalp, 1, 4);
    twrite(f_idx, 4, 2);
    return 0;
    
}
/*-------------------------------------------------------------------------*/
#define RDR_DICT_PATH_DEFAULT ".:libexec:/grpP/nwp/Open/Const/Vsrf/Dcxx/:/grpK/nwp/Open/Const/Vsrf/Dcxx/"
#define RDR_DICT_NAME "rdrdic.txt"

FILE *
open_rdr_dict()
{
  char *rdr_dict_path, *p;
  FILE *fp;

  rdr_dict_path = ((p = getenv("RDR_DICT_PATH")) == NULL) ?
    strdup(RDR_DICT_PATH_DEFAULT) : strdup(p);
  p = rdr_dict_path;
  for (p = strtok(rdr_dict_path, ":"); p != NULL; p = strtok(NULL, ":")) {
    char path[PATH_MAX];
    strcpy(path, p);
    strcat(path, "/");
    strcat(path, RDR_DICT_NAME);
    if ((fp = fopen(path, "r")) != NULL) {
      tprintf("X-rdr-dict-path", "%s\n", path);
      free(rdr_dict_path);
      return fp;
    }
  }
  free(rdr_dict_path);
  return NULL;
}

char *
get_item(char *p, int n)
{
  int i = 0, next_word;
  while (*p != '\0') {
    next_word = 0;
    while (*p == ' ' || *p == '\t') {
      p++;
      next_word = 1;
    }
    if (next_word && ++i == n)
      return p;
    p++;
  }
  return NULL;
}

	int
rdrtbl2seq(char *item)
{
  char *p = item;
  int iy = 0, im = 0, id = 0, ih = 0;
  iy = atoi(p);
  if (iy > 2100)
    iy = 2100;
  while (*p != '\0' && *p != '.')
    p++;
  if (*p == '\0' || *++p == '\0')
    return 0;
  im = atoi(p);
  while (*p != '\0' && *p != '.')
    p++;
  if (*p == '\0' || *++p == '\0')
    return 0;
  id = atoi(p);
  while (*p != '\0' && *p != '.')
    p++;
  if (*p == '\0' || *++p == '\0')
    return 0;
  ih = atoi(p);
  /*
  tprintf("X-rdr-debug", "yyyy = %d, mm = %d, dd = %d, HH = %d\n",
	  iy, im, id, ih);
  */
  return nwp_ymdhm2seq(iy, im, id, ih, 0);
}

double
rdrtbl2degree(char *item)
{
  int intval = atol(item), int_part = 0, tmp, fun, byou;

  int_part = intval / 10000;
  tmp = intval % 10000;
  fun = tmp / 100;
  byou = tmp % 100;
  return int_part + fun / 60.0 + byou / 60.0 / 100.0;
}

void
check_radar(char *proj, struct nus_dims_t *dp, N_SI4 *gridsize, float grid[][2])
{
  FILE *fp;
  char line[256];
  int start, end;
  double off_x, off_y;
  if (strncmp(proj, "RD", 2) != 0 || (fp = open_rdr_dict()) == NULL)
    return;
  while (fgets(line, 256, fp) != NULL) {
    if (line[0] == '#' || strncmp(line, dp->member, 4) != 0)
      continue;
    start = rdrtbl2seq(get_item(line, 2));
    end   = rdrtbl2seq(get_item(line, 3));
    /*
    tprintf("X-rdr-check", "start = %d ; end = %d ; check = %d\n",
	    start, end, dp->valid);
    */
    if (start > dp->valid || dp->valid > end)
      continue;
    tprintf("X-rdr-dict-entry", "%s\n", line);
    off_x = atof(get_item(line, 7));
    off_y = atof(get_item(line, 8));
    /*
    tprintf("X-rdr-offset", "x = %g ; y = %g\n", off_x, off_y);
    */
    off_x = off_x * 1000.0 / grid[2][0] / 2.0;
    off_y = off_y * 1000.0 / grid[2][1] / 2.0;
    grid[0][0] = gridsize[0] / 2.0 + off_x + 0.5;
    grid[0][1] = gridsize[1] / 2.0 + off_y + 0.5;
    grid[1][0] = rdrtbl2degree(get_item(line, 4));
    grid[1][1] = rdrtbl2degree(get_item(line, 5));;
    grid[3][0] = grid[1][0] - 1.0;
    grid[3][1] = grid[1][1];
    grid[4][0] = grid[1][0] + 1.0;
    grid[4][1] = grid[1][1];
    break;
  }
  fclose(fp);
  return;
}

/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nusmeta_opts opt;
	int	argind;
	int	r;

	if (argc < 2) {
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
