/** @file
 * @brief NuSDaS 形式データファイル (NDF) の処理
 */

#include "config.h"
#include "nusdas.h"
#include <sys/types.h>
#include "internal_types.h"
#include <stdlib.h>
#include <string.h>
#include "sys_endian.h"
#include "sys_mem.h"
# define NEED_MEMCPY4
# define NEED_MEMCPY6
# define NEED_MEMCPY8
# define NEED_MEM6SYM8
# define NEED_STRING_DUP
#include "sys_string.h"
#include "sys_sym.h"
# define NEED_MAKE_UI8
# define NEED_ULONG_TO_UI8
# define NEED_ULONG_TO_SI8
# define NEED_LONG_TO_SI8
# define NEED_SI8_ADDTO_UI4
# define NEED_SI8_SUB
# define NEED_SI8_CMP
# define NEED_SI8_ADD
#include "sys_int.h"
#include "sys_kwd.h"
#include "sys_time.h"
#include "sys_container.h"
#include "io_comm.h"
#include "dset.h"
#include "dfile.h"
#include "ndf_codec.h"
#include "glb.h"
#include "ndf.h"
#include "sys_err.h"
#include <time.h>

#define NUSD_SIZE	120
#define NDF_CNTL_SIZE(ndf) \
	(4 * (43 + (ndf)->nm + (ndf)->nv * 2 + 1) \
	 + 6 * ((ndf)->nz * 2 + (ndf)->ne))

const int err_mvze_write[4] = {
	NUSERR_WR_BadMember,
	NUSERR_BadVtime,
	NUSERR_WR_BadPlane,
	NUSERR_WR_BadElem
};

const int err_mvze_read[4] = {
	NUSERR_RD_BadMember,
	NUSERR_BadVtime,
	NUSERR_RD_BadPlane,
	NUSERR_RD_BadElem
};

	N_UI4
nusndf_recl(struct ndf_t *ndf, N_UI4 vrecl)
{
	long roundup;
	if (ndf->forcedrlen == 0) {
		if (ndf_is_largefile(ndf)) {
			vrecl += 7u;
			vrecl = vrecl & ~(N_UI4)7u;
			return vrecl;
		} else {
			return vrecl;
		}
	}
	/* @warning 固定長の場合記録長は 4 の倍数に切り上げられる.
	 * これが嫌なら nusdcntl というキャッシュ機構自体を替える必要がある */
	roundup =  (ndf->forcedrlen + 3) & ~3L;
	if (roundup < (long)vrecl) {
		nus_err((NUSERR_ForcedRlen, "forcedrlen %ld < %Pu",
					ndf->forcedrlen, vrecl));
		return 0;
	}
	return roundup;
}

	INLINE N_UI4
ndfclose_filesize(struct ndf_t *ndf, N_UI4 endsize)
{
	N_UI8	fs;
	fs = ndf->wpos;
	si8_addto_ui4(fs, endsize);
	return I8_HI(fs) ? 0 : I8_LO(fs);
}

	static int
ndf_flush_subc(sym4_t grp UNUSED, struct ndf_aux_t *subc, void *arg)
{
	struct ndf_t *ndf = arg;
	N_UI4 frecl;
	if (subc->au_data == NULL) {
		nus_debug(("subc %Ps unchanged", grp));
		return 0;
	}
	if ((frecl = nusndf_recl(ndf, subc->au_recl)) == 0) {
		return NUS_ERRNO;
	}
	if (0 != nusndf_check_format(subc->au_data, frecl,
	  ndf->seqf.sf_reclplus)) {
		return NUS_ERRNO;
	}
	nus_debug(("ndf_flush_subc %Ps", grp));
	return nusnsf_write(&ndf->seqf, subc->au_pos, frecl, subc->au_data);
}

	static int
ndf_flush_info(sym4_t grp UNUSED, struct ndf_aux_t *info, void *arg)
{
	struct ndf_t *ndf = arg;
	N_UI4 frecl;
	int r;
	if (info->au_data == NULL) {
		nus_debug(("info %Ps unchanged", grp));
		return 0;
	}
	if ((frecl = nusndf_recl(ndf, info->au_recl)) == 0) {
		return NUS_ERRNO;
	}
	if (i8_zerop(info->au_pos)) {
		info->au_pos = ndf->wpos;
		si8_addto_ui4(ndf->wpos, frecl);
	}
	nus_debug(("ndf_flush_info %Ps at %Qd", grp, info->au_pos));
	r = nusnsf_write(&ndf->seqf, info->au_pos, frecl, info->au_data);
	return r;
}

struct nusndf_makeend_param {
	sym4_t titl;
	N_UI4 *entry;
};

	static int
ndf_makeend_aux(sym4_t grp, struct ndf_aux_t *info, void *arg)
{
	struct nusndf_makeend_param *param = arg;
	param->entry[0] = param->titl;
	param->entry[1] = grp;
	param->entry[2] = HTON4(I8_HI(info->au_pos));
	param->entry[3] = HTON4(I8_LO(info->au_pos));
	param->entry[4] = HTON4(info->au_recl);
	param->entry[5] = 0;
	param->entry += 6;
	return 0;
}

	static N_UI4 *
ndf_makeend(struct ndf_t *ndf)
{
	struct nusndf_makeend_param param;
	time_t	now;
	N_UI4 *endrec;
	N_UI4 recl, vrecl;
	int r;

	if (nus_dbg_enabled && GlobalConfig(no_timestamp)) {
		now = 0;
	} else {
		time(&now);
	}
	recl = 4 * 7;
	if (ndf_is_largefile(ndf)) {
		recl += (ndf->subccount + ndf->infocount) * 6 * 4;
	}
	if ((recl = nusndf_recl(ndf, vrecl = recl)) == 0) {
		return NULL;
	}
	if ((endrec = nus_malloc(recl)) == NULL) {
		nus_err((NUSERR_MemShort, "memshort"));
		return NULL;
	}
	endrec[0] = HTON4(recl - ndf->seqf.sf_reclplus);
	endrec[1] = SYM4_END;
	endrec[2] = HTON4(vrecl - 8);
	endrec[3] = HTON4(now);
	endrec[4] = HTON4(ndfclose_filesize(ndf, recl));
	endrec[5] = HTON4(ndf->reccount);
	param.entry = endrec + 6;
	if (ndf_is_largefile(ndf)) {
		if (ndf->subctab) {
			param.titl = SYM4_SUBC;
			ndf_auxtab_each(ndf->subctab, ndf_makeend_aux, &param);
		}
		if (ndf->infotab) {
			param.titl = SYM4_INFO;
			ndf_auxtab_each(ndf->infotab, ndf_makeend_aux, &param);
		}
	}
	endrec[(recl - 4) >> 2] = endrec[0];

	/** その場しのぎ: 本当は WRITABLE に変わったところで
	 * wpos を確定すべき。 */
	if (ui8_maxp(ndf->wpos)) {
		nus_debug(("END skipped"));
		return endrec;
	}

	r = nusnsf_write(&ndf->seqf, ndf->wpos, recl, endrec);
	if (r) {
		nus_err((NUSERR_ENDWriteFail, "nusnsf_write => %d", r));
		nus_free(endrec);
		return NULL;
	}

	return endrec;
}

	static int
ndfclose_writehead(struct ndf_t *ndf)
{
	int r;
	N_UI4	*endrec, *nusd, *cntl, *indx;
	nusd = (N_UI4 *)ndf->nusd;
	cntl = (N_UI4 *)ndf->cntl;
	indx = (N_UI4 *)ndf->indx;

	if (cntl[17] == SYM4_XX) {
		return nus_err((NUSERR_BadGrid, "don't leave projection XX"));
	}

	if (ndf->subctab) {
		r = ndf_auxtab_each(ndf->subctab, ndf_flush_subc, ndf);
		if (r) goto Hell;
	}
	if (ndf->infotab) {
		r = ndf_auxtab_each(ndf->infotab, ndf_flush_info, ndf);
		if (r) goto Hell;
	}

	endrec = ndf_makeend(ndf);
	if (endrec == NULL) {
		r = NUS_ERRNO;
		goto Hell;
	}

	/** @todo CNTL 記録の時刻はアップデートすべきでない */
	nusd[3] = indx[3] = endrec[3];
	cntl[3] = ((ndf->cntltime == 0xFFFFFFFFul) ? nusd[3] : ndf->cntltime);
	/* 8バイトファイル長を書き込んでおく */
	N_UI4 endsize = NTOH4(endrec[0]) + ndf->seqf.sf_reclplus;
	N_SI8 filesize = ndf->wpos;
	si8_addto_ui4(filesize, endsize);
	ndf->totalsize = filesize;
	if (ndf_is_largefile(ndf)) {
		nusd[22] = NTOH4(I8_HI(filesize));
		nusd[23] = NTOH4(I8_LO(filesize));
	}
	nusd[25] = endrec[4];
	nusd[26] = endrec[5];
	nusd[27] = HTON4(ndf->infocount);
	nusd[28] = HTON4(ndf->subccount);

	nus_free(endrec);

	r = nusnsf_rewind(&ndf->seqf);
	if (r) goto Hell;
	r = nusnsf_write_seq(&ndf->seqf, ndf->nusdsize, ndf->nusd);
	if (r) {
		r = nus_err((NUSERR_NUSDWriteFail, 
			     "Failed to write NUSD in closing"));
		goto Hell;
	}
	r = nusnsf_write_seq(&ndf->seqf, ndf->cntlsize, ndf->cntl);
	if (r) {
		r = nus_err((NUSERR_CNTLWriteFail, 
			     "Failed to write CNTL in closing"));
		goto Hell;
	}
	r = nusnsf_write_seq(&ndf->seqf, ndf->indxsize, ndf->indx);
	if (r) {
		r = nus_err((NUSERR_INDXWriteFail, 
			     "Failed to write INDX in closing"));
		goto Hell;
	}
Hell:
	if (r) {
		nus_debug(("ndfclose_writehead => %d", r));
	}
	return r;
}

	int
nusndf_flushwrite(struct ndf_t *ndf)
{
	int r;
	NUSPROF_BUF_DECL;

	if ((ndf->wbuf == NULL) || (ndf->wbuf->pos == 0)) {
		return 0;
	}
	NUSPROF_BACKUP;
	NUSPROF_MARK(NP_WRITE);
	r = nusnsf_issue(&ndf->seqf, ndf->wbuf->ofs, ndf->wbuf->pos);
	NUSPROF_RESTORE;
	nus_debug(("nusnsf_issue(%Pu, %Pu) => %d", ndf->wbuf->ofs,
				ndf->wbuf->pos, r));
	if (r != 0) {
		return nus_err((NUSERR_IO, "issue failed"));
	}
	ndf->wbuf->pos = 0;
	return 0;
}

	static int
ndf_flush(nusdfile_t *df)
{
	int r = 0;
	int rr;
	if (df->ndf.wbuf) {
		r = nusndf_flushwrite(&df->ndf);
	}
	rr = ndfclose_writehead(&df->ndf);
	return rr ? rr : r;
}

	static int
ndf_close(nusdfile_t *df)
{
	int r;
	int rr;
	static int Counter = 0;
	if (!df->comm.is_open) {
		return 0;
	}
	if (df_is_writable(df)) {
		r = 0;
		if (df->ndf.wbuf) {
			r = nusndf_flushwrite(&df->ndf);
		}
		if ((rr = ndfclose_writehead(&df->ndf)) != 0) {
			r = rr;
		}
	} else {
		r = 0;
	}
	if ((rr = nusnsf_close(&df->ndf.seqf, df->ndf.totalsize, df->comm.filename)) != 0) {
		r = rr;
	}
	df->ndf.totalsize = 0;
	if ((GlobalConfig(pc_keep_closed_file) >= 0)
			&& (++Counter >= GlobalConfig(pc_keep_closed_file))) {
		nusglb_garbage_collect();
		Counter = 0;
	}
	df->comm.is_open = 0;
	return r;
}

	static int
ndf_delete(nusdfile_t *df)
{
	struct ndf_t *ndf = &df->ndf;
	int r;
	r = ndf_close(df);
	if (ndf->btime)
		array4_delete(ndf->btime);
	if (ndf->member)
		array4_delete(ndf->member);
	if (ndf->vtime)
		array8_delete(ndf->vtime);
	if (ndf->plane)
		array8_delete(ndf->plane);
	if (ndf->element)
		array8_delete(ndf->element);
	if (ndf->indx)
		nus_free(ndf->indx);
	if (ndf->nusd)
		nus_free(ndf->nusd);
	if (ndf->cntl)
		nus_free(ndf->cntl);
	if (ndf->wbuf) {
		/* wbuf->ptr は io 層で管理されているので開放しない */
		nus_free(ndf->wbuf);
	}
	if (ndf->subctab)
		ndf_auxtab_delete(ndf->subctab);
	if (ndf->infotab)
		ndf_auxtab_delete(ndf->infotab);
	nus_free(df);
	return r;
}

INLINE int
ndf_set_codec(struct ndf_t *ndf, N_UI4 packing, N_UI4 missing, N_UI4 buffmt)
{
	if (ndf->codec && ndf->codec->packing == packing
			&& ndf->codec->missing == missing
			&& ndf->codec->bffm == buffmt) {
		return 0;
	}
	ndf->codec = ndf_get_codec(packing, missing, buffmt);
	if (ndf->codec == NULL) {
		/* エラーコード下位ビットは read 用の値を持ち、
		 * リターンコードは write 用の値 */
		if (packing == SYM4_RLEN
			&& (missing == SYM4_UDFV || missing == SYM4_MASK)) {
			nus_err((NUSERR_RD_BadPackMiss,
				"conflicting packing=%Ps and missing=%Ps",
				packing, missing));
			return NUSERR_WR_BadPackMiss;
		} else {
			nus_err((NUSERR_RD_NoCodec,
				"missing codec <%Ps|%Ps|%Ps>",
				packing, missing, buffmt));
			return NUSERR_WR_NoCodec;
		}
	}
	return 0;
}

/** @brief 次元から記録番号への換算
 * @retval ~(size_t)0 エラー
 * @retval 他 INDX 配列の引数
 */
	INLINE size_t
ndf_dim2irec(struct ndf_t *ndf, nusdims_t *dim, const int err_mvze[4])
{
	size_t recno;
	N_UI8	vt;
	int	im, iv, iz, ie;
	const size_t error = ~(size_t)0;

	im = array4_get_index(ndf->member, dim->member);
	nus_debug(("member [%Ps] => %d", dim->member, im));
	vt = make_ui8(dim->validtime1, dim->validtime2);
	iv = array8_get_index(ndf->vtime, vt);
	nus_debug(("validtime [%#T, %#T] => %d", dim->validtime1,
			dim->validtime2, iv));
	iz = array8_get_index(ndf->plane, dim->plane1);
	nus_debug(("plane [%Qs, %Qs] => %d", dim->plane1, dim->plane2, iz));
	ie = array8_get_index(ndf->element, dim->element);
	nus_debug(("element [%Qs] => %d", dim->element, ie));

	if (im < 0) {
		nus_err((err_mvze[0],
				"member \"%Ps\" not found in %#ys",
				dim->member,
				&ndf->comm.nustype));
		return error;
	} else if (iv < 0) {
#if 0
		unsigned i;
		for (i = 0; i < ndf->nv; i++) {
			N_UI8	vt;
			vt = array8_get_value(ndf->vtime, i);
			nus_err((0, "available: %QT", vt));
		}
#endif
		nus_err((err_mvze[1],
				"validtime \"%T/%T\" not found in %#ys",
				dim->validtime1,
				dim->validtime2,
				&ndf->comm.nustype));
		return error;
	} else if (iz < 0) {
		nus_err((err_mvze[2],
				"plane \"%Qs\"/\"%Qs\" not found in %#ys",
				dim->plane1, dim->plane2,
				&ndf->comm.nustype));
		return error;
	} else if (ie < 0) {
		nus_err((err_mvze[3],
				"element \"%Qs\" not found in %#ys",
				dim->element,
				&ndf->comm.nustype));
		return error;
	}

	recno = im;
	recno *= ndf->nv;
	recno += iv;
	recno *= ndf->nz;
	recno += iz;
	recno *= ndf->ne;
	recno += ie;
	return recno;
}

	INLINE size_t
ndf_index_setpos(struct ndf_t *ndf, nusdims_t *dim, N_UI4 frecl, N_UI8 *wpos, 
		 N_UI4 *osize, int *wpos_ndf)
{
	size_t recno;
	N_UI8 pos, old_ofs;
	N_UI4 old_size;

	/* 記録番号を決定 */
	recno = ndf_dim2irec(ndf, dim, err_mvze_write);
	if (~recno == 0) {
		return recno;
	}
	/* インデックスの記録番号のところにファイル位置を書き込む */
	if (ndf->indx4) {
		if (~ndf->indx4[recno] == 0) {
			nus_err((NUSERR_WR_Elementmap,
				"elementmap bans record(%#ys:%PT:%Qs:%Qs)",
				&ndf->comm.nustype,
				dim->validtime1, dim->plane1, dim->element));
			return ~(size_t)0;
		} else if (ndf->indx4[recno] != 0) {
			old_ofs = long_to_si8(NTOH4(ndf->indx4[recno]));
			nusnsf_read_recl(&ndf->seqf, old_ofs, &old_size);
			if (old_size < frecl) {
				pos = ndf->wpos;
				*wpos_ndf = 1;
				nus_debug(("appended(%#ys:%PT:%Qs:%Qs)", 
					   &ndf->comm.nustype,
					   dim->validtime1, 
					   dim->plane1, dim->element));
			} else {
				pos = old_ofs;
				*wpos_ndf = 0;
				nus_debug(("overwritten(%#ys:%PT:%Qs:%Qs)", 
					   &ndf->comm.nustype,
					   dim->validtime1, 
					   dim->plane1, dim->element));
			}
		} else {
			pos = ndf->wpos;
			*wpos_ndf = 1;
		}
		if (I8_HI(pos)) {
			nus_err((NUSERR_Over4GiB,
				"file size exceeds 4GiB (%s)",
				ndf->comm.filename));
			return ~(size_t)0;
		}
		ndf->indx4[recno] = HTON4(I8_LO(pos));
	} else {
		N_SI8	*indx8 = (N_SI8 *)(ndf->indx + 16);
		if (ui8_maxp(indx8[recno])) {
			nus_err((NUSERR_WR_Elementmap,
				"elementmap bans record(%#ys:%PT:%Qs:%Qs)",
				&ndf->comm.nustype,
				dim->validtime1, dim->plane1, dim->element));
			return ~(size_t)0;
		} else if (!si8_iszero(indx8[recno])) {
			old_ofs = NTOH8(indx8[recno]);		
			old_size = NTOH4(ndf->indy_recl[recno]);
			if (old_size < frecl) {
				pos = ndf->wpos;
				*wpos_ndf = 1;
				nus_debug(("appended(%#ys:%PT:%Qs:%Qs)", 
					   &ndf->comm.nustype,
					   dim->validtime1, 
					   dim->plane1, dim->element));
			} else {
				pos = old_ofs;
				*wpos_ndf = 0;
				nus_debug(("overwritten(%#ys:%PT:%Qs:%Qs)", 
					   &ndf->comm.nustype,
					   dim->validtime1, 
					   dim->plane1, dim->element));
			}
		} else {
			pos = ndf->wpos;
			*wpos_ndf = 1;
		}
		indx8[recno] = HTON8(pos);
	}
	*wpos = pos;
	*osize = old_size;
	/* 御報告 */
	nus_debug(("rec #%u at %Qu", (unsigned)recno, pos));
	if (*wpos_ndf == 1) {
		ndf->reccount++;
	}
	return recno;
}

	INLINE size_t
ndf_index_killpos(struct ndf_t *ndf, nusdims_t *dim)
{
	size_t recno;

	/* 記録番号を決定 */
	recno = ndf_dim2irec(ndf, dim, err_mvze_write);
	if (~recno == 0) {
		return recno;
	}
	/* インデックスの記録番号のところのファイル位置を未処理に */
	if (ndf->indx4) {
		ndf->indx4[recno] = 0;
	} else {
		N_SI8	*indx8 = (N_SI8 *)(ndf->indx + 16);
		indx8[recno] = long_to_si8(0);
	}
	/* 御報告 */
	nus_debug(("rec #%u cancelled", (unsigned)recno, ndf->wpos));
	/* バグと思われるので削除
	ndf->reccount--;
	*/
	return recno;
}

	INLINE void
ndf_index_setsize(struct ndf_t *ndf, size_t recno, N_UI4 recl, N_UI4 nelems)
{
	if (ndf->indx4)
		return;
	ndf->indy_recl[recno] = HTON4(recl);
	ndf->indy_nelems[recno] = HTON4(nelems);
}

static int
ndf_write(nusdfile_t *df,
		nusdims_t *dim,
		struct obuffer_t *buf)
{
	struct ndf_t *ndf = &(df->ndf);
	char *datarec, *p;
	char *ecdbuf = NULL;
	size_t recl, recno, ebufsiz;
	N_UI4	frecl, old_size;
	N_UI4	packing;
	N_UI4	nx, ny;
	time_t	now;
	long re;
	int r;
	int wpos_ndf;
	N_UI8 wpos;
	NUSPROF_BUF_DECL;


	if (!(df->comm.flags & IO_WRITABLE)) {
		return nus_err((NUSERR_DfileUnwritable,
					"write to readonly file"));
	}

	nx = DynamicParam(df->comm.ds_param, pc_sizex);
	ny = DynamicParam(df->comm.ds_param, pc_sizey);
	if (nx == 0) {
		nx = ndf->nx;
	}
	if (ny == 0) {
		ny = ndf->ny;
	}
	if (SYM4_ND == buf->ob_fmt) {
	} else if (buf->nelems < nx * ny) {
		return nus_err((NUSERR_WR_SmallBuf,
				"buffer(%Pd) < DS grid(%Pdx%Pd = %Pd)",
				buf->nelems, nx, ny, nx * ny));
	} else if (buf->nelems > nx * ny) {
		nus_warn(("buffer(%Pd) > DS grid(%Pdx%Pd = %Pd)",
				buf->nelems, nx, ny, nx * ny));
		buf->nelems = nx * ny;
	}

	packing = DynamicParam(df->comm.ds_param, pc_packing);
	if (packing == 0) {
		packing = ndf->packing;
	}
	r = ndf_set_codec(ndf, packing, ndf->missing, buf->ob_fmt);
	if (r != 0) {
		SETERR(r);
		goto Hell;
	}
	if (ndf->codec->encode == NULL) {
		return nus_err((NUSERR_WR_NoCodec, "missing codec <%Ps|%Ps|%Ps>",
			packing, ndf->missing, buf->ob_fmt));
	}

	/* 悲観的記録長 (圧縮後の最大長) */
	ebufsiz = nusndf_recl(ndf, CODEC_MAXRECL(ndf->codec, buf->nelems));
	if (ebufsiz == 0) {
		goto Hell;
	}
	ecdbuf = nus_malloc(ebufsiz);
	if(!ecdbuf) return nus_err((NUSERR_MemShort, "alloc ecdbuf failed"));
	NUSPROF_BACKUP;
	NUSPROF_MARK(NP_ENCODE);
	re = ndf->codec->encode((unsigned char *)ecdbuf + 64, buf, nx, ny);
	NUSPROF_RESTORE;
	if (re < 0) {
		if(re == NUSERR_WR_MaskMissing){
			nus_err((NUSERR_WR_MaskMissing, "need mask bit"));
		} else if (re == NUSERR_WR_SmallBuf){
			nus_err((NUSERR_WR_SmallBuf, "size invalid"));
		} else {
			nus_err((NUSERR_WR_EncodeFail, "encode failed"));
		}
		goto DeleBufDie;
	}
	recl = (size_t)re + 64;

	if ((frecl = nusndf_recl(ndf, recl)) == 0) {
		/* エラーコードは強制設定しているけど、なぜか不明 */
		SETERR(NUSERR_ForcedRlen);
DeleBufDie:
		if (ndf->wbuf == NULL) {
			nus_free(ecdbuf);
			ecdbuf = NULL;
		}
		ndf_index_killpos(ndf, dim);
Hell:
		if(ecdbuf){
			nus_free(ecdbuf);
			ecdbuf = NULL;
		}
		return NUS_ERRNO;
	}

	recno = ndf_index_setpos(ndf, dim, frecl, &wpos, &old_size, &wpos_ndf);
	if (~recno == 0) {
		nus_free(ecdbuf);
		return NUS_ERRNO;
	}
	if (wpos_ndf == 0) {
		nus_debug(("frecl => %Pd, old_size => %Pd", frecl, old_size));
		frecl = old_size;
	}
	/* バッファリングの有無とバッファ割り付け状態に応じて対応 */
	if (ndf->wbuf == NULL) {
		if (ebufsiz < frecl) {
			if ((p = (char*)nus_realloc(ecdbuf, frecl)) == NULL) {
				nus_free(ecdbuf);
				return nus_err((NUSERR_MemShort, 
					       "realloc ecdbuf failed"));
			}
			ebufsiz = frecl;
			ecdbuf = p;
			
		}
		datarec = ecdbuf;
	} else if (wpos_ndf == 1 && (ndf->wbuf->ptr && ndf->wbuf->pos
			&& (ndf->wbuf->pos + frecl < ndf->wbuf->siz))) {
		datarec = (char *)ndf->wbuf->ptr + ndf->wbuf->pos;
		memcpy(datarec, ecdbuf, recl);
		nus_free(ecdbuf);
		nus_debug(("wbuf(pos=%Pd) reused wpos=%Qd",
			ndf->wbuf->pos, ndf->wpos));
	} else {
		if (nusndf_flushwrite(ndf) != 0) {
			nus_err((NUSERR_IO, "flush failed"));
			goto Hell;
		}
		if (ndf->wbuf->siz < frecl) {
			ndf->wbuf->siz = frecl;
		}
		ndf->wbuf->ptr = nusnsf_getwbuf(&ndf->seqf, wpos,
				ndf->wbuf->siz);
		nus_debug(("nusnsf_getwbuf(%Qu, %Pu) => %Px",
			   wpos, ndf->wbuf->siz,
			   (N_UI4)(size_t)ndf->wbuf->ptr));
		if (ndf->wbuf->ptr == NULL) {
			nus_err((NUSERR_IO, "nusnsf_getwbuf ERR"));
			goto Hell;
		}
		ndf->wbuf->pos = 0;
		ndf->wbuf->ofs = wpos;
		datarec = (char *)ndf->wbuf->ptr;
		memcpy(datarec, ecdbuf, recl);
		nus_free(ecdbuf);
	}
	ndf_index_setsize(ndf, recno, frecl, buf->nelems);
	POKE_N_UI4(datarec + 0, frecl - ndf->seqf.sf_reclplus);
	memcpy4(datarec  + 4, "DATA");
	POKE_N_UI4(datarec + 8, recl - 8);
	if (nus_dbg_enabled && GlobalConfig(no_timestamp)) {
		now = 0;
	} else {
		time(&now);
	}
	POKE_N_UI4(datarec + 12, now);
	memcpy4(datarec + 16, (char *)&(dim->member));
	POKE_N_UI4(datarec + 20, dim->validtime1);
	POKE_N_UI4(datarec + 24, dim->validtime2);
	memcpy6(datarec  + 28, (char *)&(dim->plane1));
	memcpy6(datarec  + 34, (char *)&(dim->plane2));
	memcpy8(datarec  + 40, (char *)&(dim->element));
	datarec[46] = datarec[47] = '\0';
	POKE_N_UI4(datarec + 48, nx);
	POKE_N_UI4(datarec + 52, ny);
	memcpy4(datarec  + 56, (char *)&packing);
	memcpy4(datarec  + 60, (char *)&(ndf->missing));
	/* 記録末尾 */
	if (frecl > recl) {
		memset(datarec + recl - 4, '\0', frecl - recl);
	}
	memcpy4(datarec + frecl - 4, datarec);

	if (ndf->wbuf == NULL) {
		NUSPROF_BACKUP;
		NUSPROF_MARK(NP_WRITE);
		r = nusnsf_write(&ndf->seqf, wpos, frecl, datarec);
		NUSPROF_RESTORE;
		nus_free(datarec);
		if (r != 0) {
			int rr;
			rr = (r >= -2) ? -99 : r;
			return nus_err((rr, "write failed: %d", r));
		}
	} else {
		ndf->wbuf->pos += frecl;
	}
	if (wpos_ndf == 1) {
		si8_addto(ndf->wpos, ulong_to_si8(frecl));
	}
	return buf->nelems;
}

/** @brief インデックス記録を読む
 * @retval 0 正常終了
 * @retval 他 エラー (返却値はエラーコード)
 * @todo 要素数チェック
 */
	INLINE int
ndf_index_read(struct ndf_t *ndf, /**< データファイル */
		nusdims_t *dim, /**< 記録を指示する次元 */
		N_UI8 *ofs, /**< バイトオフセットが返される変数を指す */
		N_UI8 *size) /**< 記録長 */
{
	size_t recno = ndf_dim2irec(ndf, dim, err_mvze_read);
	if (~recno == 0) {
		return NUS_ERRNO;
	}
	if (ndf->indx4) {
		if (~ndf->indx4[recno] == 0) {
			return nus_err((NUSERR_RD_IndexKills,
				"record %zd(%#ms) disabled in INDX",
				recno, dim));
		}
		*ofs = long_to_si8(NTOH4(ndf->indx4[recno]));
	} else {
		N_UI8 *headtab = (N_UI8 *)(ndf->indx + 16);
		if (ui8_maxp(headtab[recno])) {
			return nus_err((NUSERR_RD_IndexKills,
				"record %zd(%#ms) disabled in INDY",
				recno, dim));
		}
		*ofs = NTOH8(headtab[recno]);
	}
	if (i8_zerop(*ofs)) {
		return nus_err((NUSERR_RD_NoDrec,
			"record %zd(%#ms) not found in INDX", recno, dim));
	}
	if (ndf->indx4) {
/**
 * @note ファイルバージョン 11 以前では記録長がインデックスに書かれて
 * いないため、推定最悪値 (データファイル CNTL 記録に由来する格子数を
 * packing=R8, missing=MASK で格納した場合の記録長) が返される。
 */
		unsigned long nxy;
		unsigned long ulsize;
		nxy = ndf->nx * ndf->ny;
		ulsize = nxy * 8 + ((nxy - 1) / 8 + 1) + 64 + 4;
		*size = long_to_si8(ulsize);
	} else {
		*size = make_ui8(0, NTOH4(ndf->indy_recl[recno]));
	}
	nus_debug(("recno=%Pd ofs=%Qd size=%Qd (%#ms)", recno, *ofs, *size, dim));
	return 0;
}

	INLINE void *
ndf_load_data(struct ndf_t *ndf, nusdims_t *dim, N_UI8 *ofs)
{
	N_UI8 siz8;
	N_UI4 *rec4;
	void *rec;
	/** @note 書き込みが完結しないのに読み始めたら一旦フラッシュ */
	if (nusndf_flushwrite(ndf) != 0) {
		nus_err((NUSERR_IO,
			"nusndf_flush failed before reading"));
		return NULL;
	}

	if (ndf_index_read(ndf, dim, ofs, &siz8) != 0) {
		return NULL;
	}

	rec4 = rec = nusnsf_read_at(&ndf->seqf, *ofs, siz8);
	if (rec == NULL) {
		nus_err((NUSERR_RD_NoDrec, "nusnsf_read_at(%Qu, %Qu) => NULL",
					*ofs, siz8));
		return NULL;
	}

	if (rec4[1] != SYM4_DATA) {
		nus_err((NUSERR_DATAReadFail, "record %#Ps != DATA", rec4[1]));
		return NULL;
	}
	return rec;
}

	static int
ndf_inq_data(nusdfile_t *df, nusdims_t *dim,
		int item, void *buf, N_UI4 bufnelems)
{
	N_UI4 *rec4;
	N_UI4 *buf4;
	N_UI8 ofs;
	N_UI4 nbytes;
	int r;
	if (buf == NULL) {
		return nus_err((NUSERR_IQ_DestNull, "can't write to NULL"));
	}
	if (item == N_DATA_EXIST) {
		struct ndf_t *ndf = &df->ndf;
		size_t recno = ndf_dim2irec(ndf, dim, err_mvze_read);
		if (~recno == 0) {
			return NUS_ERRNO;
		}
		if (bufnelems < 1) {
			r = nus_err((NUSERR_IQ_ShortBuf,
			"ndf_inq_data(N_DATA_EXIST) short buffer"));
			return r;
		}
		buf4 = buf;
		buf4[0] = 0;
		if (ndf->indx4) {
			if (~ndf->indx4[recno] == 0) {
				buf4[0] = 0;
			} else if (ndf->indx4[recno] != 0) {
				buf4[0] = 1;
			}
		} else {
			N_SI8 *indx8 = (N_SI8 *)(ndf->indx + 16);
			if (ui8_maxp(indx8[recno])) {
				buf4[0] = 0;
			} else if (!si8_iszero(indx8[recno])) {
				buf4[0] = 1;
			}
		}
		return 1;
	}
	if ((rec4 = ndf_load_data(&df->ndf, dim, &ofs)) == NULL) {
		nus_debug(("ndf_load_data => %d", NUS_ERRNO));
		return NUS_ERRNO;
	}
	switch (item) {
		case N_DATA_QUADRUPLET:
			if (bufnelems < 4) {
				r = nus_err((NUSERR_IQ_ShortBuf,
				"ndf_inq_data(N_GRID_SIZE) short buffer"));
			} else {
				buf4 = buf;
				buf4[0] = NTOH4(rec4[12]);
				buf4[1] = NTOH4(rec4[13]);
				buf4[2] = rec4[14];
				buf4[3] = rec4[15];
				r = 4;
			}
			break;
		case N_GRID_SIZE:
			if (bufnelems < 2) {
				r = nus_err((NUSERR_IQ_ShortBuf,
				"ndf_inq_data(N_GRID_SIZE) short buffer"));
			} else {
				buf4 = buf;
				buf4[0] = NTOH4(rec4[12]);
				buf4[1] = NTOH4(rec4[13]);
				r = 2;
			}
			break;
		case N_PC_PACKING:
			if (bufnelems < 1) {
				r = nus_err((NUSERR_IQ_ShortBuf,
				"ndf_inq_data(N_PC_PACKING) short buffer"));
			} else {
				buf4 = buf;
				buf4[0] = rec4[14];
				r = 1;
			}
			break;
		case N_MISSING_MODE:
			if (bufnelems < 1) {
				r = nus_err((NUSERR_IQ_ShortBuf,
				"ndf_inq_data(N_MISSING_MODE) short buffer"));
			} else {
				buf4 = buf;
				buf4[0] = rec4[15];
				r = 1;
			}
			break;
		case N_MISSING_VALUE:
			if (bufnelems < 1) {
				r = nus_err((NUSERR_IQ_ShortBuf,
				"ndf_inq_data(N_MISSING_VALUE) short buffer"));
			} else if (rec4[15] != SYM4_UDFV) {
				r = nus_err((NUSERR_IQ_BadParam,
				"ndf_inq_data: no missing value; missing=%Ps",
				rec4[15]));
			} else {
				goto CopyMissingValue;
			}
			break;
		case N_DATA_NBYTES:
			if (bufnelems < 1) {
				r = nus_err((NUSERR_IQ_ShortBuf,
				"ndf_inq_data(N_DATA_NBYTES) short buffer"));
			} else {
				buf4 = buf;
				buf4[0] = NTOH4(rec4[2]) - 44;
				r = 1;
			}
			break;
		case N_DATA_CONTENT:
			nbytes = NTOH4(rec4[2]) - 44;
			if (bufnelems < nbytes) {
				r = nus_err((NUSERR_IQ_ShortBuf,
					"ndf_inq_data: short buffer"
					" (%Pu given,"
					" %Pu required)", bufnelems, nbytes));
			} else {
				memcpy(buf, rec4 + 12, nbytes);
				r = nbytes;
			}
			break;
		case N_RECORD_TIME:
			if (bufnelems < 1) {
				r = nus_err((NUSERR_IQ_ShortBuf,
				"ndf_inq_data(N_RECORD_TIME) short buffer"));
			} else {
				buf4 = buf;
				buf4[0] = NTOH4(rec4[3]);
				r = 1;
			}
			break;
		default:
			r = nus_err((NUSERR_IQ_BadParam, "iqd"));
	}
	return r;

CopyMissingValue:
	switch (rec4[14]) {
		case SYM4_I1:
			memcpy(buf, rec4 + 16, 1);
			break;
		case SYM4_N1I2:
		case SYM4_I2:
		case SYM4_2PAC:
		case SYM4_2UPC:
			memcpy(buf, rec4 + 16, 2);
			break;
		case SYM4_R4:
		case SYM4_I4:
		case SYM4_4PAC:
			memcpy(buf, rec4 + 16, 4);
			break;
		default:
			nus_warn(("unknown packing %Ps; assuming 8-byte"));
		case SYM4_R8:
			memcpy(buf, rec4 + 16, 8);
			break;
	}
	return 1;
}

	INLINE size_t
CutWidth(sym4_t buftype)
{
	switch (buftype) {
		case SYM4_I1:
			return 1;
		case SYM4_I2:
			return 2;
		case SYM4_I4:
		case SYM4_R4:
			return 4;
		case SYM4_R8:
			return 8;
		default:
			nus_err((NUSERR_RD_NoCodec,
				"nusdas_cut: N_%Ps not supported", buftype));
			return 0;
	}
}

	INLINE void
ArrayCut(char *dest, char *src, size_t elemsize, N_UI4 src_nx,
		const struct cut_rectangle *cut)
{
	int iy, linewidth;
	char *srcline;
	linewidth = cut->cr_xnelems * elemsize;
	srcline = src + (src_nx * cut->cr_yofs + cut->cr_xofs) * elemsize;
	for (iy = cut->cr_yofs; iy < cut->cr_yofs + cut->cr_ynelems; iy++) {
		memcpy(dest, srcline, linewidth);
		srcline += elemsize * src_nx;
		dest += linewidth;
	}
}

	static int
ndf_read(nusdfile_t *df, nusdims_t *dim, struct ibuffer_t *buf)
{
	struct ndf_t *ndf = &(df->ndf);
	N_UI8	ofs;
	N_UI4		*rec4, nxd, nyd;
	void *rec;

	if ((rec4 = rec = ndf_load_data(ndf, dim, &ofs)) == NULL) {
		nus_debug(("ndf_load_data => %d", NUS_ERRNO));
		return NUS_ERRNO;
	}

	nxd = NTOH4(rec4[12]);
	nyd = NTOH4(rec4[13]);
	nus_debug(("m<%#Ps> v%P#T/%P#T <%#.18s>",
		NTOH4(rec4[4]), NTOH4(rec4[5]), NTOH4(rec4[6]), rec4 + 7));
	nus_debug((" %Pux%Pu pack=%Ps miss=%Ps recl=%Pd", nxd, nyd,
				rec4[14], rec4[15], NTOH4(rec4[2])));

	if (ndf_set_codec(ndf, rec4[14], rec4[15], buf->ib_fmt) != 0) {
		return NUS_ERR_CODE();
	}
	if (ndf->codec->decode == NULL) {
		return nus_err((NUSERR_WR_NoCodec, "missing codec <%Ps|%Ps|%Ps>",
			rec4[14], rec4[15], buf->ib_fmt));
	}

	if (cut_rectangle_disabled(&buf->ib_cut)) {
		return ndf->codec->decode(buf, (const N_UI1 *)rec + 64,
				nxd, nyd);
	} else {
		struct ibuffer_t fullbuf;
		size_t elemwidth;
		int r;
		if ((elemwidth = CutWidth(buf->ib_fmt)) == 0) {
			return NUS_ERR_CODE();
		}
		if ((fullbuf.ib_ptr = nus_malloc(nxd * nyd * elemwidth)) == NULL) {
			return nus_err((NUSERR_MemShort, "nus_malloc"));
		}
		fullbuf.ib_fmt = buf->ib_fmt;
		fullbuf.nelems = nxd * nyd;
		cut_rectangle_disable(&fullbuf.ib_cut);
		r = ndf->codec->decode(&fullbuf, (const N_UI1 *)rec + 64,
				nxd, nyd);
		if ((r < 0) || ((N_UI4)r < nxd * nyd)) {
			return r;
		}
		ArrayCut(buf->ib_ptr, fullbuf.ib_ptr, elemwidth, nxd,
				&buf->ib_cut);
		nus_free(fullbuf.ib_ptr);
		return cut_rectangle_size(&buf->ib_cut);
	}
}

/** ENDレコードに含まれるSUBC・INFOレコードの表を用いて
 *  subctab, infotab を初期化する。レコード内容 au_data は後で必要時に読む。
 */
	INLINE int
ndf_auxtab_load(struct ndf_t *ndf, N_UI4 *rec, N_UI4 endrecl)
{
	N_UI4 n_aux = (endrecl - 28) / 24;
	N_UI4 i;
	N_UI4 *ent;
	ndf->subctab = ndf_auxtab_ini(17);
	ndf->infotab = ndf_auxtab_ini(17);
	for (i = 0, ent = rec + 6; i < n_aux; i++, ent += 6) {
		struct ndf_auxtab_t *tab;
		struct ndf_aux_t *aux;
		int r;
		if ((aux = nus_malloc(sizeof *aux)) == NULL) {
			return NUSERR_MemShort;
		}
		aux->au_grp = ent[1];
		aux->au_pos = make_ui8(NTOH4(ent[2]), NTOH4(ent[3]));
		aux->au_recl = NTOH4(ent[4]);
		aux->au_data = NULL;
		tab = (ent[0] == SYM4_SUBC) ? ndf->subctab : ndf->infotab;
		r = ndf_auxtab_put(tab, &aux->au_grp, aux);
		if (r) {
			return r;
		}
		nus_debug(("%Ps%Ps regd", ent[0], ent[1]));
	}
	return 0;
}

	static int
ndf_free_aux(sym4_t grp UNUSED, struct ndf_aux_t *val, void *arg UNUSED)
{
	if(val){
		if(val->au_data) nus_free(val->au_data);
		nus_free(val);
	}
	return 0;
}

	static int
ndf_loadfile_wpos(struct ndf_t *ndf, N_UI4 *nusd)
{
	N_UI8 eof;
	/** NuSDaS データファイルは少なくとも
	 * (NUSD 30語 + CNTL 45語 + INDX 5語 + END 7語) = 348 バイト
	 * 以上ある
	 */
	N_UI4 *rec;
	if (!(ndf->comm.flags & IO_WRITABLE)) {
		goto Dont_Need_WPos;
	}
	/** NUSD 記録のファイル長から 4 バイト退いたところを読んで
	 * END 記録長を得る */
	if (ndf_is_largefile(ndf)) {
		eof = make_ui8(NTOH4(nusd[22]), NTOH4(nusd[23]));
	} else {
		eof = make_ui8(0, NTOH4(nusd[25]));
	}
	if ((rec = nusnsf_read_before(&ndf->seqf, eof)) == NULL) {
		goto Tolerable_Error;
	}
	ndf->wpos = ndf->seqf.sf_pos;

	if (rec[1] != SYM4_END) {
		nus_warn(("missing END record %Qd:%Pd", ndf->wpos,
			ndf->seqf.sf_recl));
Tolerable_Error:
		/* 読む側であるから極力寛容に */
		/** @todo このフラグをちゃんとみる */
		nus_warn(("file treated as readonly"));
		ndf->comm.flags &= ~IO_WRITABLE;
Dont_Need_WPos:
		ndf->wpos = long_to_si8(0);
		if (ndf->subctab) {
			ndf_auxtab_each(ndf->subctab, ndf_free_aux, NULL);
			ndf_auxtab_delete(ndf->subctab);
		}
		if (ndf->infotab) {
			ndf_auxtab_each(ndf->infotab, ndf_free_aux, NULL);
			ndf_auxtab_delete(ndf->infotab);
		}
		ndf->subctab = ndf->infotab = NULL;
		return 0;
	}
	/* END 記録を読んだので補助記録表を作っておく */
	if (! ndf_is_largefile(ndf)) {
		/* ファイル形式 12 以前では全記録探査しないと
		 * INFO 情報を構築できないので
		 * とりあえず SUBC/INFO 表を割り付けないでおく
		 * すでに確保している場合は削除する
		 */
		if (ndf->subctab) {
			ndf_auxtab_each(ndf->subctab, ndf_free_aux, NULL);
			ndf_auxtab_delete(ndf->subctab);
		}
		if (ndf->infotab) {
			ndf_auxtab_each(ndf->infotab, ndf_free_aux, NULL);
			ndf_auxtab_delete(ndf->infotab);
		}
		ndf->subctab = ndf->infotab = NULL;
	} else {
		int r;
		r = ndf_auxtab_load(ndf, rec, ndf->seqf.sf_recl);
		if (r) return r;
	}
	return 0;
}

/** データファイルの CNTL 記録等を読み ndf を初期化
 */
static int
ndfopen_loadfile(struct ndf_t *ndf)
{
	net4_t		*nusd, *cntl, *indx;
	N_UI4		recnel, nusdsize, cntlsize;
	unsigned	i;
	char		*cntl_list;

	/* NUSD 記録の取得 */
	if ((nusd = nusnsf_read_rec(&ndf->seqf)) == NULL) {
		return nus_err((NUSERR_NUSDReadFail, "can't read NUSD"));
	}
	nusdsize = ndf->seqf.sf_recl;

	if (nusd[1] != SYM4_NUSD) {
		nus_err((NUSERR_NUSDReadFail, "bad magic number %Ps for NUSD",
					nusd[1]));
		return -1;
	}

	if (df_is_writable(ndf) && (nusdsize > NUSD_SIZE)) {
		ndf->forcedrlen = nusdsize;
		nus_debug(("forcedrlen = %ld", ndf->forcedrlen));
	}
	switch (NTOH4(nusd[24])) {
		case 1:
			ndf->filever = 10;
			break;
		case 10:
		case 11:
		case 13:
		case 14:
			ndf->filever = NTOH4(nusd[24]);
			break;
		default:
			nus_err((NUSERR_BadVersion, "bad data_file version %x",
						NTOH4(nusd[24])));
			return -1;
	}
	ndf->reccount = NTOH4(nusd[26]);
	ndf->infocount = NTOH4(nusd[27]);
	ndf->subccount = NTOH4(nusd[28]);

	/* CNTL 記録の取得 */
	if ((cntl = nusnsf_read_rec(&ndf->seqf)) == NULL) {
		return nus_err((NUSERR_CNTLReadFail, "can't read CNTL"));
	}
	cntlsize = ndf->seqf.sf_recl;

	if (cntl[1] != SYM4_CNTL) {
		nus_err((NUSERR_CNTLReadFail, "wrong magic CNTL"));
		return -1;
	}
	ndf->cntltime = cntl[3];

	ndf->btime = array4_ini(ndf->nb);
	array4_set(ndf->btime, 0, NTOH4(cntl[11]));
	/* ndf の次元長を上書きすることに注意 */
	ndf->member = array4_ini(ndf->nm = NTOH4(cntl[13]));
	ndf->vtime = array8_ini(ndf->nv = NTOH4(cntl[14]));
	ndf->plane = array8_ini(ndf->nz = NTOH4(cntl[15]));
	ndf->element = array8_ini(ndf->ne = NTOH4(cntl[16]));

	ndf->nx = NTOH4(cntl[18]);
	ndf->ny = NTOH4(cntl[19]);

	cntl_list = (char *)(cntl + 43);
	for (i = 0; i < ndf->nm; i++) {
		array4_set(ndf->member, i, MEM2SYM4(cntl_list + i * 4));
	}
	cntl_list += 4 * ndf->nm;

	for (i = 0; i < ndf->nv; i++) {
		N_UI4	vt1, vt2;
		N_UI8	vt12;
		vt1 = NTOH4(*(net4_t *)(cntl_list + i * 4));
		vt2 = NTOH4(*(net4_t *)(cntl_list + (ndf->nv + i) * 4));
		vt12 = make_ui8(vt1, vt2);
		array8_set(ndf->vtime, i, vt12);
	}
	cntl_list += 8 * ndf->nv;

	for (i = 0; i < ndf->nz; i++) {
		array8_set(ndf->plane, i, mem6sym8(cntl_list + i * 6));
	}
	cntl_list += 12 * ndf->nz;

	for (i = 0; i < ndf->ne; i++) {
		array8_set(ndf->element, i, mem6sym8(cntl_list + i * 6));
	}
	/* もしこのあと何かが続くなら  cntl_list += 6 * ndf->ne; */

	/* 解読が終了した NUSD/CNTL の後始末
	 * 書き込み可なら ndf->nusd, ndf->cntl を保存しておく。
	 */
	if (df_is_writable(ndf)) {
		ndf->nusd = (N_UI1 *)nusd;
		ndf->cntl = (N_UI1 *)cntl;
		ndf->nusdsize = nusdsize;
		ndf->cntlsize = cntlsize;
	} else {
		nus_free(nusd);
		nus_free(cntl);
	}

	/* INDX の読み取り */
	if ((ndf->indx = nusnsf_read_rec(&ndf->seqf)) == NULL) {
		return nus_err((NUSERR_INDXReadFail, 
				"can't read index record"));
	}
	ndf->indxsize = ndf->seqf.sf_recl;

	recnel = ndf->nm * ndf->nv * ndf->nz * ndf->ne;
	if (ndf->indxsize < 20 + (ndf_is_largefile(ndf) ? 16 : 4) * recnel) {
		return nus_err((NUSERR_INDXReadFail, 
				"too small index record"));
	}

	indx = (N_UI4 *)(ndf->indx + 16);
	if (indx[-3] != (ndf_is_largefile(ndf) ? SYM4_INDY : SYM4_INDX)) {
		nus_err((NUSERR_INDXReadFail, "bad record <%Ps> expecting %Ps",
			indx[-3],
			(ndf_is_largefile(ndf) ? SYM4_INDY : SYM4_INDX)));
		return -1;
	}
	if (ndf_is_largefile(ndf) == 0) {
		ndf->indx4 = indx;
		ndf->indy_recl = ndf->indy_nelems = NULL;
	} else {
		ndf->indy_recl = indx + 2 * recnel;
		ndf->indy_nelems = indx + 3 * recnel;
		ndf->indx4 = NULL;
	}

	/* 次に記録を書き込める位置を決める */
	ndf_loadfile_wpos(ndf, nusd);

	return 0;
}

	static int
ndf_reopen(union nusdfile_t *df, int open_flags)
{
	union nusio_t *io;
	int switch_to_writable = 0;
	/* @note 重要: GC による破棄を防ぐため、あらゆる malloc より
	 * 先に is_open フラグを立てる必要がある。
	 */
	df->comm.is_open = 1;
	io = (df_param(df, io_open))(df->comm.filename, open_flags);
	if (io == NULL) {
		return -1;
	}
	nusnsf_ini(&df->ndf.seqf, io, df->ndf.filever < 11 ? 0 : 8, 0);
	if (df->comm.flags == open_flags) {
		return 0;
	}
	nus_debug(("flags 0%o -> 0%o", df->comm.flags, open_flags));
	if (!(df->comm.flags & IO_WRITABLE) &&
			(open_flags & IO_WRITABLE)) {
		switch_to_writable = 1;
		if (nusndf_load_cntl(df) == NULL)
			return NUS_ERRNO;
	}
	df->comm.flags = open_flags;
	if (switch_to_writable) {
		ndf_loadfile_wpos(&df->ndf, (N_UI4 *)df->ndf.nusd);
	}
	return 0;
}

/** @brief NUSD, CNTL 記録を後からロードする.
 */
	N_UI4 *
nusndf_load_cntl(union nusdfile_t *df)
{
	struct ndf_t *ndf = &df->ndf;

	if (ndf->nusd && ndf->cntl) {
		return (N_UI4 *)ndf->cntl;
	}

	if (nusnsf_rewind(&ndf->seqf) != 0) {
		return NULL;
	}

	/* NUSD 記録の取得 */
	if ((ndf->nusd = nusnsf_read_rec(&ndf->seqf)) == NULL) {
		nus_err((NUS_ERRNO, "can't read NUSD"));
		return NULL;
	}
	ndf->nusdsize = ndf->seqf.sf_recl;

	/* CNTL 記録の取得 */
	if ((ndf->cntl = nusnsf_read_rec(&ndf->seqf)) == NULL) {
		nus_err((NUS_ERRNO, "can't read CNTL"));
		return NULL;
	}
	ndf->cntlsize = ndf->seqf.sf_recl;

	return (N_UI4 *)ndf->cntl;
}

	static int 
ndf_write_grid(union nusdfile_t *df,
		const char proj[4], const N_SI4 size[2],
		const float projparam[14], const char value[4])
{
	N_UI4 *cntl;
	const N_UI4 *xparam = (const N_UI4 *)projparam;
	int i;
	if (df_is_writable(df) == 0) {
		return nus_err((NUSERR_DfileUnwritable, "read only dfile"));
	}
	cntl = nusndf_load_cntl(df);
	if (cntl == NULL) {
		return NUS_ERR_CODE();
	}
	memcpy4((char *)(cntl + 17), proj);
	cntl[18] = HTON4(size[0]);
	cntl[19] = HTON4(size[1]);
	for (i = 0; i < 14; i++) {
		cntl[20 + i] = HTON4(xparam[i]);
	}
	memcpy4((char *)(cntl + 34), value);
	return ndf_grid_check(cntl);
}

static struct df_functab ndf_methods = {
	ndf_close,
	ndf_delete,
	ndf_read,
	ndf_write,
	ndf_reopen,
	nusndf_read_aux,
	nusndf_write_aux,
	nusndf_inq_cntl,
	ndf_write_grid,
	ndf_inq_data,
	ndf_flush,
	nusndf_inq_aux
};

#define MAPITEM(def, im, iv, iz, ie) \
	((def)->elementmap[ \
	(((im * (def)->n_vt) \
	 + iv) * (def)->n_lv \
	 + iz) * (def)->n_el \
	 + ie ])

	INLINE int
ndf_creat_index(struct ndf_t *ndf, nusdef_t *def)
{
	N_UI4 nelems = ndf->nm * ndf->nv * ndf->nz * ndf->ne;
	net4_t *indx4;
	N_UI4 indxvsize;
	unsigned int i, ie, iz, iv, im;

	if (ndf_is_largefile(ndf)) {
		ndf->indxsize = nusndf_recl(ndf, indxvsize = 20 + 16 * nelems);
	} else {
		ndf->indxsize = nusndf_recl(ndf, indxvsize = 20 + 4 * nelems);
	}
	if (ndf->indxsize == 0) {
		return NUS_ERRNO;
	}
	ndf->indx = nus_malloc(ndf->indxsize);
	if (ndf->indx == NULL) {
		nus_free(ndf->nusd);
		nus_free(ndf->cntl);
		return nus_err((NUSERR_MemShort, "memory short"));
	}

	indx4 = (N_UI4 *)ndf->indx + 4;
	/** バイトオーダー注意 */
	indx4[-4] = HTON4(ndf->indxsize - ndf->seqf.sf_reclplus);
	indx4[-3] = ndf_is_largefile(ndf) ? SYM4_INDY : SYM4_INDX;
	indx4[-2] = HTON4(indxvsize - 8);
	indx4[-1] = ~(N_UI4)0; /* time_t */
	nus_debug(("version=%Pu index=%Ps size=%zu", ndf->filever,
				indx4[-3], ndf->indxsize));
	indx4[(ndf->indxsize >> 2) - 5] = indx4[-4];
	if (ndf_is_largefile(ndf)) {
		N_SI8 *headtab = (N_SI8 *)indx4;
		ndf->indy_recl = indx4 + 2 * nelems;
		ndf->indy_nelems = indx4 + 3 * nelems;
		i = 0;
		for (im = 0; im < ndf->nm; im++) {
		for (iv = 0; iv < ndf->nv; iv++) {
		for (iz = 0; iz < ndf->nz; iz++) {
		for (ie = 0; ie < ndf->ne; ie++) {
			headtab[i++] = (MAPITEM(def, (im + ndf->om), (iv + ndf->ov), iz, ie)
				? long_to_si8(-1)
				: long_to_si8(0));
		}
		}
		}
		}
		memset(ndf->indy_recl, 0, nelems * 4);
		memset(ndf->indy_nelems, 0, nelems * 4);
		ndf->indx4 = NULL;
	} else {
		i = 0;
		for (im = 0; im < ndf->nm; im++) {
		for (iv = 0; iv < ndf->nv; iv++) {
		for (iz = 0; iz < ndf->nz; iz++) {
		for (ie = 0; ie < ndf->ne; ie++) {
			indx4[i++] = MAPITEM(def, (im + ndf->om), (iv + ndf->ov), iz, ie);
		}
		}
		}
		}
		ndf->indx4 = indx4;
		ndf->indy_recl = ndf->indy_nelems = NULL;
	}
	return 0;
}

/** @brief NDF に NUSD, CNTL, INDX 記録のメモリイメージを構築する
 * NDF を書き込み用にオープンして新規作成となった場合に呼ばれる。
 *
 */
INLINE int
ndf_creat(struct ndf_t *ndf, struct dds_t *dds, const nusdims_t *dim)
{
	net4_t *nusd, *cntl, *cntl4;
	char *cntl_list;
	N_UI4 cntlsize_min;
	unsigned i;
	struct nusdef_subcinfo_t *psi;
	nusdef_t *def = &dds->def;
	int r;

	ndf->btime = array4_ini(ndf->nb);
	array4_set(ndf->btime, 0, dim->basetime);
	ndf->member = array4_ini(ndf->nm);
	ndf->vtime = array8_ini(ndf->nv);
	ndf->plane = array8_ini(ndf->nz);
	ndf->element = array8_ini(ndf->ne);
	ndf->filever = def->version;

	ndf->nusdsize = nusndf_new_record(&(ndf->nusd), ndf,
		NUSD_SIZE, SYM4_NUSD, NULL);
	if (ndf->nusdsize == 0) {
		return NUS_ERRNO;
	}
	nusd = (net4_t *)ndf->nusd;
	/* ファイルバージョン 13 以降では 72..79 バイト目はファイル長で
	 * 上書きされる */
	memcpy(nusd + 4, def->creator, 80);
	nusd[24] = HTON4(ndf->filever);

	cntlsize_min = NDF_CNTL_SIZE(ndf);
	ndf->cntlsize = nusndf_new_record(&(ndf->cntl), ndf,
		cntlsize_min, SYM4_CNTL, NULL);
	if (ndf->cntlsize == 0) {
		return NUS_ERRNO;
	}
	cntl = (net4_t *)ndf->cntl;
	*(sym8_t *)&(cntl[4]) = def->nustype.type1;
	cntl[6] = def->nustype.type2;
	cntl[7] = def->nustype.type3;
	time_to_chars((char *)&(cntl[8]), dim->basetime);
	cntl[11] = HTON4(dim->basetime);
	cntl[12] = def->ftunits;
	cntl[13] = HTON4(ndf->nm);
	cntl[14] = HTON4(ndf->nv);
	cntl[15] = HTON4(ndf->nz);
	cntl[16] = HTON4(ndf->ne);
	cntl[17] = nusdef_projcode(def);
	cntl[18] = HTON4(ndf->nx);
	cntl[19] = HTON4(ndf->ny);
	for (i = 0; i < 7 * 2; i++) {
		cntl[20 + i] = HTON4((((N_UI4 *)(def->projparam))[i]));
	}
	cntl[34] = def->value;
	for (i = 0; i < 8; i++) {
		cntl[i + 35] = 0;
	}
	cntl4 = cntl + 43;
	if (def->mb_out > 0) {
		ndf->om = -1;
		for (i = 0; i < def->n_mb; i++) {
			if (def->mb[i] == dim->member) {
				ndf->om = i;
				break;
			}
		}
		if (ndf->om < 0) {
			return nus_err((NUSERR_WR_BadMember, "member \"%Ps\" not found in %#ys",
					dim->member, &ndf->comm.nustype));
		}
		cntl4[0] = dim->member;
		array4_set(ndf->member, 0, dim->member);
	} else {
		for (i = 0; i < ndf->nm; i++) {
			cntl4[i] = def->mb[i];
			array4_set(ndf->member, i, def->mb[i]);
		}
	}
	cntl4 += ndf->nm;
	if (def->vt_out > 0) {
		N_UI4   vt1, vt2;
		N_UI8   vt;
		ndf->ov = -1;
		for (i = 0; i < def->n_vt; i++) {
			vt1 = time_add(dim->basetime, def->ft1[i], def->ftunits);
			vt2 = (def->ft2
				? time_add(dim->basetime, def->ft2[i], def->ftunits)
				: 1);
			if ((vt1 == dim->validtime1) && (vt2 == dim->validtime2)) {
				ndf->ov = i;
				break;
			}
		}
		if (ndf->ov < 0) {
			return nus_err((NUSERR_BadVtime, "validtime \"%T/%T\" not found in %#ys",
					dim->validtime1, dim->validtime2, &ndf->comm.nustype));
		}
		cntl4[0] = HTON4(dim->validtime1);
		cntl4[0 + ndf->nv] = HTON4(dim->validtime2);
		vt = make_ui8(dim->validtime1, dim->validtime2);
		array8_set(ndf->vtime, 0, vt);
	} else {
		for (i = 0; i < ndf->nv; i++) {
			N_UI4	vt1, vt2;
			vt1 = time_add(dim->basetime, def->ft1[i], def->ftunits);
			vt2 = (def->ft2
				? time_add(dim->basetime, def->ft2[i], def->ftunits)
				: 1);
			cntl4[i] = HTON4(vt1);
			cntl4[i + ndf->nv] = HTON4(vt2);
			array8_set(ndf->vtime, i, make_ui8(vt1, vt2));
		}
	}
	cntl4 += (ndf->nv * 2);
	cntl_list = (char *)cntl4;
	for (i = 0; i < ndf->nz; i++) {
		memcpy6(cntl_list + i * 6, (char *)(def->lv1 + i));
		array8_set(ndf->plane, i, def->lv1[i]);
	}
	cntl_list += ndf->nz * 6;
	for (i = 0; i < ndf->nz; i++) {
		memcpy6(cntl_list + i * 6,
			(char *)((def->lv2 ? def->lv2 : def->lv1) + i));
	}
	cntl_list += ndf->nz * 6;
	for (i = 0; i < ndf->ne; i++) {
		memcpy6(cntl_list + i * 6, (char *)(def->el + i));
		array8_set(ndf->element, i, def->el[i]);
	}
#if NEED_ALIGN & 4
	memcpy4((char *)cntl + ndf->cntlsize - 4, (const char *)cntl);
#else
	cntl4 = (net4_t *)((unsigned char *)cntl + ndf->cntlsize - 4);
	*cntl4 = cntl[0];
#endif
	if (cntl[17] != SYM4_XX) {
		if (ndf_grid_check(cntl)) {
			return NUS_ERRNO;
		}
	}

	if (ndf_creat_index(ndf, def)) {
		return NUS_ERRNO;
	}

	ndf->wpos = long_to_si8(ndf->nusdsize + ndf->cntlsize + ndf->indxsize);

	ndf->subctab = ndf_auxtab_ini(17);
	ndf->infotab = ndf_auxtab_ini(17);
	ndf->infocount = ndf->subccount = 0;

	/* データがゼロの場合 NUSD, CNTL, INDX, END の 4 記録ある */
	ndf->reccount = 4;
	ndf->cntltime = 0xFFFFFFFFul;

	for (psi = def->subctab; psi; psi = psi->next) {
		r = nusndf_creat_subc(ndf, dds, psi);
		if (r) {
			goto Hell;
		}
		ndf->reccount++;
	}
	for (psi = def->infotab; psi; psi = psi->next) {
		r = nusndf_creat_info(ndf, psi);
		if (r) {
			goto Hell;
		}
		ndf->reccount++;
	}

	return 0;

Hell:
	nus_free(ndf->nusd);
	nus_free(ndf->cntl);
	nus_free(ndf->indx);
	ndf_auxtab_delete(ndf->subctab);
	ndf_auxtab_delete(ndf->infotab);
	return r;
}

/** @brief データファイルの初期化
 *
 * 新設/既存オープンを問わず必ず行う処理
 */
static void
ndfopen_initfields(struct ndf_t *ndf, const char *filename, int flags,
		nusdef_t *def)
{
	ndf->comm.is_open = 1;
	ndf->comm.filename = string_dup(filename);
	ndf->comm.methods = ndf_methods;
	ndf->comm.nustype = def->nustype;
	ndf->comm.flags = flags;
	ndf->codec = NULL;
	ndf->infotab = NULL;
	ndf->subctab = NULL;
	ndf->nusd = ndf->cntl = NULL;
	ndf->wpos = long_to_si8(0);
	ndf->totalsize = long_to_si8(0);
	/* この2欄は CNTL 記録に書いていないので
	 * 読み込み時であっても定義ファイルから補う */
	ndf->packing = def->packing;
	ndf->missing = def->missing;
	/* 定義ファイルからデータの大きさに関する諸元をコピー
	 * この値は ndfopen_loadfile() が上書きする */
	ndf->nb = 1;
	ndf->nm = def->mb_out > 0 ? def->mb_out : def->n_mb;
	ndf->nv = def->vt_out > 0 ? def->vt_out : def->n_vt;
	ndf->om = 0;
	ndf->ov = 0;
	ndf->nz = def->n_lv;
	ndf->ne = def->n_el;
	ndf->ny = def->ny;
	ndf->nx = def->nx;
	ndf->forcedrlen = def->forcedrlen;
	if (ndf->forcedrlen) {
		nus_debug(("forcedrlen = %ld", ndf->forcedrlen));
	}
	/* 引数ではなくシステム設定からのコピー */
	if (df_param(ndf, pc_wbuffer)) {
		ndf->wbuf = nus_malloc(sizeof(struct ndf_wbuf_t));
		if (ndf->wbuf) {
			ndf->wbuf->siz = df_param(ndf, pc_wbuffer) * 1024;
			ndf->wbuf->pos = 0;
			ndf->wbuf->ptr = NULL;
			ndf->wbuf->ofs = long_to_si8(0);
		}
		nus_debug(("write buffer %dKB", df_param(ndf, pc_wbuffer)));
	} else {
		ndf->wbuf = NULL;
	}
}

nusdfile_t *
ndf_open(union nusio_t *io, const char *filename,
		int flags, struct dds_t *dds, const nusdims_t *dim)
{
	struct ndf_t *ndf;
	int r;
	N_SI4 btime;   
	nus_debug(("ndf_open(%s)", filename));
	ndf = nus_malloc(sizeof(struct ndf_t));
	if (ndf == NULL) {
		SETERR(NUSERR_MemShort);
		return NULL;
	}
	ndf->comm.ds_param = &dds->comm.param;
	ndfopen_initfields(ndf, filename, flags, &dds->def);
	if (flags & IO_CREATED) {
		nusnsf_ini(&ndf->seqf, io, dds->def.version < 11 ? 0 : 8,
				DynamicParam(&dds->comm.param, pc_rbuffer));
		r = ndf_creat(ndf, dds, dim);
	} else {
		nusnsf_ini(&ndf->seqf, io, flags & IO_OLDSEQF ? 0 : 8,
				DynamicParam(&dds->comm.param, pc_rbuffer));
		r = ndfopen_loadfile(ndf);
		btime = array4_get_value(ndf->btime, 0);
		if(dim->basetime != btime && dim->basetime != -1){
			nus_warn(("basetime of CNTL record is different."));
		}
	}
	if (r != 0) {
		io_close(io, 0, NULL);
		nus_free(ndf);
		return NULL;
	}
	return (nusdfile_t *)ndf;
}
