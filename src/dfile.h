/** @file
 * @brief �Ƽ�ǡ����ե�����˴ؤ��뷿������ؿ��������
 */

/** @brief �ǡ��������ɤ߹��߻����ڽФ�����.
 *
 * cr_xofs ����ξ���ڽФ���̵���Ĥޤ������ɤߤ�����ɽ�魯
 * */
struct cut_rectangle {
	int	cr_xofs;	/**< X �������ϰ��֤��� (0�Ϥޤ�ź��������) */
	int	cr_xnelems;	/**< X �������ǿ� */
	int	cr_yofs;	/**< Y �������ϰ��֤��� (0�Ϥޤ�ź��������) */
	int	cr_ynelems;	/**< Y �������ǿ� */
};

#define cut_rectangle_disable(p) \
	((p)->cr_xofs = (p)->cr_xnelems = (p)->cr_yofs = (p)->cr_ynelems = -1)
#define cut_rectangle_disabled(p) \
	((p)->cr_xofs < 0)
#define cut_rectangle_size(p) \
	((p)->cr_xnelems * (p)->cr_ynelems)

/** @brief ���ѼԤ����Ϥ���� read �Хåե� */
struct ibuffer_t {
	void		*ib_ptr;	/**< �ݥ��� */
	sym4_t		ib_fmt;		/**< �ǡ����� */
	N_UI4		nelems;		/**< ���ǿ� @todo N_NC �ξ��ΰ�̣ */
	struct cut_rectangle	ib_cut;	/**< �ڽФ����� */
};

/** @brief ���ѼԤ����Ϥ���� write �Хåե� */
struct obuffer_t {
	const void	*ob_ptr;	/**< �ݥ��� */
	sym4_t		ob_fmt;		/**< �ǡ����� */
	N_UI4		nelems;		/**< ���ǿ� @todo N_NC �ξ��ΰ�̣ */
	const N_UI1	*ob_mask;	/**< �ӥåȥޥ���. ds_write ������ */
};

/* ����ϲ� */
union nusdfile_t;
/* ����� dset.h */
union nusdset_t;

/** @brief �Ƽ�ǡ����ե�����˶��̤���᥽�åɤ�ɽ */
struct df_functab {
	/** @brief �Ĥ���ؿ� (�ƥ����ץ��ǽ) */
	int (*xdf_close)(union nusdfile_t *df);
	/** @brief ndf �۲������񸻤����������˾��Ǥ����� */
	int (*xdf_delete)(union nusdfile_t *df);
	/** @brief ��Ͽ�ɤߤ����ؿ� */
	int (*xdf_read)(union nusdfile_t *df, nusdims_t *dim,
			struct ibuffer_t *buf);
	/** @brief ��Ͽ�񤭹��ߴؿ� */
	int (*xdf_write)(union nusdfile_t *df, nusdims_t *dim,
			struct obuffer_t *buf);
	/** @brief �����Ĥ����ե�����򳫤�ľ���ؿ� */
	int (*xdf_reopen)(union nusdfile_t *df, int open_flags);
	/** @brief SUBC/INFO ��Ͽ�ɤߤ����ؿ� */
	int (*xdf_read_aux)(union nusdfile_t *df, sym4_t ttl, sym4_t grp,
			int (*callback)(const void *rec, N_UI4 siz,
				void *arg, union nusdset_t *ds, N_SI4 ofs_flg),
			void *arg, union nusdset_t *ds);
	/** @brief SUBC/INFO ��Ͽ�񤭹��ߴؿ� */
	int (*xdf_write_aux)(union nusdfile_t *df, sym4_t ttl, sym4_t grp,
			size_t nbytes,
			int (*encoder)(void *rec, N_UI4 siz,
				void *arg, union nusdset_t *ds),
			void *arg, union nusdset_t *ds);
	/** @brief CNTL ��Ͽ��礻�ؿ�. */
	int (*xdf_inq_cntl)(union nusdfile_t *df, sym4_t query, void *dest,
			void *arg);
	/** @brief �ʻҾ���񤭹��ߴؿ� */
	int (*xdf_write_grid)(union nusdfile_t *df,
			const char proj[4], const N_SI4 size[2],
			const float projparam[14], const char value[4]);
	/** @brief DATA ��Ͽ��礻 */
	int (*xdf_inq_data)(union nusdfile_t *df, nusdims_t *dim,
			int item, void *buf, N_UI4 bufnelems);
	/** @brief �񤭹��ߤ򴰷뤵����ؿ� */
	int (*xdf_flush)(union nusdfile_t *df);
	/** @brief SUBC/INFO ��Ͽ��礻�ؿ� */
	int (*xdf_inq_aux)(union nusdfile_t *df, int query,
			sym4_t group, void *buf, N_UI4 bufnelems);
};

/** @brief �ǡ����ե�������Ĥ���. ndf_close() ��
 * @param df �Ĥ��褦�Ȥ���ǡ����ե����� (nusdfile_t *)
 * @note �ե�����ϥ�ɥ�ʳ��λ� (�ä˥���)
 * �ϳ�������ʤ��Τ� df_close() �������ȥݥ��󥿤�ΤƤʤ����ȡ�
 * �����˺������ˤ� df_delete() ��Ȥ���
 */
#define df_close(df) \
	((df)->comm.methods.xdf_close((df)))

#define df_delete(df) \
	((df)->comm.methods.xdf_delete((df)))

/** @brief �ǡ����ե����뤫��1��Ͽ���ɤ�. ndf_read() ��
 * @param df �ɤ⤦�Ȥ���ǡ����ե����� (nusdfile_t *)
 * @param dim ��Ͽ�λ��� (nusdims_t *)
 * @param buf ���ϥХåե� (ibuffer_t *)
 */
#define df_read(df, dim, buf) \
	((df)->comm.methods.xdf_read((df), (dim), (buf)))

/** @brief �ǡ����ե������1��Ͽ���. ndf_write() ��
 * @param df �񤳤��Ȥ���ǡ����ե����� (nusdfile_t *)
 * @param dim ��Ͽ�λ��� (nusdims_t *)
 * @param buf ���ϥХåե� (obuffer_t *)
 */
#define df_write(df, dim, buf) \
	((df)->comm.methods.xdf_write((df), (dim), (buf)))

/** @brief �Ĥ����ǡ����ե�����򳫤�ľ��. ndf_reopen() ��
 * @param df �����Ĥ����ǡ����ե�����
 * @param open_flags �ե饰
 */
#define df_reopen(df, open_flags) \
	((df)->comm.methods.xdf_reopen((df), (open_flags)))

/** @brief �ǡ����ե����뤫��SUBC/INFO��Ͽ���ɤ�. nusndf_read_aux() ��
 * @param df �ǡ����ե����� (nusdfile_t *)
 * @param ttl ��Ͽ���� (sym4_t; SYM4_SUBC �ޤ��� SYM4_INFO)
 * @param grp ���롼��̾ (sym4_t; SYM4_ETA �ʤ�)
 * @param decoder �ɤ����Ͽ���������ؿ�
 *                 (int (*)(const void *rec, N_UI4 siz, void *arg,
 *                 union nusdset_t *ds))
 *                 �Ϥ���� rec, siz �ϳƼﵭϿ�������� grp ���������Τ�
 *                 ���ꡢSUBC ��Ͽ�ˤĤ��Ƥ�����ե������ subcntl ʸ
 *                 �˽�Ĺ�����������롣
 * @param arg �ǥ��������Ϥ����� (void *)
 * @param ds �ǥ��������Ϥ��ǡ������å� (union nusdset_t *)
 */
#define df_read_aux(df, ttl, grp, decoder, arg, ds) \
	((df)->comm.methods.xdf_read_aux((df), (ttl), (grp), (decoder), \
		(arg), (ds)))

/** @brief �ǡ����ե������ SUBC/INFO ��Ͽ��񤭹���. nusndf_write_aux() ��
 * @param df �ǡ����ե����� (nusdfile_t *)
 * @param ttl ��Ͽ���� (sym4_t; SYM4_SUBC �ޤ��� SYM4_INFO)
 * @param grp ���롼��̾ (sym4_t; SYM4_ETA �ʤ�)
 * @param size ��Ͽ�ΥХ��ȿ�
 * @param encoder ��Ͽ���������ؿ�
 *                 (int (*)(void *rec, N_UI4 siz, void *arg,
 *                 union nusdset_t *ds))
 *                 �Ϥ���� rec, siz �ϳƼﵭϿ�������� grp ���������Τ�
 *                 ���ꡢSUBC ��Ͽ�ˤĤ��Ƥ�����ե������ subcntl ʸ
 *                 �˽�Ĺ�����������롣
 * @param arg ���󥳡������Ϥ����� (void *)
 * @param ds ���󥳡������Ϥ��ǡ������å� (union nusdset_t *)
 */
#define df_write_aux(df, ttl, grp, size, encoder, arg, ds) \
	((df)->comm.methods.xdf_write_aux((df), (ttl), (grp), (size), \
					  (encoder), (arg), (ds)))

/** @brief CNTL ��Ͽ��礻�ؿ�
 *
 * ��礻���� query ��������ξ��
 * dest �ϥǡ����񤭹�����ݥ���,
 * ����ΤФ��������ǤˤĤ��ƸƤӽФ����ؿ��ؤΥݥ��󥿡�
 * arg ��ɬ�פʤ��Ф��� NULL �򤤤�롣
 * */
#define df_inq_cntl(df, query, dest, arg) \
	((df)->comm.methods.xdf_inq_cntl((df), (query), (dest), (arg)))

/** @brief CNTL ��Ͽ��񤭴ؿ�
 *
 * �Ͽ���ƥѥ�᥿���񤭤��롣
 */
#define df_write_grid(df, proj, size, projparam, value) \
	((df)->comm.methods.xdf_write_grid((df), \
				  (proj), (size), (projparam), (value)))

/** @brief DATA ��Ͽ��礻�ؿ�
 */
#define df_inq_data(df, dims, item, buf, bufnelems) \
	((df)->comm.methods.xdf_inq_data((df), (dims), \
					 (item), (buf), (bufnelems)))

/** @brief �ǡ����ե������̤��λ�񤭹��ߤ򽪤���. ndf_flush() ��
 * @param df �Ĥ��褦�Ȥ���ǡ����ե����� (nusdfile_t *)
 */
#define df_flush(df) \
	((df)->comm.methods.xdf_flush((df)))

/** @brief �ǡ����ե������ SUBC/INFO �쥳���ɤ˴ؤ�����礻
 *
 * ��: nusndf_inq_aux
 */
#define df_inq_aux(df, query, group, buf, bufnelems) \
	((df)->comm.methods.xdf_inq_aux((df), (query), (group), \
					(buf), (bufnelems)))

/** @brief �Ƽ�ǡ������åȤ���ͭ������� */
struct df_common_t {
	/** @brief �ؿ��ơ��֥� */
	struct df_functab methods;
	/** @brief �񤭹��߲ĥե饰 */
	int	flags;
	/** @brief �ե�����̾.
	 *
	 * ����: ndfopen_initfields() */
	const char *filename;
	/** @brief ����̾
	 */
	nustype_t nustype;
	/** @brief ������Ƥ���Ȥ��� */
	int	is_open;
	/** @brief �ǡ������åȤ�����
	 * @warning ���줢�뤬�Τ˥ǡ����ե������Ĥ��ƥǡ������åȤ�
	 * �ä��뤳�Ȥϵ�����ʤ�.
	 * */
	struct nusxds_param_t *ds_param;
};

#define df_is_closed(df)	((df)->comm.is_open == 0)
#define df_is_writable(df)	((df)->comm.flags & IO_WRITABLE)
#define df_param(df, name)	DynamicParam((df)->comm.ds_param, name)

/** @brief ��Ͽ����õ���Ѿ���
 *
 * �������äѤˤ��ä� stdio �� FILE ��¤�Τ��������롣
 */
struct nsf_t {
	/** @brief �ե����� */
	union nusio_t *sf_io;
	/** @brief ��ϿĹ�û���
	 *
	 * �쥳������Ƭ�� 4 �Х��Ȥ��ɤ߼�ä����ȡ������ͤ�û�����
	 * �쥳����Ĺ�����롣
	 * ɸ��Ū�� Fortran �������󥷥��ե������ 8, NuSDaS 1.0 �� 0.
	 * */
	N_SI4	sf_reclplus;
	/** @brief ľ�����ɤ߹��ޤ줿��Ͽ�ΰ���
	 *
	 * �ե�����ݥ��󥿤Ȥϰ��פ��ʤ��Τ����
	 * */
	N_SI8	sf_pos;
	/** @brief �����ɤ߹��ޤ�Ƥ������ϥХåե��ε���
	 *
	 * ����� -1 (nusnsf_ini() ������)
	 * */
	N_SI8	sf_base;
	/** @brief ľ�������ϥХåե�
	 *
	 * ����� -1 (nusnsf_ini() ������)
	 */
	N_SI8	sf_base_prev;
	/** @brief ���ϥХåե��Υ����� */
	N_SI8	sf_size;
	/** @brief ���ϥХåե��Υݥ��� */
	N_UI4	*sf_rec;
	/** ����õ����ε�Ͽ��Ĺ�� (ü����ü�ޤ�) */
	N_UI4	sf_recl;
	/** �ɤ߼��Хåե�Ĺ�����뤿��˥��եȤ���� */
	N_UI4	sf_rbuffer;
	/** ��ü���뤫�ݤ� */
	N_UI4	sf_align;
	/** ��ü�ѥХåե� */
	N_UI4	*sf_alibuf;
	/** ��ü�ѥХåե���Ĺ�� */
	N_UI4	sf_alibuflen;
};

/** @brief ����Ū NuSDaS �ǡ����ե����� */
struct ndf_t {
	/** @brief �ǡ������åȶ��̾��� */
	struct df_common_t	comm;
	/** @brief ����ե�����
	 * ����: ndf_open(), �̥벽: ndf_close().
	 */
	struct nsf_t		seqf;
	/** @brief �ե�����С������ */
	N_UI4			filever;
#define ndf_is_largefile(ndfp)  (ndfp->filever >= 13)
	/** @brief ������ο� �Ȥ��äƤ⺣��ɬ��1
	 *
	 * ����: ndfopen_initfields(), ndfopen_loadfile() ����񤭤��뤫��
	 */
	N_UI4			nb;
	/** @brief ���С��ο�
	 *
	 * ����: ndfopen_initfields(), ndfopen_loadfile() ����񤭤��뤫��
	 */
	N_UI4			nm;
	/** @brief ���С��ΰ���
	 *
	 * member out �ξ������ե�����ΰ���
	 */
	int				om;
	/** @brief �оݻ���ο�
	 *
	 * ����: ndfopen_initfields(), ndfopen_loadfile() ����񤭤��뤫��
	 */
	N_UI4			nv;
	/** @brief �оݻ���ΰ���
	 *
	 * validtime out �ξ������ե�����ΰ���
	 */
	int				ov;
	/** @brief �̤ο�
	 *
	 * ����: ndfopen_initfields(), ndfopen_loadfile() ����񤭤��뤫��
	 */
	N_UI4			nz;
	/** @brief ���Ǥο�
	 *
	 * ����: ndfopen_initfields(), ndfopen_loadfile() ����񤭤��뤫��
	 */
	N_UI4			ne;
	/** @brief ������Υꥹ�� �Ȥ��äƤ⺣��ɬ��1����
	 *
	 * ����: ndfopen_loadfile(), ���Ԥ����� ndf_creat().
	 */
	struct array4_t		*btime;
	/** ���С��Υꥹ��
	 *
	 * ����: ndfopen_loadfile(), ���Ԥ����� ndf_creat().
	 */
	struct array4_t		*member;
	/** �оݻ���Υꥹ�� (vt1, vt2 ��ѥå�)
	 *
	 * ����: ndfopen_loadfile(), ���Ԥ����� ndf_creat().
	 */
	struct array8_t		*vtime;
	/** �̤Υꥹ��
	 *
	 * ����: ndfopen_loadfile(), ���Ԥ����� ndf_creat().
	 * @note plane2 ���礱�Ƥ��� */
	struct array8_t		*plane;
	/** ���ǤΥꥹ�� */
	struct array8_t		*element;
	/** indx �˳�����Ƥ��Х��ȿ� */
	size_t			indxsize;
	/** INDX ��Ͽ���� @note nus_free ����Ȥ��˻Ȥ�
	 *
	 * ����: ndfopen_loadfile(), ���Ԥ����� ndf_creat().
	 */
	unsigned char		*indx;
	/** �ե�����С������11�ޤǤ� INDX �ơ��֥�
	 *
	 * ����� indx �Ȱ���Ū�˳��ݡ�
	 * �ե�����С������13�Ǥ� INDX ��8�Х���ñ�̤Ȥʤ뤿��
	 * �������Ǥϥ̥�Ȥʤ롣
	 *
	 * ����: ndfopen_loadfile(), ���Ԥ����� ndf_creat().
	 */
	N_UI4			*indx4;
	/** �ե�����С������ 13 �ʹߤε�ϿĹ�ơ��֥� */
	N_UI4			*indy_recl;
	/** �ե�����С������ 13 �ʹߤ����ǿ��ơ��֥� */
	N_UI4			*indy_nelems;
	/** x �����γʻҿ� */
	N_UI4			nx;
	/** y �����γʻҿ� */
	N_UI4			ny;
	/** �����ǥå��Υ���å��� (ľ���˻Ȥä������ǥå�) */
	struct ndf_codec_t	*codec;
	/** nusdcntl �˳�����Ƥ��Х��ȿ� */
	size_t			nusdsize;
	size_t			cntlsize;
	/** NUSD, CNTL ��Ͽ (���ϻ��Τ߻���) */
	unsigned char		*nusd;
	unsigned char		*cntl;
	/** ���� DATA ��Ͽ��񤱤���֡�
	 * ndfopen_initfields() ����������ꡢ
	 * ndfopen_loadfile() �ޤ��� ndf_creat() �����ꡣ
	 * ndf_write ��������
	 */
	N_UI8			wpos;
	/** �ե����������������
	 * �񤭹��߽����Ǹ��ndfclose_writehead�����ꤹ�롣
	 * �ɤ߹��߻������ꤵ��ʤ���
	 */
	N_UI8			totalsize;
	/** ��Ͽ�� */
	N_UI4			reccount;
	/** �ǡ����ѥå����� */
	sym4_t			packing;
	/** ��»ɽ������ */
	sym4_t			missing;
	/** INFO ��Ͽ��.
	 *
	 * ndfopen_loadfile �ǥ��ɤ���뤫��
	 * ndf_creat ������ե�����˽���ͥե������
	 * ��ά�����˽񤫤�Ƥ��� information ʸ�ο������ꤵ��롣
	 * (sitab �Υ���ȥ���ǤϤʤ����ºݤ˥ե�����˽񤫤줿��)
	 * ����ɲòġ�
	 * */
	N_UI4			infocount;
	/** SUBC ��Ͽ��.
	 *
	 * ndfopen_loadfile �ǥ��ɤ���뤫��
	 * ndf_creat ������ե�����ˤ���������ꤵ��롣
	 * ���θ���ɲäϵ�����ʤ���
	 * */
	N_UI4			subccount;
	/** CNTL ��Ͽ�λ��� */
	N_UI4			cntltime;
	/** ���ϥХåե�: ndfopen_initfields() �� GlobalConfig(pc_wbuffer)
	 * �˽�������դ��ޤ��� NULL ��. NULL �ʤ�Хåե���󥰤��ʤ�. */
	struct ndf_wbuf_t {
		/** ndf_open() ���� GlobalConfig(pc_wbuffer) ���ͤȤʤ� */
		N_UI4		siz;
		N_UI4		pos;
		N_SI8		ofs;
		unsigned char	*ptr;
	}			*wbuf;
	/** SUBC ��Ͽ��ɽ.
	 *
	 * SUBC ��Ͽ�ο��ϥե�����������˷��ꤷ�Ƥ���Τǡ�
	 * subctab ���ؤ� ndf_aux_t �����Τ� ndf_creat() �ޤ���
	 * ndf_auxtab_load() �ǰ�礷�ƥ������Ȥ��롣
	 */
	struct ndf_auxtab_t *subctab;
	/** INFO ��Ͽ��ɽ.
	 *
	 * INFO ��Ͽ�ο��Ϸ�ޤäƤ��ʤ��Τǡ�ndf_aux_t �����Τ�
	 * �ʤ�餫�η��ǥǡ������Ƥ� ndf �⥸�塼����Ϥ����Ȥ���
	 * ���̤˥������Ȥ���롣
	 */
	struct ndf_auxtab_t *infotab;
	/** ����Ĺ��Ͽ�ˤ���������Ȥʤ�
	 */
	long forcedrlen;
};

/** @brief ���ѥǡ����ե�����
 *
 * ���ΤϺ��ΤȤ��� ndf_t
 */
typedef union nusdfile_t {
	struct df_common_t	comm;
	struct ndf_t		ndf;
} nusdfile_t;

/** ����� dset.h */
struct dds_t;

/* ����� ndf_open.c */
extern nusdfile_t *
ndf_open(union nusio_t *io, const char *filename,
		int flags, struct dds_t *dds, const nusdims_t *dim);

/* ����� dfile.c */
extern nusdfile_t *
df_open(const char *filename, int flags, struct dds_t *dds, const nusdims_t *dim);
