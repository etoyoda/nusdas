/** @file
 * @brief NDF 関係ルーチンのうち、補助記録 (SUBC/INFO) 関係
 */
#include "config.h"
#include "nusdas.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
# define NEED_MAKE_UI8
# define NEED_LONG_TO_SI8
# define NEED_ULONG_TO_SI8
# define NEED_SI8_CMP
# define NEED_SI8_ADD
#include "sys_int.h"
#include "internal_types.h"
#include "sys_kwd.h"
# define NEED_MEMCPY4
#include "sys_string.h"
# define NEED_PEEK_SYM4_T
# define NEED_PEEK_N_UI4
#include "sys_endian.h"
#include "sys_err.h"
#include "sys_file.h"
#include "dfile.h"
#include "dset.h"
#include "glb.h"
#include "sys_mem.h"
#include "io_comm.h"
#include "ndf.h"

/** @brief レコードの整形・メモリ確保
 * 必要サイズ minrecl を指定すると、8の倍数に切上げを要すればやった
 * 結果のレコード長を返す。
 */
	size_t
nusndf_new_record(N_UI1 **datap, struct ndf_t *ndf,
	size_t minrecl, sym4_t recname, const char *infofile)
{
	time_t now;
	N_UI1 *data;
	N_UI4 frecl, hrecl, vrecl;
	/* --- adjust size --- */
	if ((minrecl == 24) && infofile) {
		minrecl = 24 + file_size(infofile);
		nus_warn(("%Ps size = %zu (%s)", recname, minrecl, infofile));
	}
	frecl = nusndf_recl(ndf, minrecl);
	if (frecl == 0) {
		return 0;
	} else if (frecl != minrecl) {
		nus_debug(("%Ps size adjusted %zu => %zu",
			recname, minrecl, frecl));
	}
	/* --- get buffer --- */
	data = *datap = nus_malloc(frecl);
	if (data == NULL) {
		nus_err((NUSERR_MemShort, "memory short"));
		return 0;
	}
	/* --- make format --- */
	hrecl = frecl - ndf->seqf.sf_reclplus;
	vrecl = minrecl - 8;
	if (nus_dbg_enabled && GlobalConfig(no_timestamp)) {
		now = 0;
	} else {
		time(&now);
	}
	/* head */
	POKE_N_UI4(data, hrecl);
	memcpy4((char *)data + 4, (char *)&recname);
	POKE_N_UI4(data + 8, vrecl);
	POKE_N_UI4(data + 12, now);
	/* body of INFO record */
	if (infofile) {
		size_t r;
		r = file_read_size(data + 20, frecl - 24, infofile);
		if (r == 0) {
			nus_err((NUSERR_NoInfoFile, "missing %s for INFO",
			infofile));
			return 0;
		}
	}
	/* tail */
	POKE_N_UI4(data + frecl - 4, hrecl);
	return frecl;
}

/** @brief レコード構造のチェック
 */
	int
nusndf_check_format(N_UI1 *rec, size_t xrecl, N_SI4 reclplus)
{
	N_UI4 recl, frecl, vrecl, trecl;
	recl = PEEK_N_UI4(rec);
	frecl = recl + reclplus;
	vrecl = PEEK_N_UI4(rec + 8);
	trecl = PEEK_N_UI4(rec + frecl - 4);
	nus_debug(("recl head:%Pu tail:%Pu valid:%Pu full:%Pu ext:%zu %s",
		recl, trecl, vrecl, frecl, xrecl,
		reclplus ? "new" : "old"));
	if ((vrecl + 8) > xrecl) {
		return nus_err((NUSERR_Internal,
			"recl mismatch valid:%Pu + 8 > ext:%Pu",
			vrecl, xrecl));
	}
	if (frecl != xrecl) {
		nus_warn(("recl bug-adjusted %Pu -> %Pu", frecl, xrecl));
		recl = xrecl - reclplus;
		POKE_N_UI4(rec, recl);
	}
	if (recl != trecl) {
		nus_warn(("recl tail recovered %Pu -> %Pu", trecl, recl));
		POKE_N_UI4(rec + xrecl - 4, recl);
	}
	return 0;
}

/** ndf_write_aux および ndf_read_aux の下請けで、
 * 必要時に ndf->subctab, ndf->infotab を構築
 */

	INLINE int
ndf_auxtab_scan(struct ndf_t *ndf)
{
	ndf->subctab = ndf_auxtab_ini(17);
	ndf->infotab = ndf_auxtab_ini(17);
	nusnsf_rewind(&ndf->seqf);
	while (nusnsf_read_head(&ndf->seqf) == 0) {
		struct ndf_aux_t *aux;
		sym4_t titl = PEEK_sym4_t(ndf->seqf.sf_rec + 1);
		struct ndf_auxtab_t *tab;
		int r;
		if (titl == SYM4_END) {
			break;
		}
		/** INFO 記録がないなら、DATA に突き当たり次第終了
		 * (この関数は既存ファイルに対してのみ呼ばれ得る)
		 */
		if (ndf->infocount == 0 && titl == SYM4_DATA) {
			break;
		}
		if (titl != SYM4_SUBC && titl != SYM4_INFO) {
			continue;
		}
		aux = nus_malloc(sizeof *aux);
		aux->au_grp = PEEK_sym4_t(ndf->seqf.sf_rec + 4);
		aux->au_pos = ndf->seqf.sf_pos;
		aux->au_recl = ndf->seqf.sf_recl;
		nus_debug(("aux %Ps %Ps", titl, aux->au_grp));
		if (nusnsf_read_full(&ndf->seqf) == 0) {
			/* この sf_recl は実長だから補正しない */
			aux->au_data = nus_malloc(ndf->seqf.sf_recl);
			if (aux->au_data) {
				memcpy(aux->au_data, ndf->seqf.sf_rec,
						ndf->seqf.sf_recl);
  if (!!(r = nusndf_check_format(
    aux->au_data, aux->au_recl, ndf->seqf.sf_reclplus))) {
    return r;
  }
			}
		}
		tab = (titl == SYM4_SUBC ? ndf->subctab : ndf->infotab);
		r = ndf_auxtab_put(tab, &aux->au_grp, aux);
		if (r) {
			return r;
		}
	}
	return 0;
}

	INLINE struct ndf_aux_t *
ndf_aux_getcached(struct ndf_t *ndf, sym4_t titl, sym4_t grp)
{
	struct ndf_auxtab_t *tab;
	struct ndf_aux_t *cached;
	switch (titl) {
		case SYM4_SUBC:
			tab = ndf->subctab;
			break;
		case SYM4_INFO:
			tab = ndf->infotab;
			break;
		default:
			nus_err((NUSERR_Internal, "record %Ps unacceptable",
						titl));
			return NULL;
	}
	if ((cached = ndf_auxtab_get(tab, &grp)) == NULL) {
		nus_err((NUSERR_SC_PeekFailed,
					"%Ps %Ps not allowed in deffile %#ys",
					titl, grp, &ndf->comm.nustype));
		return NULL;
	}
	if (cached->au_data == NULL) {
		N_UI8 recl8, r8;
		N_UI4 recl;
		recl = nusndf_recl(ndf, cached->au_recl);
		if (recl == 0) {
			nus_err((NUSERR_Internal,
			"%Ps %Ps larger than forcedrlen", titl, grp));
			return NULL;
		}
		if (recl != cached->au_recl) {
			nus_warn(("%Ps %Ps resized %Pu -> %Pu (END rec bug)",
				titl, grp, cached->au_recl, recl));
			cached->au_recl = recl;
		}
		cached->au_data = nus_malloc(cached->au_recl);
		if (cached->au_data == NULL) {
			nus_err((NUSERR_MemShort, "memshort"));
			return NULL;
		}
		nus_debug(("loading cached %Ps %Ps size=%Pd", titl, grp,
			cached->au_recl));
		recl8 = make_ui8(0, cached->au_recl);
		r8 = nusnsf_read_to(&ndf->seqf,
				cached->au_pos, recl8, cached->au_data);
		if (!ui8_eq(r8, recl8)) {
			nus_free(cached);
			nus_err((NUSERR_Internal, "nusnsf_read_to %Ps %Ps",
						titl, grp));
			return NULL;
		}
		if (0 != nusndf_check_format(
		cached->au_data, cached->au_recl, ndf->seqf.sf_reclplus)) {
			return NULL;
		}
	}
	return cached;
}

	int
nusndf_read_aux(union nusdfile_t *df, sym4_t ttl, sym4_t grp,
		int (*callback)(const void *rec, N_UI4 siz,
			void *arg, union nusdset_t *ds, N_SI4 ofs_flg),
		void *arg, union nusdset_t *ds)
{
	struct ndf_t *ndf = &df->ndf;
	struct ndf_aux_t *cached;
	N_UI4 vrecl;
	int	r;

	if (ndf->subctab == NULL) {
		r = ndf_auxtab_scan(ndf);
		if (r) return r;
	}
	cached = ndf_aux_getcached(ndf, ttl, grp);
	if (cached == NULL) {
		return SETERR(NUSERR_SC_PeekFailed);
	}
	vrecl = PEEK_N_UI4(cached->au_data + 8);
	r = callback(cached->au_data + 20, vrecl - 16, arg, ds, 1);
	nus_debug(("hit cache => %d", r));
	return r;
}

	int 
nusndf_write_aux(union nusdfile_t *df, sym4_t ttl, sym4_t grp, size_t nbytes,
		int (*encoder)(void *rec, N_UI4 siz,
			void *arg, union nusdset_t *ds),
		void *arg, union nusdset_t *ds)
{
	struct ndf_t *ndf = &df->ndf;
	struct ndf_aux_t *ai;
	int r;
	/** write 直後に呼ばれた場合 flush しておく
	 * @todo より効率的な方法を考える
	 * */
	if (nusndf_flushwrite(ndf) != 0) {
		return nus_err((NUSERR_IO,
			"nusndf_flushwrite failed before reading"));
	}
	if (ndf->subctab == NULL) {
		r = ndf_auxtab_scan(ndf);
		if (r) return r;
	}
	nuserr_mark(MARK_FOR_INFO);
	ai = ndf_aux_getcached(ndf, ttl, grp);
	if (ttl == SYM4_SUBC) {
		if (ai == NULL) {
			return NUS_ERRNO;
		}
		if (~nbytes == 0) {
			nbytes = ai->au_recl - 24;
		}
		if ((ndf->forcedrlen == 0) &&
		    ((ai->au_recl < nbytes + 24) ||
		     (ai->au_recl > nbytes + 24 + 8))) {
			return nus_err((NUSERR_SC_SizeMismatch,
				"size mismatch (arg=%zu def=%zu)",
				nbytes + 24, ai->au_recl));
		}
	} else {
		/* ttl == SYM4_INFO */
		if (ai == NULL) {
			size_t frecl;
			if (NUS_ERRNO != NUSERR_SC_PeekFailed) {
				return NUS_ERRNO;
			}
			nuserr_cancel(MARK_FOR_INFO);
			ai = nus_malloc(sizeof *ai);
			ai->au_grp = grp;
			ai->au_pos = long_to_si8(0);
			frecl = nusndf_new_record(&(ai->au_data), ndf,
				nbytes + 24, SYM4_INFO, NULL);
			if (frecl == 0) { return NUS_ERRNO; }
			ai->au_recl = frecl;
#if 0
			ai->au_recl = nbytes + 24;
			if ((frecl = nusndf_recl(ndf, ai->au_recl)) == 0) {
				return NUS_ERRNO;
			}
			ai->au_data = nus_malloc(frecl);
			nusndf_format_record(ndf, ai->au_data, ai->au_recl,
					SYM4_INFO);
#endif
			memcpy4((char *)ai->au_data + 16, (char *)&grp);
			ndf_auxtab_put(ndf->infotab, &grp, ai);
			ndf->infocount++;
			ndf->reccount++;
			nus_warn(("new INFO %Ps (not found in deffile %#ys)",
						grp,
						&ndf->comm.nustype));
		} else if (ai->au_recl < nbytes + 24) {
			nus_warn(("truncating INFO %Ps %zd => %zd (%#ys %s)",
				grp, nbytes, ai->au_recl - 24,
				&ndf->comm.nustype, ndf->comm.filename));
			nbytes = ai->au_recl - 24;
		}
	}
	r = encoder(ai->au_data + 20, nbytes, arg, ds);
	if (GlobalConfig(io_mark_end)) {
		df_flush(df);
	}
	return r;
}

	int
nusndf_creat_subc(struct ndf_t *ndf,
		struct dds_t *dds,
	struct nusdef_subcinfo_t *psi)
{
	struct ndf_aux_t	*ai;
	int r;
	ai = nus_malloc(sizeof *ai);
	ai->au_grp = psi->group;
	ai->au_pos = ndf->wpos;
	switch (psi->group) {
		case SYM4_TDIF:
		case SYM4_LOCA:
			ai->au_recl = ndf->nv * ndf->nm * psi->size + 24;
			break;
		case SYM4_RADR:
		case SYM4_RADS:
		case SYM4_DPRD:
		case SYM4_ISPC:
		case SYM4_THUN:
			ai->au_recl = ndf->nv * ndf->nm * psi->size + 24;
			break;
		case SYM4_ETA:
		case SYM4_SIGM:
		case SYM4_ZHYB:
		case SYM4_RGAU:
		default:
			ai->au_recl = psi->size + 24;
			break;
	}
	ai->au_recl = nusndf_new_record(
		&(ai->au_data), ndf, ai->au_recl, SYM4_SUBC, NULL);
	if (ai->au_recl == 0) {
		return NUS_ERRNO;
	}
	si8_addto(ndf->wpos, ulong_to_si8(ai->au_recl));
	nus_debug(("subc %Ps psi->size=%Pd", psi->group, psi->size));
	memcpy4((char *)ai->au_data + 16,
			(char *)&ai->au_grp);
	if (psi->group == SYM4_ETA && dds->comm.sc_eta) {
		memcpy(ai->au_data + 20, dds->comm.sc_eta->sp_contents,
			ai->au_recl - 24);
	} else if (psi->group == SYM4_SIGM && dds->comm.sc_sigm) {
		memcpy(ai->au_data + 20, dds->comm.sc_sigm->sp_contents,
			ai->au_recl - 24);
	} else if (psi->group == SYM4_ZHYB && dds->comm.sc_zhyb) {
		memcpy(ai->au_data + 20, dds->comm.sc_zhyb->sp_contents,
			ai->au_recl - 24);
	} else if (psi->group == SYM4_RGAU && dds->comm.sc_rgau) {
		memcpy(ai->au_data + 20, dds->comm.sc_rgau->sp_contents,
			ai->au_recl - 24);
	} else if (psi->group == SYM4_DELT && dds->comm.sc_delt) {
		memcpy(ai->au_data + 20, dds->comm.sc_delt->sp_contents,
			ai->au_recl - 24);
	} else {
		memset(ai->au_data + 20, 0xFF, ai->au_recl - 24);
	}
	r = ndf_auxtab_put(ndf->subctab, &ai->au_grp, ai);
	if (r) {
		return r;
	}
	
	ndf->subccount++;
	return 0;
}

	int
nusndf_creat_info(struct ndf_t *ndf,
	struct nusdef_subcinfo_t *psi)
{
	struct ndf_aux_t	*ai;
	size_t frecl;
	int r;
	ai = nus_malloc(sizeof *ai);
	ai->au_grp = psi->group;
	ai->au_pos = long_to_si8(0);
	ai->au_recl = 0;
	frecl = nusndf_new_record(&(ai->au_data), ndf,
		ai->au_recl + 24, SYM4_INFO, psi->filename);
	if (frecl == 0) {
		return NUS_ERRNO;
	}
	ai->au_recl = frecl;
	memcpy4((char *)ai->au_data + 16, (char *)&ai->au_grp);
	nus_debug(("info %Ps file=%s size=%Pu",
			psi->group, psi->filename, ai->au_recl));
	r = ndf_auxtab_put(ndf->infotab, &ai->au_grp, ai);
	if (r) {
		return r;
	}

	ndf->infocount++;
	return 0;
}

	INLINE int
SaveAuxName(sym4_t key, struct ndf_aux_t *val UNUSED, void *arg)
{
	sym4_t **names = arg;
	**names = key;
	(*names)++;
	return 0;
}

	static int
MemCmp4(const void *a, const void *b)
{
	return memcmp(a, b, 4);
}

	extern int
nusndf_inq_aux(union nusdfile_t *df, int query, sym4_t group,
		void *buf, N_UI4 bufnelems)
{
	struct ndf_t *ndf = &df->ndf;
	struct ndf_aux_t *cached;
	N_UI4 *iptr;
	sym4_t *sptr;
	N_UI4 recl;
	int r;
	if (buf == NULL) {
		return nus_err((NUSERR_IQ_DestNull, "buffer NULL"));
	}
	if (ndf->subctab == NULL) {
		r = ndf_auxtab_scan(ndf);
		if (r) return r;
	}
	switch (query) {
		case N_SUBC_NUM:
			iptr = buf;
			*iptr = ndf->subccount;
			r = 1;
			break;
		case N_INFO_NUM:
			iptr = buf;
			*iptr = ndf->infocount;
			r = 1;
			break;
		case N_SUBC_LIST:
			if (bufnelems < ndf->subccount) {
				return nus_err((NUSERR_IQ_ShortBuf,
					"buffer < %Pu elements",
					ndf->subccount));
			}
			sptr = buf;
			ndf_auxtab_each(ndf->subctab, SaveAuxName, &sptr);
			qsort(buf, ndf->subccount, 4, MemCmp4);
			r = ndf->subccount;
			break;
		case N_INFO_LIST:
			if (bufnelems < ndf->infocount) {
				return nus_err((NUSERR_IQ_ShortBuf,
					"buffer < %Pu elements",
					ndf->infocount));
			}
			sptr = buf;
			ndf_auxtab_each(ndf->infotab, SaveAuxName, &sptr);
			qsort(buf, ndf->subccount, 4, MemCmp4);
			r = ndf->infocount;
			break;
		case N_SUBC_NBYTES:
			cached = ndf_aux_getcached(ndf, SYM4_SUBC, group);
			goto nbytes_ready;
		case N_INFO_NBYTES:
			cached = ndf_aux_getcached(ndf, SYM4_INFO, group);
nbytes_ready:
			if (cached == NULL) {
				return NUS_ERRNO;
			}
			iptr = buf;
			*iptr = PEEK_N_UI4(cached->au_data + 8) - 16u;
			r = 1;
			break;
		case N_SUBC_CONTENT:
			cached = ndf_aux_getcached(ndf, SYM4_SUBC, group);
			goto content_ready;
		case N_INFO_CONTENT:
			cached = ndf_aux_getcached(ndf, SYM4_INFO, group);
content_ready:
			if (cached == NULL) {
				return NUS_ERRNO;
			}
			recl = PEEK_N_UI4(cached->au_data + 8) - 16u;
			if (bufnelems < recl) {
				return nus_err((NUSERR_IQ_ShortBuf,
					"buffer %Pu < needed %Pu bytes",
					bufnelems, recl));
			}
			memcpy(buf, cached->au_data + 20, recl);
			r = recl;
			break;
		default:
			r = nus_err((NUSERR_IQ_BadParam,
				"unsupported query code %d", query));
			break;
	}
	return r;
}
