/* nusinqdef.c */
/* 2005/6/6 tabito */
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <strings.h>
#include <limits.h>
#include "stringplus.h"
#include "nusdas.h"
#include "textout.h"
#include "nusdim.h"


static char rcsid[] = "$Id: nusinqdef.c,v 1.5 2007-03-17 12:13:09 suuchi04 Exp $";

struct nusdef_opts{
    int (*operator)(struct nus_dims_t *dp, struct nusdef_opts *op);
    int put_title;
    int is_radar;
};

int nusdims_def_rd(struct nus_dims_t *dp, struct nusdef_opts *op);
int nusdims_def_emap(struct nus_dims_t *dp, struct nusdef_opts *op);



	void
opt_clear(struct nusdef_opts *op)
{
    op->operator =nusdims_def_rd; 
    op->put_title = 0;
}

	void
option(struct nusdef_opts *op, const char *str)
{
	if (str == NULL)
		return;
	if (*str == '-')
		str++;
	switch (*str) {
	case 't':
             if (str[1] == 'r') {
                tset_content_type("text/x-rd");
                op->operator =nusdims_def_rd;  
            }
            break;
	case 'm':
		op->operator = nusdims_def_emap;
                tset_content_type("application/x-uint8-stream");
		break;
        case 'l':
            op->put_title = 1;
	default:
		break;
	}
}

	int
nusdims_def_rd(struct nus_dims_t *dp, struct nusdef_opts *op)
{
    int	r, n = 2;
    int     array_num;
    int     member_num, vtime_num, plane_num, elem_num;
    char    *member_list, *plane_list, *elem_list;
    int     *vtime_list;
    char buf[1024];
    char unit[4];
    int  gridsize[2];
    float distance[2];
    float basepoint[4];    /* x0, y0, lat_0, lon_0 */
    float stand_latlon[4]; /* lat_s1, lon_s1, lat_s2, lon_s2 */
    float spare_latlon[4]; /* lat_1, lon_1, lat_2, lon_2 */
    char proj[4];
    char *elmap;
    int elmap_num;
    int  ii;
    int subc_num, info_num;
    char *subc_list, *info_list;
    
    /* MEMBER */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_MEMBER_NUM, (void*)&member_num, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_MEMBER_NUM)] %d, %d\n", r, array_num);
        return r;
    }
    if((member_list = (char*)malloc(4 * member_num)) == NULL){
        eprintf(HTTP_ERR, "nusdas_inq_def: malloc error, %s, %d\n", 
                __FILE__, __LINE__ - 1);
        return -10;
    }
    
    array_num = member_num;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_MEMBER_LIST, (void*)member_list, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_MEMBER_LIST)] %d, %d\n", r, array_num);
        return r;
    }

    /* VALID_TIME */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_VALIDTIME_NUM, (void*)&vtime_num, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_VALIDTIME_NUM)] %d, %d\n", r, array_num);
        return r;
    }
    if((vtime_list = (int*)malloc(sizeof(int) * vtime_num)) == NULL){
        eprintf(HTTP_ERR, "nusdas_inq_def: malloc error, %s, %d\n", 
                __FILE__, __LINE__ - 1);
        return -10;
    }
    
    array_num = vtime_num;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_VALIDTIME_LIST, (void*)vtime_list, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_VALIDTIME_LIST)] %d, %d\n", r, array_num);
        return r;
    }

    /* VALIDTIME UNIT */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_VALIDTIME_UNIT, unit, (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_VALIDTIME_UNIT)] %d, %d\n", r, array_num);
        return r;
    }

    /* PLANE */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_PLANE_NUM, (void*)&plane_num, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_PLANE_NUM)] %d, %d\n", r, array_num);
        return r;
    }
    if((plane_list = (char*)malloc(6 * plane_num)) == NULL){
        eprintf(HTTP_ERR, "nusdas_inq_def: malloc error, %s, %d\n", 
                __FILE__, __LINE__ - 1);
        return -10;
    }

    array_num = plane_num;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_PLANE_LIST, (void*)plane_list, 
                         (N_SI4*)&array_num);

    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_PLANE_LIST)] %d, %d\n", r, array_num);
        return r;
    }

    /* ELEMENT */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_ELEMENT_NUM, (void*)&elem_num, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_ELEMENT_NUM)] %d, %d\n", r, array_num);
        return r;
    }
    if((elem_list = (char*)malloc(6 * elem_num)) == NULL){
        eprintf(HTTP_ERR, "nusdas_inq_def: malloc error, %s, %d\n", 
                __FILE__, __LINE__ - 1);
        return -10;
    }
    
    array_num = elem_num;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_ELEMENT_LIST, (void*)elem_list, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_ELEMENT_LIST)] %d, %d\n", r, array_num);
        return r;
    }

    /* ELEMENT MAP */
    elmap_num = member_num * vtime_num * plane_num * elem_num;
    if((elmap = (char*)malloc(elmap_num * sizeof(char))) == NULL ){
        eprintf(HTTP_ERR, "nusdas_inq_def: malloc error,  %s, %d\n", 
                __FILE__, __LINE__ - 1);
        return -10;
    }
    array_num = elmap_num;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_ELEMENT_MAP, (void*)elmap, (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_ELEMENT_MAP)] %d, %d\n", r, array_num);
        return r;
    }

    /* PROJECTION */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_PROJECTION, (void*)proj, (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_PROJECTION)] %d, %d\n", r, array_num);
        return r;
    }

    /* GRID SIZE */
    array_num = 2;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_GRID_SIZE, (void*)gridsize, (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_GRID_SIZE)] %d, %d\n", r, array_num);
        return r;
    }

    /* GRID DISTANCE */
    array_num = 2;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_GRID_DISTANCE, (void*)distance, (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_GRID_DISTANCE)] %d, %d\n", r, array_num);
        return r;
    }

    /* GRID BASEPOINT */
    array_num = 4;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_GRID_BASEPOINT, (void*)basepoint, (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_GRID_BASEPOINT)] %d, %d\n", r, array_num);
        return r;
    }

    /* STAND LATLON */
    array_num = 4;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_STAND_LATLON, (void*)stand_latlon,
                       (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_STAND_LATLON)] %d, %d\n", r, array_num);
        return r;
    }

    /* SPARE LATLON */
    array_num = 4;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_SPARE_LATLON, (void*)spare_latlon,
                       (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_SPARE_LATLON)] %d, %d\n", r, array_num);
        return r;
    }

    /* SUBC_NUM */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_SUBC_NUM, (void*)&subc_num,
                       (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_SUBC_NUM)] %d, %d\n", r, array_num);
        return r;
    }
    
    /* SUBC_LIST */
    if (subc_num > 0) {
	    if((subc_list = (char*)malloc(4 * subc_num)) == NULL){
		    eprintf(HTTP_ERR, 
			    "nusdas_inq_def: malloc error, %s, %d\n", 
			    __FILE__, __LINE__ - 1);
		    return -10;
	    }
	    array_num = subc_num;
	    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
			       N_SUBC_LIST, (void*)subc_list,
			       (N_SI4*)&array_num);
	    if (r != array_num) {
		    eprintf(HTTP_ERR, 
			    "nusdas error [nusdas_inq_def(N_SUBC_LIST)] "
			    "%d, %d\n", r, array_num);
		    return r;
	    }
    }

    /* INFO_NUM */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                       N_INFO_NUM, (void*)&info_num,
                       (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error [nusdas_inq_def(N_INFO_NUM)] %d, %d\n", r, array_num);
        return r;
    }
    
    /* INFO_LIST */
    if (info_num > 0) {
	    if((info_list = (char*)malloc(4 * info_num)) == NULL){
		    eprintf(HTTP_ERR, 
			    "nusdas_inq_def: malloc error, %s, %d\n", 
			    __FILE__, __LINE__ - 1);
		    return -10;
	    }
	    array_num = info_num;
	    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
			       N_INFO_LIST, (void*)info_list,
			       (N_SI4*)&array_num);
	    if (r != array_num) {
		    eprintf(HTTP_ERR, 
			    "nusdas error [nusdas_inq_def(N_INFO_LIST)] "
			    "%d, %d\n", r, array_num);
		    return r;
	    }
    }

    /*  caution !!
        add 1 space after comma
    */

    tprintf(0,"=begin\n");
    if(op->put_title == 1){
        tprintf(0,"=def for %s\n", nusdims_to_path(dp));
    }
    tprintf(0,"==definition info\n");
    tprintf(0,"=end\n");
    tprintf(0,"=begin RT\n");
    /* MEMBER */
    tprintf(0, "number of member, %d\n", member_num);
    tprintf(0, "member list, ");
    for (ii = 0; ii < member_num; ii++){
        memcpy(buf, member_list + ii * 4, 4);
        buf[4] = '\0';
        tprintf(0, "\'%s\'", buf);
        if(ii != member_num - 1){
            tprintf(0, " ");
        }
        else{
            tprintf(0, "\n");
        }
    }

    /* VALIDTIME */
    tprintf(0, "number of validtime, %d\n", vtime_num);
    tprintf(0, "validtime list, ");
    for (ii = 0; ii < vtime_num; ii++){
        tprintf(0, "%d", vtime_list[ii]);
        if(ii != vtime_num -1){
            tprintf(0, " ");
        }
        else{
            tprintf(0, "\n");
        }
    }
    /* VALIDTIME UNIT */
    tprintf(0, "validtime unit, %4.4s\n", unit);

    /* PLANE */
    tprintf(0, "number of plane, %d\n", plane_num);
    tprintf(0, "plane list, ");
    for (ii = 0; ii < plane_num; ii++){
        memcpy(buf, plane_list + ii * 6, 6);
        buf[6] = '\0';
        tprintf(0, "\'%s\'", buf);
        if(ii != plane_num - 1){
            tprintf(0, " ");
        }
        else{
            tprintf(0, "\n");
        }
    }

    /* ELEMENT */
    tprintf(0, "number of element, %d\n", elem_num);
    tprintf(0, "element list, ");
    for (ii = 0; ii < elem_num; ii++){
        memcpy(buf, elem_list + ii * 6, 6);
        buf[6] = '\0';
        tprintf(0, "\'%s\'", buf);
        if(ii != elem_num - 1){
            tprintf(0, " ");
        }
        else{
            tprintf(0, "\n");
        }
    }

    /* ELEMENT MAP */
    /*
    tprintf(0, "element map, ");
    for (ii = 0; ii < elmap_num; ii++){
        tprintf(0, "%1d", elmap[ii]);
    }
    tprintf(0, "\n");
    */
    /* PROJECTION */
    tprintf(0, "projection type, %4.4s\n", proj);
    tprintf(0, "number of x grids, %d\n", gridsize[0]);
    tprintf(0, "number of y grids, %d\n", gridsize[1]);
    tprintf(0, "base point x, %g\n", basepoint[0]);
    tprintf(0, "base point y, %g\n", basepoint[1]);
    tprintf(0, "base point lat, %.5f\n", basepoint[2]);
    tprintf(0, "base point lon, %.5f\n", basepoint[3]);
    tprintf(0, "grid interval x, %g\n", distance[0]);
    tprintf(0, "grid interval y, %g\n", distance[1]);
    tprintf(0, "standard lat 1, %.5f\n", stand_latlon[0]);
    tprintf(0, "standard lon 1, %.5f\n", stand_latlon[1]);
    tprintf(0, "standard lat 2, %.5f\n", stand_latlon[2]);
    tprintf(0, "standard lon 2, %.5f\n", stand_latlon[3]);
    tprintf(0, "latitude 1, %.5f\n", spare_latlon[0]);
    tprintf(0, "longitude 1, %.5f\n", spare_latlon[1]);
    tprintf(0, "latitude 2, %.5f\n", spare_latlon[2]);
    tprintf(0, "longitude 2, %.5f\n", spare_latlon[3]);

    /* SUBC */
    tprintf(0, "number of subcntl, %d\n", subc_num);
    tprintf(0, "subcntl list, ");
    for (ii = 0; ii < subc_num; ii++){
        memcpy(buf, subc_list + ii * 4, 4);
        buf[4] = '\0';
        tprintf(0, "\'%s\'", buf);
        if(ii != subc_num - 1){
            tprintf(0, " ");
        }
        else{
            tprintf(0, "\n");
        }
    }
    if (subc_num == 0) {
            tprintf(0, "\n");
    }
    /* INFO */
    tprintf(0, "number of info, %d\n", info_num);
    tprintf(0, "info list, ");
    for (ii = 0; ii < info_num; ii++){
        memcpy(buf, info_list + ii * 4, 4);
        buf[4] = '\0';
        tprintf(0, "\'%s\'", buf);
        if(ii != info_num - 1){
            tprintf(0, " ");
        }
        else{
            tprintf(0, "\n");
        }
    }
    if (info_num == 0) {
            tprintf(0, "\n");
    }
    tprintf(0,"=end RT\n");
    tprintf(0,"\r\n");
    
    free(member_list);
    free(vtime_list);
    free(plane_list);
    free(elem_list);
    if (subc_num > 0) {
	    free(subc_list);
    }
    if (info_num > 0) {
	    free(info_list);
    }
    return 0;
}
/*-------------------------------------------------------------------------*/
	int
nusdims_def_emap(struct nus_dims_t *dp, struct nusdef_opts *op)
{
    int	    r;
    int     array_num;
    int     indx_size;
    int     query;
    char    *emap;
    
    /* N_INDX_SIZE */
    array_num = 1;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_INDX_SIZE, (void*)&indx_size, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error "
		"[nusdas_inq_def(N_INDX_SIZE)] %d, %d\n", r, array_num);
        return r;
    }
    if ((emap = (char*)malloc(indx_size)) == NULL) {
	    eprintf(HTTP_ERR,  "malloc failed\n");
	    return -10;
    }
    array_num = indx_size;
    r = nusdas_inq_def(dp->type.type1, dp->type.type2, dp->type.type3,
                         N_ELEMENT_MAP, (void*)emap, 
                         (N_SI4*)&array_num);
    if (r != array_num) {
        eprintf(HTTP_ERR, "nusdas error "
		"[nusdas_inq_def(N_ELEMENT_MAP)] %d, %d\n", r, array_num);
        return r;
    }
    
    twrite(emap, 1, indx_size);
    free(emap);
    return r;

}


/*-------------------------------------------------------------------------*/
    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	struct nusdef_opts opt;
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
		if (!(r & NUSDIM_TYPE)) {
			eprintf(HTTP_ERR, "type not specified\n");
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
