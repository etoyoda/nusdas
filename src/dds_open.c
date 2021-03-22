/** \file
 * \brief 定義ファイル型データセット (DDS)
 */

#include "config.h"
#include "nusdas.h"
#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "internal_types.h"
#include "sys_time.h"
# define NEED_MAKE_UI8
#include "sys_int.h"
#include "sys_mem.h"
# define NEED_STRING_DUP
# define NEED_MEMCPY6
# define NEED_MEMCPY4
#include "sys_string.h"
#include "sys_kwd.h"
#include "sys_err.h"
#include "io_comm.h"
#include "dset.h"
#include "dfile.h"
#include "glb.h"

	static int
dds_inq_def(nusdset_t *ds,
		const N_SI4 param,
		void	*data,
		N_SI4 datasize)
{
	N_SI4	*iptr;
	char	*cptr;
	sym4_t	*s4ptr;
	float	*fptr;
	long	i, imax;
	if (ds->dds.def.lexbuffer) {
		if (nusdef_endparse(&ds->dds.def))
			return NUS_ERRNO;
	}
	if (data == NULL || datasize <= 0) {
		return -2;
	}
	switch (param) {
		case N_MEMBER_NUM:
			if (datasize < 1) return -1;
			iptr = data;
			*iptr = ds->dds.def.n_mb;
			return 1;
		case N_VALIDTIME_NUM:
			if (datasize < 1) return -1;
			iptr = data;
			*iptr = ds->dds.def.n_vt;
			return 1;
		case N_PLANE_NUM:
			if (datasize < 1) return -1;
			iptr = data;
			*iptr = ds->dds.def.n_lv;
			return 1;
		case N_ELEMENT_NUM:
			if (datasize < 1) return -1;
			iptr = data;
			*iptr = ds->dds.def.n_el;
			return 1;
		case N_GRID_SIZE:
			if (datasize < 2) return -1;
			iptr = data;
			iptr[0] = ds->dds.def.nx;
			iptr[1] = ds->dds.def.ny;
			return 2;
		case N_MEMBER_LIST:
			if (datasize < ds->dds.def.n_mb) return -1;
			s4ptr = data;
			for (i = 0; i < ds->dds.def.n_mb; i++) {
				s4ptr[i] = ds->dds.def.mb[i];
			}
			return ds->dds.def.n_mb;
		case N_VALIDTIME_LIST:
			imax = ds->dds.def.n_vt;
			if (datasize < imax) return -1;
			iptr = data;
			for (i = 0; i < imax; i++) {
				iptr[i] = ds->dds.def.ft1[i];
			}
			return imax;
		case N_VALIDTIME2_LIST:
			imax = ds->dds.def.n_vt;
			if (datasize < imax) return -1;
			iptr = data;
			for (i = 0; i < imax; i++) {
				iptr[i] = (ds->dds.def.ft2
						? ds->dds.def.ft2[i] : 1);
			}
			return imax;
		case N_VALIDTIME_UNIT:
			if (datasize < 1) return -1;
			s4ptr = data;
			*s4ptr = ds->dds.def.ftunits;
			return 1;
		case N_PLANE_LIST:
#define	Get_Char6_List(number, ptr) { \
			imax = ds->dds.def.number; \
			if (datasize < imax) return -1; \
			cptr = data; \
			for (i = 0; i < imax; i++) { \
				memcpy6(cptr, \
					(char *)((ptr) + i)); \
				cptr += 6; \
			} \
			return imax; }
			Get_Char6_List(n_lv, ds->dds.def.lv1)
		case N_PLANE2_LIST:
			Get_Char6_List(n_lv,
			 ds->dds.def.lv1 ? ds->dds.def.lv1 : ds->dds.def.lv2)
		case N_ELEMENT_LIST:
			Get_Char6_List(n_el, ds->dds.def.el)
		case N_ELEMENT_MAP:
			imax = ds->dds.def.n_mb * ds->dds.def.n_vt
				* ds->dds.def.n_lv * ds->dds.def.n_el;
			if (datasize < imax) {
				return -1;
			}
			cptr = data;
			for (i = 0; i < imax; i++) {
				/* 0xFFFFFFFF のとき 0 を入れる */
				cptr[i] = (~(ds->dds.def.elementmap[i])
					? 1 : 0);
			}
			return imax;
		case N_PROJECTION:
			if (datasize < 1) return -1;
			s4ptr = data;
			*s4ptr = nusdef_projcode2(&ds->dds.def);
			return 1;
		case N_GRID_DISTANCE:
			if (datasize < 2) return -1;
			fptr = data;
			fptr[0] = ds->dds.def.projparam[2][0];
			fptr[1] = ds->dds.def.projparam[2][1];
			return 2;
		case N_GRID_BASEPOINT:
			if (datasize < 4) return -1;
			fptr = data;
			fptr[0] = ds->dds.def.projparam[0][0];
			fptr[1] = ds->dds.def.projparam[0][1];
			fptr[2] = ds->dds.def.projparam[1][0];
			fptr[3] = ds->dds.def.projparam[1][1];
			return 4;
		case N_STAND_LATLON:
			if (datasize < 4) return -1;
			fptr = data;
			fptr[0] = ds->dds.def.projparam[3][0];
			fptr[1] = ds->dds.def.projparam[3][1];
			fptr[2] = ds->dds.def.projparam[4][0];
			fptr[3] = ds->dds.def.projparam[4][1];
			return 4;
		case N_SPARE_LATLON:
			if (datasize < 4) return -1;
			fptr = data;
			fptr[0] = ds->dds.def.projparam[5][0];
			fptr[1] = ds->dds.def.projparam[5][1];
			fptr[2] = ds->dds.def.projparam[6][0];
			fptr[3] = ds->dds.def.projparam[6][1];
			return 4;
	        case N_SUBC_NUM:
			if (datasize < 1) return -1;
			iptr = data;
			*iptr = ds->dds.def.n_subc;
			return 1;
	        case N_SUBC_LIST:
#define Get_Char4_List_Tab(cptr, table, imax) { \
                        nusdef_subcinfo_t *p; \
			if (datasize < imax) return -1; \
			cptr = data; \
			for (p = table; p ; p = p->next) { \
				memcpy4(cptr,(char *)&(p -> group)); \
				cptr += 4; \
			} \
                        return imax; }
			Get_Char4_List_Tab(cptr, ds->dds.def.subctab, 
					   ds->dds.def.n_subc);
	        case N_INFO_NUM:
			if (datasize < 1) return -1;
			iptr = data;
			*iptr = ds->dds.def.n_info;
			return 1;
	        case N_INFO_LIST:
			Get_Char4_List_Tab(cptr, ds->dds.def.infotab, 
					   ds->dds.def.n_info);
	        case N_INDX_SIZE:
			if (datasize < 1) return -1;
			iptr = data;
			*iptr = ds->dds.def.n_mb * ds->dds.def.n_vt
				* ds->dds.def.n_lv * ds->dds.def.n_el;
			return 1;
		default:
			return -3;
	}
}

/** @brief データセットが許容しない対象時刻か否か
 * @retval 0 対象時刻は有効
 * @retval 1 対象時刻は無効
 */
	static int
dds_unacceptable_vtime(const nusdset_t *ds, const nusdims_t *dim)
{
	int	i;
	const nusdef_t	*def;
	def = &ds->dds.def;
	for (i = 0; i < def->n_vt; i++) {
		N_SI4 vt1, vt2;
		vt1 = time_add(dim->basetime, def->ft1[i], def->ftunits);
		if (vt1 != dim->validtime1) {
			continue;
		}
		if (def->ft2 == NULL) {
			return 0;
		}
		vt2 = time_add(dim->basetime, def->ft2[i], def->ftunits);
		if (vt2 == dim->validtime2) {
			return 0;
		}
	}
	return 1;
}

/** @brief データセットの定義ファイル解読を完結させる
 * @retval 0 正常終了
 * @retval 他 エラー
 */
	int
dds_endparse(struct dds_t *dds)
{
	int r;
	if (!nusdef_immature(&dds->def)) {
		return 0;
	}
	r = nusdef_endparse(&dds->def);
	if (r) {
		return r;
	}
	if (dds->def.options) {
		nusdas_opts(&dds->comm.param,
			(unsigned char *)dds->def.options);
	}
	return 0;
}

/** @brief データセット内でデータファイルを開く
 *
 * データセットがキャッシュで持っていればそれを返し、
 * 持っていなければ df_open() で開く。
 */
	union nusdfile_t *
nusdds_open_dfile(nusdset_t *ds, char *fullpath, const nusdims_t *dim,
		int open_flags)
{
	struct dds_t	*dds = &(ds->dds);
	union nusdfile_t *df;

	if ((df = dds_dftab_get(dds->dftab, fullpath)) != NULL) {
		nus_debug(("dds_findfile has %s (%s)", fullpath,
					df->comm.is_open ? "open" : "closed"));
		if (!df->comm.is_open) {
			int r;
			if (open_flags == 0) {
				return NULL;
			}
			r = df_reopen(df, open_flags);
			nus_debug(("df_reopen(%d) => %d", open_flags, r));
		} else if ((open_flags & IO_WRITABLE)
				&& !(df->comm.flags & IO_WRITABLE)) {
			int r;
			r = df_close(df);
			nus_debug(("df_close() => %d", r));
			if (r == 0) {
				r = df_reopen(df, open_flags);
				nus_debug(("df_reopen(%d) => %d",
							open_flags, r));
			}
		}
		if (df->comm.is_open) {
			return df;
		}
	}
	if (open_flags == 0) {
		return NULL;
	}

	df = df_open(fullpath, open_flags, dds, dim);
	if (df) {
		dds_dftab_put(dds->dftab, fullpath, df);
	}
	return df;
}

/** @brief データファイルの探索.
 *
 * 与えられたデータセット dds の中でキー群 dim に対応する
 * データファイルを開いて返す。
 * 同じファイル名について既に開かれたファイルがあればそれを返す
 * (この場合ファイルは閉じられているかもしれない)。
 *
 * @p open_flags がゼロの場合はファイルが開かれている場合にだけこれを返し、
 * 開かれていなければ NULL を返す。
 *
 * @note オープンした df ハンドルは df_close() で閉じなければならない。
 */
	static union nusdfile_t *
dds_findfile_specific(nusdset_t *ds, const nusdims_t *dim, int open_flags)
{
	struct dds_t	*dds = &(ds->dds);
	char *fullpath;
	union nusdfile_t *df;
	int r;

        /* todo: IOドライバが eio_open のときパス規則を迂回すべき。
	   たぶんこのへんでやる */

	if ((r = nusdds_build_template(dds, -1)) != 0) {
		SETERR(r); /* ぬるぽの壁 */
		return NULL;
	}

	fullpath = nusdds_tmpl_expand(&dds->tmpl, dim);
	if (fullpath == NULL) {
		return NULL;
	}

	df = nusdds_open_dfile(ds, fullpath, dim, open_flags);
	if (df == NULL && open_flags == 0) {
		if (dds_unacceptable_vtime(ds, dim)) {
			nus_err((NUSERR_BadVtime,
				"bad base/valid time %PT:%PT:%PT",
				dim->basetime, dim->validtime1,
				dim->validtime2));
		} else {
			SETERR(NUSERR_OC_NoOpenDfile);
		}
	}
	nus_free(fullpath);
	return df;
}

/** @brief ワイルドカード (btime == -1) つきの dds_findfile()
 */
	static union nusdfile_t *
dds_findfile(nusdset_t *ds, const nusdims_t *dim, int open_flags)
{
	union nusdfile_t *df;
	struct dds_t *dds = &(ds->dds);
	nusdims_t xdim;
	int r, i;

	/** @note 書き込み用に開く場合と basetime != -1 は
	 * ワイルドカードは無効 */
	if ((open_flags & IO_WRITABLE) || (dim->basetime != -1)) {
		return dds_findfile_specific(ds, dim, open_flags);
	}
	/** @note 定義ファイルの解読が未完了ならば完成させる. */
	if ((r = dds_endparse(dds)) != 0) {
		SETERR(r); /* ぬるぽの壁 */
		return NULL;
	}
	xdim = *dim;
	for (i = 0; i < dds->def.n_vt; i++) {
		xdim.basetime = time_add(dim->validtime1,
				-(dds->def.ft1[i]),
				dds->def.ftunits);
		/* 下位メッセージを抑止してファイル検索 */
		df = dds_findfile_specific(ds, &xdim, open_flags | IO_ERRMSG_OFF );
		if (df != NULL) {
			return df;
		}
	}
	if(df == NULL) {
		nus_err((NUSERR_OpenRFailed, "cannot find valid time %PT", dim->validtime1));
	}
	if (NUS_ERRNO == NUSERR_OpenRFailed) {
		SETERR(NUSERR_WildcardBTFail);
	}
	return NULL;
}

/** @brief ds 内で dim で指定される記録を読み出す。
 */
static int
dds_readdata(union nusdset_t *ds, nusdims_t *dim, struct ibuffer_t *buf)
{
	nusdfile_t *df;
	int r;
	df = dds_findfile(ds, dim, IO_READABLE);
	if (df == NULL) {
		if (NUS_ERRNO == NUSERR_OpenRFailed) {
			return SETERR(NUSERR_RD_OpenFail);
		}
		return NUS_ERRNO;
	}
	r = df_read(df, dim, buf);
	if (GlobalConfig(io_r_fclose)) {
		int	rc;
		rc = df_close(df);
		if (r == 0) {
			r = rc;
		}
	}
	return r;
}

/** @brief ds 内で dim で指定される記録に対する各種問合せ
 */
static int
dds_inq_data(union nusdset_t *ds, nusdims_t *dim,
		int item, void *buf, N_UI4 bufnelems)
{
	nusdfile_t *df;
	int r;
	df = dds_findfile(ds, dim, IO_READABLE);
	if (df == NULL) {
		return SETERR(NUSERR_RD_OpenFail);
	}
	r = df_inq_data(df, dim, item, buf, bufnelems);
	if (GlobalConfig(io_r_fclose)) {
		int	rc;
		rc = df_close(df);
		if (r == 0) {
			r = rc;
		}
	}
	return r;
}

	static int
dds_read_aux(union nusdset_t *ds, const nusdims_t *dim,
		sym4_t ttl, sym4_t grp,
		int (*callback)(const void *rec, N_UI4 siz, void *arg,
				union nusdset_t *dsetp, N_SI4 ofs_flg),
		void *arg)
{
	nusdfile_t *df;
	int r;
	df = dds_findfile(ds, dim, IO_READABLE);
	if (df == NULL) {
		if (NUS_ERRNO == NUSERR_OpenRFailed) {
			return SETERR(NUSERR_NoDfileToRead);
		}
		return NUS_ERRNO;
	}
	r = df_read_aux(df, ttl, grp, callback, arg, ds);
	if (GlobalConfig(io_r_fclose)) {
		int	rc;
		rc = df_close(df);
		if (r == 0) {
			r = rc;
		}
	}
	return r;
}

	static int
dds_write_aux(union nusdset_t *ds, const nusdims_t *dim,
		sym4_t ttl, sym4_t grp, size_t nbytes,
		int (*callback)(void *rec, N_UI4 siz,
			void *arg, union nusdset_t *dsetp),
		void *arg)
{
	nusdfile_t *df;
	int r;
	df = dds_findfile(ds, dim, IO_READWRITE);
	if (df == NULL) {
		return NUS_ERRNO;
	}
	r = df_write_aux(df, ttl, grp, nbytes, callback, arg, ds);
	if (GlobalConfig(io_w_fclose)) {
		int	rc;
		rc = df_close(df);
		if (r >= 0) {
			r = rc;
		}
	}
	return r;
}

/** @brief ds 内で dim で指定される記録を書き込む。
 */
static N_SI4
dds_writedata(union nusdset_t *ds, nusdims_t *dim, struct obuffer_t *buf)
{
	nusdfile_t *df;
	N_SI4 r;
	int ir;
	if (ds->comm.nrd >= 50) {
		return nus_err((-69, "NRD #%d is read only", ds->comm.nrd));
	}
	df = dds_findfile(ds, dim, IO_READWRITE);
	if (df == NULL) {
		return NUS_ERR_CODE();
	}
	buf->ob_mask = DynamicParam(&ds->comm.param, pc_mask_bit);
	r = df_write(df, dim, buf);
	nus_debug(("df_write => %Pd", r));
	if (GlobalConfig(io_w_fclose)) {
		ir = df_close(df);
		nus_debug(("df_close => %d", ir));
		if (ir != 0) {
			if (NUSERR_WR_Inconsistent == ir) return ir;
			return r ? r : ir;
		}
	} else if (GlobalConfig(io_mark_end)) {
		ir = df_flush(df);
		nus_debug(("df_flush => %d", ir));
		if (ir != 0) {
			return r ? r : ir;
		}
	}
	return r;
}

/** @brief データセットの定義ファイルに SUBC grp が許容されているか
 *
 * 仕様は xds_subc_namecheck() 参照 */
	static int
dds_subc_namecheck(union nusdset_t *ds, sym4_t grp, size_t size)
{
	struct dds_t *dds = &ds->dds;
	struct nusdef_subcinfo_t *si;
	int	i;
	if ((i = dds_endparse(dds)) != 0) {
		return i;
	}
	for (si = dds->def.subctab; si; si = si->next) {
		if (si->group == grp) {
			if ((size_t)si->size == size) {
				return 0;
			} else {
				return nus_err((NUSERR_SC_SizeMismatch,
					"size mismatch %d != %d",
					size, si->size));
			}
		}
	}
	return nus_err((NUSERR_SP_BadGroup,
		"SUBC %Ps not allowed on %#ys", grp, &ds->comm.nustype));
}

	static int
dds_inq_aux(union nusdset_t *ds,
		const nusdims_t *dim,
		int query,
		sym4_t grp,
		void *buf,
		N_UI4 bufnelems)
{
	nusdfile_t *df;
	int r;
	df = dds_findfile(ds, dim, IO_READABLE);
	if (df == NULL) {
		return SETERR(NUSERR_NoDfileToRead);
	}
	r = df_inq_aux(df, query, grp, buf, bufnelems);
	if (GlobalConfig(io_r_fclose)) {
		int	rc;
		rc = df_close(df);
		if (r == 0) {
			r = rc;
		}
	}
	return r;
}

	static int
dds_inq_cntl(union nusdset_t *ds,
		const nusdims_t *dim,
		int query,
		void *buf,
		void *arg,
		int keep_open)
{
	nusdfile_t *df;
	int r;
	nus_debug(("---- dds_inq_cntl %p", ds));
	df = dds_findfile(ds, dim, IO_READABLE);
	if (df == NULL) {
		return SETERR(NUSERR_NoDfileToRead);
	}
	r = df_inq_cntl(df, query, buf, arg);
	if (GlobalConfig(io_r_fclose) && !keep_open) {
		int	rc;
		rc = df_close(df);
		if (r == 0) {
			r = rc;
		}
	}
	return r < 0 ? 0 : r;
}

	static int
dds_inq_grid(union nusdset_t *ds,
		struct inq_grid_info *info)
{
	N_SI4 size;
	int r;

	size = 1;
	r = ds_inq_cntl(ds, &info->nusdims, N_PROJECTION, info->proj, &size, 1);
	if (r >= 0) {
		size = 2;
		r = ds_inq_cntl(ds, &info->nusdims,
				N_GRID_SIZE, info->gridsize, &size, 1);
	}
	if (r >= 0) {
		size = 4;
		r = ds_inq_cntl(ds, &info->nusdims,
				N_GRID_BASEPOINT, info->gridinfo, &size, 1);
	}
	if (r >= 0) {
		size = 2;
		r = ds_inq_cntl(ds, &info->nusdims,
				N_GRID_DISTANCE, info->gridinfo + 4, &size, 1);
	}
	if (r >= 0) {
		size = 4;
		r = ds_inq_cntl(ds, &info->nusdims,
				N_STAND_LATLON, info->gridinfo + 6, &size, 1);
	}
	if (r >= 0) {
		size = 4;
		r = ds_inq_cntl(ds, &info->nusdims,
				N_SPARE_LATLON, info->gridinfo + 10, &size, 1);
	}
	if (r >= 0) {
		size = 1;
		r = ds_inq_cntl(ds, &info->nusdims,
				SYM4_VALU, info->value, &size, 1);
	}
	return r < 0 ? 0 : r;
}

/** @brief 単データセットを閉じた結果情報 */
struct DDS_Close_Info {
	int	flags; /**< 閉じるファイルの種類を選ぶフラグ */
	int	ok; /**< 閉じたデータファイル数 */
	int	ng; /**< 閉じる際にエラーになったデータファイル数 */
};

/** @brief 単データセットを閉じるために dds_close() から呼ばれる
 */
static int
DDS_Close_Callback(char *filename UNUSED, nusdfile_t *df, void *arg)
{
	struct DDS_Close_Info *info = arg;
	int r;
	if (!df->comm.is_open) {
		return 0;
	}
	switch (info->flags) {
		case N_FOPEN_ALL:
			break;
		case N_FOPEN_READ:
			if (df_is_writable(df))
				return 0;
			break;
		case N_FOPEN_WRITE:
			if (! df_is_writable(df))
				return 0;
			break;
		default:
			return 0;
	}
	nus_debug(("df_close(%s)", df->comm.filename));
	r = df_close(df);
	if (r) {
		info->ng--;
	} else {
		info->ok++;
	}
	return 0;
}

/** @brief データセット ds を閉じる。
 *
 * @note ファイルを閉じるだけでデータセット構造自体は破壊しない。
 */
static int
dds_close(nusdset_t *ds, int flags)
{
	struct dds_t	*dds = &ds->dds;
	struct DDS_Close_Info	info;
	info.flags = flags;
	info.ok = info.ng = 0;
	dds_dftab_each(dds->dftab,
			DDS_Close_Callback, &info);
	return info.ng ? info.ng : info.ok;
}

static int
dds_delete(nusdset_t *ds)
{
	/* fake */
	return dds_close(ds, N_FOPEN_ALL);
}

static int
DDS_Compact_Callback(union nusdfile_t *df)
{
	if (df_is_closed(df)) {
		df_delete(df);
		return 1;
	} else {
		return 0;
	}
}

static int
dds_compact(nusdset_t *ds)
{
	struct dds_t	*dds = &ds->dds;
	dds_dftab_reject(dds->dftab, DDS_Compact_Callback);
	return 0;
}

/** @brief データセット ds の次元 dim で指示されるファイルを閉じる。
 *
 * @note ファイルを閉じるだけでデータセット構造自体は破壊しない。
 */
static int
dds_close_file(nusdset_t *ds, const nusdims_t *dim)
{
	union nusdfile_t *df;
	int r;
	df = dds_findfile(ds, dim, 0);
	if (df == NULL) {
		return NUS_ERR_CODE();
	}
	r = df_close(df);
	return r;
}

/** 定義ファイル型データセットの関数テーブル.
 */
static struct ds_functab dds_methods = {
	dds_close,
	dds_delete,
	dds_compact,
	dds_findfile,
	dds_readdata,
	dds_inq_data,
	dds_writedata,
	dds_read_aux,
	dds_subc_namecheck,
	nusdds_btlist,
	nusdds_vtlist,
	dds_inq_aux,
	dds_inq_cntl,
	dds_write_aux,
	dds_close_file,
	dds_inq_def,
	dds_inq_grid
};

/** @brief dds_t 構造体の初期化。
 */
static struct dds_t *
dds_init(const char *dirname, int nrd)
{
	struct dds_t *dds;
	dds = nus_malloc(sizeof(struct dds_t));
	/* dds->comm.nustype は不定値 */
	dds->comm.nrd = nrd;
	dds->comm.methods = dds_methods;
	dds->comm.sc_eta = dds->comm.sc_sigm = NULL;
	dds->comm.sc_rgau = dds->comm.sc_zhyb = NULL;
	dds->comm.sc_delt = NULL;
	dds->dirname = string_dup(dirname);
	dds->tmpl.fullpath = NULL;
	dds->dftab = dds_dftab_ini(127);
	memset(&dds->comm.param, '\0', sizeof dds->comm.param);
	dds->comm.dead_flag = 0;
	return dds;
}

/** @brief DDS を開く。
 *
 * ルートディレクトリ dirname, 定義ファイル deffile で
 * 特定される定義ファイル型データセットを構築する。
 */
nusdset_t *
dds_open(const char *dirname, const char *deffile, int nrd)
{
	struct dds_t	*dds;
	if ((dds = dds_init(dirname, nrd)) == NULL) {
		return NULL;
	}
	nusdef_init(&(dds->def));
	if (nusdef_readfile(deffile, &(dds->def)) != 0) {
		nus_free(dds);
		return NULL;
	}
	dds->comm.nustype = dds->def.nustype;
	return (nusdset_t *)dds;
}
