/** @file
 * @brief NuSDaS データファイル (ndf モジュール) に関する宣言
 */

/** SUBC/INFO 記録表 */
struct ndf_aux_t {
	/** グループ名 (ETA, SIGM, TDIF, ...)
	 * または終端を示すゼロ */
	sym4_t		au_grp;
	/** 記録先頭位置 */
	N_UI8		au_pos;
	/** 記録長 (Fortran 記録全長, 確定前 0) */
	size_t		au_recl;
	/** 記録内容 (NULL でなければ si_recl バイト長) */
	N_UI1		*au_data;
};

/* 定義は dset.h */
struct nusdef_subcinfo_t;

#define nusndf_aux_eq(p, q) (*(p) == *(q))

/* 定義は ndf_open.c */
extern int nusndf_flushwrite(struct ndf_t *ndf);
extern N_UI4 nusndf_recl(struct ndf_t *ndf, N_UI4 vrecl);
extern N_UI4 *nusndf_load_cntl(union nusdfile_t *df);

/* 定義は ndf_aux.c */

extern size_t
nusndf_new_record(N_UI1 **datap, struct ndf_t *ndf,
	size_t minrecl, sym4_t recname, const char *infofile);

extern int
nusndf_check_format(N_UI1 *rec, size_t xrecl, N_SI4 reclplus);

extern int
nusndf_read_aux(union nusdfile_t *df, sym4_t ttl, sym4_t grp,
		int (*callback)(const void *rec, N_UI4 siz,
			void *arg, union nusdset_t *ds, N_SI4 ofs_flg),
		void *arg, union nusdset_t *ds);
extern int 
nusndf_write_aux(union nusdfile_t *df, sym4_t ttl, sym4_t grp, size_t nbytes,
		int (*encoder)(void *rec, N_UI4 siz,
			void *arg, union nusdset_t *ds),
		void *arg, union nusdset_t *ds);
extern int nusndf_creat_info(struct ndf_t *ndf, struct nusdef_subcinfo_t *psi);
extern int nusndf_creat_subc(struct ndf_t *ndf, struct dds_t *dds,
		struct nusdef_subcinfo_t *psi);
extern int nusndf_inq_aux(union nusdfile_t *df, int query, sym4_t group,
		void *buf, N_UI4 bufnelems);

/* 定義は ndf_inqcntl.c */
extern int
nusndf_inq_cntl(union nusdfile_t *df, sym4_t query, void *dest, void *arg);

/* 定義は ndf_auxtab.c */
extern struct ndf_auxtab_t *ndf_auxtab_ini(unsigned nbins /**< 仕訳箱の数 */);
extern void ndf_auxtab_delete(struct ndf_auxtab_t *hp);
extern int
ndf_auxtab_put(struct ndf_auxtab_t *hp, sym4_t *key, struct ndf_aux_t *val);
extern struct ndf_aux_t *ndf_auxtab_get(struct ndf_auxtab_t *hp,
		const sym4_t *key);
extern int ndf_auxtab_each(struct ndf_auxtab_t *hp,
	int (*callback)(sym4_t key, struct ndf_aux_t *val, void *arg),
	void *arg);

/* ndf_grid.c */
extern int ndf_grid_check(N_UI4 *cntl);

/* ndf_seqf.c */
extern void nusnsf_ini(struct nsf_t *sf, union nusio_t *io, N_SI4 reclplus,
		N_UI4 pc_rbuffer);
extern int nusnsf_read_head(struct nsf_t *sf);
extern int nusnsf_read_full(struct nsf_t *sf);
extern void *nusnsf_read_rec(struct nsf_t *sf);
extern void *nusnsf_read_at(struct nsf_t *sf, N_SI8 pos, N_SI8 size);
extern int nusnsf_read_recl(struct nsf_t *sf, N_SI8 pos, N_UI4 *recl);
#define nusnsf_read_to(sf, pos, recl, data) \
	io_read((sf)->sf_io, (pos), (recl), (data))
extern void *nusnsf_read_before(struct nsf_t *sf, N_SI8 pos);
extern int nusnsf_rewind(struct nsf_t *sf);
extern int nusnsf_write(struct nsf_t *sf, N_SI8 pos, N_UI4 size, void *data);
extern int nusnsf_write_seq(struct nsf_t *sf, N_UI4 size, void *data);
extern void *nusnsf_getwbuf(struct nsf_t *sf, N_SI8 pos, N_UI4 size);
extern int nusnsf_issue(struct nsf_t *sf, N_SI8 pos, N_UI4 size);
extern int nusnsf_close(struct nsf_t *sf, N_SI8 totalsize, const char* filename);
