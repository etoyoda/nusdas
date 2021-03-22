/* nusdim.h */
/* $Id: nusdim.h,v 1.5 2007-02-27 09:06:08 suuchi43 Exp $ */

#ifndef _NUSDIM_H
#define _NUSDIM_H

#ifndef __GNUC__
# define __attribute__(attrs)
#endif

/*
 * type definitions
 */

struct nus_type_t {
	/* they are NOT null-terminated string */
	char	type1[8];
	char	type2[4];
	char	type3[4];
};

	/* added in v2.0 */
struct nus_dims_t {
	/* bit fields for validity of following dimensions */
        int       		stat;
#define	NUSDIM_TYPE		(1 << 0)
#define	NUSDIM_BASE		(1 << 1)
#define	NUSDIM_MEMBER		(1 << 2)
#define	NUSDIM_VALID		(1 << 3)
#define	NUSDIM_VALID2		(1 << 4)
#define	NUSDIM_PLANE		(1 << 5)
#define	NUSDIM_PLANE2		(1 << 6)
#define	NUSDIM_ELEMENT		(1 << 7)
        struct nus_type_t       type;
        N_SI4                   base;
	/* this is NOT null-terminated string */
        char                    member[4];
        N_SI4                   valid;
        N_SI4                   valid2;
	/* they are NOT null-terminated string */
        char                    plane[6];
        char                    plane2[6];
        char                    element[6];
};

enum miss_modes { ERROR, NONE, UDFV, MASK };

struct grids_t {
	int	xysize[2];
        enum miss_modes miss_mode;
        unsigned char *ucbuf;
        unsigned char ucmissval;
	int	*ibuf;
        int     imissval;
	float	*fbuf;
        float   fmissval;
	double	*dbuf;
        double  dmissval;
};

/*
 * function prototypes
 */

/* getgrids.h */
int nusdims_get_grids(struct nus_dims_t *dp, struct grids_t *gp);
int nusdims_get_grids_uint8(struct nus_dims_t *dp, struct grids_t *gp);
int nusdims_get_grids_int32(struct nus_dims_t *dp, struct grids_t *gp);
int nusdims_get_grids_double64(struct nus_dims_t *dp, struct grids_t *gp);


int split_type(const char *type, struct nus_type_t *result);

/* toyoda-hacked NuSDaS */
extern int	test_dir(const char *dirname);

/* setenv.c */
extern int nusdas_addroot(const char *root);

/* panpath.c */
extern char *nusdims_to_path(const struct nus_dims_t *dp);
extern int path_to_nusdims(const char *path, struct nus_dims_t *dp);

#endif /* _NUSDIM_H */
