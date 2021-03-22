/** @file
 * @brief 各種別ごとのデータセットのリスト
 */

#include "config.h"
#include "nusdas.h"
#include "sys_container.h"
#include "internal_types.h"
#include <stddef.h>
#include "dset.h"
#include "glb.h"
#include "sys_err.h"
#include <stdlib.h>
#include "sys_mem.h"

#define ARRAY_SIZE_INI	8

struct nustype_dstab {
	int	last_scanned_nrd;
	arrayp_t	*dslist;
};

static int
nusdset_cmp_nrd(const void *a, const void *b)
{
	const nusdset_t *dsa = a;
	const nusdset_t *dsb = b;
	return dsa->comm.nrd - dsb->comm.nrd;
}

struct nustype_dstab *
nustype_dstab_ini(void)
{
	struct nustype_dstab *p;
	p = nus_malloc(sizeof(struct nustype_dstab));
	if (p == NULL) {
		return NULL;
	}
	p->last_scanned_nrd = 0;
	p->dslist = arrayp_ini(ARRAY_SIZE_INI, nusdset_cmp_nrd);
	if (p->dslist == NULL) {
		nus_free(p);
		return NULL;
	}
	return p;
}

int
nustype_dstab_each(struct nustype_dstab *dst,
		int (*callback)(nusdset_t *, void *), void *arg)
{
	unsigned i;
	int r;
	for (i = 0; i < dst->dslist->num_entries; i++) {
		if ((r = callback(dst->dslist->list[i], arg)) != 0) {
			nus_debug(("movehead 0"));
			arrayp_movehead(dst->dslist, 
					dst->dslist->list[i]);
			return r;
		}
	}
	return 0;
}

union nusdset_t *
nustype_dstab_first(struct nustype_dstab *dst)
{
	if (GlobalConfig(nrd_override) == NRD_UNFIX) {
		if (dst->dslist->num_entries > 0) {
			return dst->dslist->list[0];
		} else {
			return NULL;
		}
	} else {
		unsigned i;
		for (i = 0; i < dst->dslist->num_entries; i++) {
			union nusdset_t *p;
			p = dst->dslist->list[i];
			if (p->comm.nrd == GlobalConfig(nrd_override)) {
				return p;
			}
		}
		return NULL;
	}
}

int
nustype_dstab_push(struct nustype_dstab *dst, union nusdset_t *ds)
{
	unsigned i;
	if (GlobalConfig(nrd_override) != NRD_UNFIX) {
		return 0;
	}
	for (i = 0; i < arrayp_size(dst->dslist); i++) {
		void *ds2;
		ds2 = arrayp_get_value(dst->dslist, i);
		if (ds == ds2)
			return 0;
	}
	return arrayp_push(dst->dslist, ds);
}
int
nustype_dstab_movehead(struct nustype_dstab *dst, union nusdset_t *ds)
{
	return arrayp_movehead(dst->dslist, ds);
}
