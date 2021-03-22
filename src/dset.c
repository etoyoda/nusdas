/** @file
 * @brief 各種データセット共通処理
 */

#include "config.h"
#include <stddef.h>
#include <stdlib.h>
#include "internal_types.h"
#include "dset.h"
#include "sys_kwd.h"
#include "sys_err.h"
#include "sys_endian.h"
#include "glb.h"
#include "sys_mem.h"

	int
nusxds_subcpreset(union nusdset_t *ds,
		sym4_t grp,
		size_t nbytes,
		int (*encoder)(void *rec, N_UI4 siz,
			void *arg, union nusdset_t *ds),
		void *arg)
{
	struct ds_preset_t *sp;
	struct ds_preset_t **dest;
	int r;
	switch (grp) {
		case SYM4_ETA:
			dest = &(ds->comm.sc_eta);
			break;
		case SYM4_SIGM:
			dest = &(ds->comm.sc_sigm);
			break;
		case SYM4_ZHYB:
			dest = &(ds->comm.sc_zhyb);
			break;
		case SYM4_RGAU:
			dest = &(ds->comm.sc_rgau);
			break;
		case SYM4_DELT:
			dest = &(ds->comm.sc_delt);
			break;
		default:
			return nus_err((NUSERR_SP_BadGroup,
						"invalid group %Ps", grp));
	}
	r = ds_subc_namecheck(ds, grp, nbytes);
	if (r != 0) {
		return r;
	}
	sp = nus_malloc(sizeof(*sp) + nbytes);
	if (sp == NULL) {
		return nus_err((NUSERR_SP_MemShort, "memory short"));
	}
	sp->sp_grp = grp;
	sp->sp_nbytes = nbytes;
	sp->sp_contents = (N_UI1 *)(sp + 1);
	encoder(sp->sp_contents, nbytes, arg, NULL);
	if (*dest) {
		nus_warn(("preset subc %Ps override", grp));
		nus_free(*dest);
	}
	*dest = sp;
	nus_debug(("preset %Ps %u %p", grp, nbytes, sp));
	return 0;
}

	int
nusdset_opt(struct nusxds_param_t *param, sym4_t key, const char *val)
{
	if (nusio_opt(param, key, val) == 0)
		return 0;
	switch (key) {
		case SYM4_FWBF:
			DynParam(param, pc_wbuffer) = (val ? atoi(val) : 0);
			return 0;
			break;
		case SYM4_FRBF:
			DynParam(param, pc_rbuffer) = (val ? atoi(val) : 0);
			return 0;
			break;
	}
	return -1;
}

/** @brief データセットの探索
 *
 * 通常は NUSDAS_INIT から nusglb_config() を経由して呼ばれる。
 * @warning 複数回起動してはならない。nusglb_config() で重複起動回避している。
 */
	int
nusdset_scan(void)
{
	int r;

	r = nusdds_scan();
	if (r) return r;

	r = nuspds_scan();
	if (r) nus_err((-1, "Cannot read file PANDORA_SERVER_LIST"));
	r = nusglb_allds_push();
	nus_debug(("allds_push => %d", r));
	if (r) return r;
	return 0;
}
