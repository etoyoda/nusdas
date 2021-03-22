/* nusdir.h */
/* $Id: nusdir.h,v 1.2 2007-02-26 03:01:49 suuchi43 Exp $ */

#include "nusdim.h"

struct nusdir_opts {
	int				longe;
	int				debug;
};

/* listtype.c */
extern int	nusls_type(const char *pattern, const int longe);

/* dirsub.c */
extern int nusdir_base(struct nus_dims_t *dp, struct nusdir_opts *op);
extern int nusdir_member(struct nus_dims_t *dp, struct nusdir_opts *op);
extern int nusdir_valid(struct nus_dims_t *dp, struct nusdir_opts *op);
extern int nusdir_valid2(struct nus_dims_t *dp, struct nusdir_opts *op);
extern int nusdir_plane(struct nus_dims_t *dp, struct nusdir_opts *op);
extern int nusdir_plane2(struct nus_dims_t *dp, struct nusdir_opts *op);
extern int nusdir_element(struct nus_dims_t *dp, struct nusdir_opts *op);
