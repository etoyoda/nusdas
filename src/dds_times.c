#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <sys/types.h> /* for opendir */
#include <dirent.h> /* for opendir */
#include <stddef.h>
#include <stdlib.h>
#include <string.h> /* for memcpy */
#include "dset.h"
#include "dfile.h"
# define NEED_SI8_ADD
#include "sys_int.h"
#include "io_comm.h"
#include "sys_mem.h"
# define NEED_STRING_COPY
#include "sys_string.h"
#include "sys_time.h"
#include "sys_err.h"
#include "sys_container.h"
#include "sys_kwd.h"

	static int
BtimeCompare(const void *va, const void *vb)
{
	return *(N_SI4 *)va - *(N_SI4 *)vb;
}

struct dds_eachfile_info {
	array4v_t *tlist;
	nusdset_t *ds;
	N_SI4 basetime;
	int verbose;
};

	static int
dsscanfile_addtolist(N_SI4 time, void *arg)
{
	array4v_t *tlist = arg;
	int r;
	if (array4v_includep(tlist, time) == 0) {
		r = array4v_push(tlist, time);
		if (r) {
			return r;
		}
	}
	return 0;
}

#ifdef USE_NUS_DEBUG
# define message(args) \
	(info->verbose ? nus_warn(args) : nus_debug(args))
#else
# define message(args) (info->verbose && nus_warn(args))
#endif

	static int
ds_scanfile(char *fullpath, N_SI4 searchtype,
		struct dds_eachfile_info *info)
{
	union nusdfile_t *df;
	nusdims_t dim;
	int (*puttime)(N_SI4 time, void *arg) = dsscanfile_addtolist;
	int r;

	dim.basetime = -1;
	df = nusdds_open_dfile(info->ds, fullpath, &dim, IO_READABLE);
	if (df == NULL) {
		nus_warn(("missing/broken file %s", fullpath));
		/** 壊れたファイルがあってもそれだけでエラーにはしない */
		return 0;
	}
	if (searchtype == SYM4_VTLS && info->basetime != -1) {
		N_SI4 bt;
		df_inq_cntl(df, SYM4_BTIM, &bt, NULL);
		if (info->basetime != bt) {
			message(("bt mismatch %#PT %#PT",
				info->basetime, bt));
			/* dds_eachfile の返却値はチェックされない*/
			r = NUSERR_Internal;
			goto finish;
		}
	}
	r = df_inq_cntl(df, searchtype, &puttime, info->tlist);
finish:
	df_close(df);
	return r;
}

	static int
dds_eachfile(char *template, N_UI4 *globtab, sym4_t searchtype,
		struct dds_eachfile_info *info)
{
	struct dirent *entp;
	DIR *dp;
	char dirname[256];
	char subtmpl[256];
	N_UI4 globpos, globtype;
	N_SI4 bvt = -1;
	globpos = globtab[0];
	if (~globpos == 0) {
		message(("file %s query=%Ps", template, searchtype));
		return ds_scanfile(template, searchtype, info);
	}
	globtype = globtab[1];
	if (globpos == 0) {
		string_copy(dirname, ".", sizeof dirname);
	} else {
		string_copy(dirname, template, sizeof dirname);
		dirname[globpos - 1] = '\0';
	}
	message(("opendir(%s)", dirname));
	dp = opendir(dirname);
	if (dp == NULL) {
		/* dds_eachfile の返却値はチェックされない*/
		return nus_err((NUSERR_Internal, "opendir %s failed", dirname));
	}
	string_copy(subtmpl, template, sizeof subtmpl);
	while ((entp = readdir(dp)) != NULL) {
		char *leafname = entp->d_name;
		size_t leaflen;
		leaflen = strlen(leafname);
		switch (globtype) {
			case SYM4_MBLS:
				if (leaflen != 4) {
					continue;
				}
/** @todo 定義ファイルにないメンバ名は却下すべき */
				message(("found m=%s", leafname));
				break;
			case SYM4_BTLS:
			case SYM4_VTLS:
				if (leaflen != 12) {
					continue;
				}
				if (string_to_time(&bvt, leafname) != 0) {
					continue;
				}
				break;
			default:
				return nus_err((NUSERR_Internal,
					"BUG: bad search=%d", searchtype));
				break;
		}
		if (searchtype == globtype) {
			int r;
			message(("found t=%#PT", bvt));
			if (array4v_includep(info->tlist, bvt) == 0) {
				r = array4v_push(info->tlist, bvt);
				if (r) {
					return r;
				}
			}
		} else {
			memcpy(subtmpl + globpos, leafname, leaflen);
			dds_eachfile(subtmpl, globtab + 2, searchtype, info);
		}
	}
	return 0;
}

/** @brief 基準時刻の一覧
 */
	N_SI4
nusdds_btlist(nusdset_t *ds,
		N_SI4 *data,
		N_SI4 data_nelems,
		int verbose)
{
	struct dds_t *dds = &ds->dds;
	struct dds_template *tmpl = &ds->dds.tmpl;
	struct dds_eachfile_info info;
	array4v_t *btlist;
	N_UI4 *globtab;
	int r;
	if ((r = nusdds_build_template(dds, -1)) != 0) {
		return NUSERR_BuildTemplFail;
	}
	btlist = array4v_ini(1000, BtimeCompare);
	globtab = nusdds_tmpl_scanglob(tmpl);
	if (globtab == NULL) {
		return nus_err((NUSERR_MemShort, "memory short"));
	}
	info.tlist = btlist;
	info.ds = ds;
	info.basetime = -1;
	info.verbose = verbose;
	dds_eachfile(tmpl->fullpath, globtab, SYM4_BTLS, &info);
	nus_free(globtab);
	r = array4v_size(btlist);
	array4v_sort(btlist);
	memcpy(data, btlist->list,
			((data_nelems >= r) ? r : data_nelems) * 4);
	array4v_delete(btlist);
	return r;
}

	N_SI4
nusdds_vtlist(nusdset_t *ds,
		N_SI4 *data,
		N_SI4 data_nelems,
		N_SI4 basetime,
		int verbose)
{
	struct dds_t *dds = &ds->dds;
	struct dds_template *tmpl = &ds->dds.tmpl;
	struct dds_eachfile_info info;
	array4v_t *vtlist;
	N_UI4 *globtab;
	int r;
	if ((r = nusdds_build_template(dds, basetime)) != 0) {
		return NUSERR_BuildTemplFail;
	}
	vtlist = array4v_ini(1000, BtimeCompare);
	globtab = nusdds_tmpl_scanglob(tmpl);
	if (globtab == NULL) {
		return nus_err((NUSERR_MemShort, "memory short"));
	}
	info.tlist = vtlist;
	info.ds = ds;
	info.basetime = basetime;
	info.verbose = verbose;
	dds_eachfile(tmpl->fullpath, globtab, SYM4_VTLS, &info);
	nus_free(globtab);
	r = array4v_size(vtlist);
	if (verbose) {
		nus_warn(("list size %d", r));
	} else {
		nus_debug(("list size %d", r));
	}
	array4v_sort(vtlist);
	memcpy(data, vtlist->list,
			((data_nelems >= r) ? r : data_nelems) * 4);
	array4v_delete(vtlist);
	return r;
}
