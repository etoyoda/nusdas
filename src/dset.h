/** @file
 * @brief �ǡ������åȤ˴ؤ��뷿������ؿ�����.
 */

#ifndef NULL
# error "please use stddef.h for size_t"
#endif
#ifndef INTERNAL_TYPES_H
# error "include internal_types.h for sym4_t, N_SI4 etc."
#endif

/** @brief nusdef_t �������֤���� SUBC �ޤ��� INFO ��Ͽ�ξ���
 */
typedef struct nusdef_subcinfo_t {
	/** SUBC ��Ͽ�Υ����� (INFO �Ǥϥ���)  */
	N_SI4	size;
	/**  ���롼��̾ */
	sym4_t	group;
	/** INFO ��Ͽ�ν�����ե�����̾ */
	char	*filename;
	/** ���Υ��롼��̾�ؤΥݥ��� */
	struct nusdef_subcinfo_t *next;
} nusdef_subcinfo_t;

/** @brief NuSDaS ����ե�����Υǥ����ɷ��
 */
typedef struct {
	/** �С�������ֹ� */
	int		version;
	/** �ǡ������� */
	nustype_t	nustype;
	/** ���������ꤹ���礽���̻�ʬ���͡��̾�� -1 */
	N_SI4		basetime;
	/** NRD ��Υǡ����ե�����ǥ��쥯�ȥ�Υƥ�ץ졼�� (�̥뽪ü) */
	char		path[256];
	/** NRD ��Υǡ����ե�����̾�Υƥ�ץ졼�� (�̥뽪ü) */
	char		filename[80];
	/** �ǡ���������̾�� (�̥뽪ü).
	 * �ºݤˤ� 72 �Х��Ȥ����Ȥ�ʤ������ե��������11��
	 * �񤭽Ф��ݤ������̤������ʤ�����ˤ������Ƥ��� */
	char		creator[80];
	/** ���С��� */
	int		n_mb;
	/** ���ʤ�С��̤Υ��С��ε�Ͽ���̤Υե�����˽񤫤�� */
	int		mb_out;
	/** ���С�̾������ */
	sym4_t		*mb;
	/** �оݻ���ο� */
	int		n_vt;
	/**
	 * �оݻ���ˤ��ǡ����ե�����ʬ����ˡ
	 * \li\c 0 ���٤Ƥ��оݻ��郎Ʊ��Υǡ����ե�����˽񤫤��
	 * \li\c 1 ���줾����оݻ��郎���̤Υǡ����ե�����˽񤫤��
	 * \li\c >1 �оݻ��� @p vt_out ��ʬ�Υǡ�������ĤΥե�����˽񤫤��
	 */
	int		vt_out;
	/** ͽ�����1������ */
	int		*ft1;
	/** ͽ�����2������ */
	int		*ft2;
	/** �оݻ����ñ�� */ 
	sym4_t		ftunits;
	/** �̤ο� */
	int		n_lv;
	/** ��1 */
	sym8_t		*lv1;
	/** ��2 */
	sym8_t		*lv2;
	/** ���Ǥο� */
	int		n_el;
	/** ����̾������ */
	sym8_t		*el;
	/** ���С����оݻ�����̡����� �������ܤ���ɽ */
	N_UI4		*elementmap;
	/** X(�����Ƥ�����)�ʻҿ� */
	int		nx;
	/** Y(�����Ƥ�����)�ʻҿ� */
	int		ny;
	/** ���ˡ���� */
	float		projparam[7][2];
	/** �Ƴʻ����ͤ���򤤤�����ɽ���뤫 */
	sym4_t		value;
	/** �ѥå����������� */
	sym4_t		packing;
	/** ��»��ɽ��ˡ */
	sym4_t		missing;
	/** SUBC ��Ͽ�ξ��� */
	struct nusdef_subcinfo_t	*subctab;
	/** INFO ��Ͽ�ξ��� */
	struct nusdef_subcinfo_t	*infotab;
	/** SUBC ��Ͽ�θĿ� */
	int		n_subc;
	/** INFO ��Ͽ�θĿ� */
	int		n_info;
	/** ����Ĺ��Ͽ���ץ����
	 * \li\c 0: �ǥե���Ȥβ���Ĺ��Ͽ
	 * \li\c >0: ����Ĺ��Ͽ�������ͤϵ�ϿĹ
	 * */
	long		forcedrlen;
	/** ���ץ��������ʸ����
	 * �̥�����: nusdef_init(), OPTIONS ʸ������а��������ꤵ��롣
	 */
	char		options[256];
	/** �ɤߤ����Υե�����ǡ����ؤΥݥ��󥿡�
	 * �� ����̤��λ�ե饰��
	 * def_read.c �ʳ��Ǥ� struct lex_buffer ��̤����Ǥ���
	 * ������¤�ϻ��ȤǤ��ʤ�(ɬ�פ�ʤ��Ȼפ���)��
	 */
	struct lex_buffer		*lexbuffer;
} nusdef_t;

/* ����ϲ��ˤ��� */
union nusdset_t;
/* ����� dfile.h �ˤ��� */
struct ibuffer_t;
struct obuffer_t;
union nusdfile_t;

/* ds_inq_grid ���䤤��碌�ѥ�᥿ */
struct inq_grid_info {
	nusdims_t nusdims; /**< ���� */
	char *proj; /**< ���ˡ̾�Τγ�Ǽ�� (Ĺ��4ʸ����) */
	N_SI4 *gridsize; /**< �ʻҥ������γ�Ǽ�� (2��������) */
	float *gridinfo; /**< ���ˡ�ѥ�᥿�γ�Ǽ�� (14��������) */
	char *value; /**< ������ɽ��̾�Τγ�Ǽ�� (Ĺ��4ʸ����) */
};

/** @brief �Ƽ�ǡ������åȤ˶��̤���᥽�åɤ�ɽ��
 */
struct ds_functab {
	/** ��°�ե�����������Ĥ��� */
	int (*xds_close)(union nusdset_t *ds, int flags);
	/** ���꥽���������������˾��Ǥ��� */
	int (*xds_delete)(union nusdset_t *ds);
	/** ����å���ʤ����פʥ꥽�����������Ѵ����ƿȷڤˤʤ� */
	int (*xds_compact)(union nusdset_t *ds);
	/** �ǡ����ե������ߤĤ���ؿ� */
	union nusdfile_t *(*xds_findfile)(union nusdset_t *ds,
			const nusdims_t *dim, int open_flags);
	/** �ǡ����ե������ߤĤ���1��Ͽ�ɤ�ؿ� */
	int (*xds_readdata)(union nusdset_t *ds, nusdims_t *dim,
			struct ibuffer_t *buf);
	/** �ǡ����ե�������Υǡ�����Ͽ���Ф�����礻 */
	int (*xds_inq_data)(union nusdset_t *ds, nusdims_t *dim,
			int item, void *buf, N_UI4 buf_nelems);
	/** �ǡ����ե������ߤĤ���1��Ͽ�񤯴ؿ� */
	int (*xds_writedata)(union nusdset_t *ds, nusdims_t *dim,
			struct obuffer_t *buf);
	/** �ǡ����ե������ߤĤ��� SUBC/INFO ��Ͽ��1���ɤ�ؿ� */
	int (*xds_read_aux)(union nusdset_t *ds, const nusdims_t *dim,
		sym4_t rectitle, sym4_t recgroup,
		int (*callback)(const void *rec, N_UI4 siz, void *arg,
			union nusdset_t *dset, N_SI4 ofs_flg),
		void *arg);
	/** �ǡ������åȤ�����ե�����˻������������������Ƥ���뤫.
	 *
	 * @param grp ������������롼��̾
	 * @param nbytes ����������ΥХ��ȿ� (����������"����ե������Ĺ��")
	 * @retval 0 ����ʤ�
	 * @retval ¾ �������롼��̾�ϵ�����Ƥ��ʤ� (nus_err ���кѤ�)
	 * @retval ¾ �Х��ȿ����ۤʤ� (nus_err ���кѤ�)
	 * */
	int (*xds_subc_namecheck)(union nusdset_t *ds, sym4_t grp,
			size_t nbytes);
	/** �ǡ������åȤ�°���������ꥹ�Ȥ����
	 * @param data ��̳�Ǽ����
	 * @param data_nelems ��̳�Ǽ�������ǿ�
	 * @param verbose �в�ɽ���ե饰 (����ΤȤ��в��ɽ��)
	 * @retval �� ���顼
	 * @retval 0 �ǡ������åȤϤ��뤬õ���оݤ��ߤĤ���ʤ�
	 * @retval �� �������˳�Ǽ���٤�����θĿ�
	 *    (data_nelems ��Ķ�����硢data_nelems �ʾ�Ͻ񤭹��ޤ�ʤ�)
	 */
	int (*xds_btlist)(union nusdset_t *ds,
			N_SI4 data[], N_SI4 data_nelems, int verbose);
	/** �ǡ������åȤ�°�����оݻ���ꥹ�Ȥ����
	 * @param data ��̳�Ǽ����
	 * @param data_nelems ��̳�Ǽ�������ǿ�
	 * @param basetime ��������-1 �ʤ�и���ʤ�
	 * @param verbose �в�ɽ���ե饰 (����ΤȤ��в��ɽ��)
	 * @retval �� ���顼
	 * @retval 0 �ǡ������åȤϤ��뤬õ���оݤ��ߤĤ���ʤ�
	 * @retval �� �������˳�Ǽ���٤�����θĿ�
	 *    (data_nelems ��Ķ�����硢data_nelems �ʾ�Ͻ񤭹��ޤ�ʤ�)
	 */
	int (*xds_vtlist)(union nusdset_t *ds,
			N_SI4 data[], N_SI4 data_nelems,
			N_SI4 basetime, int verbose);
	/** SUBC/INFO �쥳���ɤ˴ؤ�����礻
	 *
	 * @param ds �ǡ������å�
	 * @param dims �ե���������Τ���μ���
	 * @param query ��礻������
	 * @param group �쥳���ɤη�̾
	 * @param buf ��̳�Ǽ����
	 * @param arg ������ (��礻�����ɤˤ��)
	 * @param keep_open ����ˤ���ȥե�������Ĥ��ʤ�
	 * @retval �� �������˳�Ǽ���줿���ǿ�
	 * @retval �� ���顼
	 */
	int (*xds_inq_aux)(union nusdset_t *ds, const nusdims_t *dims,
			int query, sym4_t group, void *buf, N_UI4 bufnelems);
	/** �ǡ����ե�����˴ؤ����䤤��碌
	 */
	int (*xds_inq_cntl)(union nusdset_t *ds, const nusdims_t *dims,
			int query, void *buf, void *arg,
			int keep_open);
	/** �ǡ����ե������ߤĤ��� SUBC/INFO ��Ͽ��1�Ľ� */
	int (*xds_write_aux)(union nusdset_t *ds, const nusdims_t *dim,
		sym4_t rectitle, sym4_t recgroup,
		size_t nbytes,
		int (*callback)(void *rec, N_UI4 siz, void *arg,
			union nusdset_t *dset),
		void *arg);
	/** �ե������1�Ĥ����Ĥ���ؿ� */
	int (*xds_close_file)(union nusdset_t *ds, const nusdims_t *dim);
	/** ����ե�����ؤ���礻 */
	int (*xds_inq_def)(union nusdset_t *ds, N_SI4 param,
		void *data, N_SI4 datasize);
	/** �ǡ����ե�����˴ؤ����䤤��碌 (GRID ����)
	 */
	int (*xds_inq_grid)(union nusdset_t *ds, struct inq_grid_info *info);
};

#define ds_readdata(ds, dimp, buf) \
	((ds)->comm.methods.xds_readdata((ds), (dimp), (buf)))
#define ds_inq_data(ds, dimp, item, buf, bufnelems) \
	((ds)->comm.methods.xds_inq_data((ds), (dimp), \
					 (item), (buf), (bufnelems)))
#define ds_writedata(ds, dimp, buf) \
	((ds)->comm.methods.xds_writedata((ds), (dimp), (buf)))
/** ������: dds_close() */
#define ds_close(ds, flags) \
	((ds)->comm.methods.xds_close((ds), (flags)))
#define ds_delete(ds) \
	((ds)->comm.methods.xds_delete((ds)))
#define ds_compact(ds) \
	((ds)->comm.methods.xds_compact((ds)))
#define ds_close_file(ds, dim) \
	((ds)->comm.methods.xds_close_file((ds), (dim)))
#define ds_findfile(ds, dim, open_flags) \
	((ds)->comm.methods.xds_findfile((ds), (dim), (open_flags)))
/** ������: dds_read_aux() */
#define ds_read_aux(ds, dim, ttl, grp, callback, arg) \
	((ds)->comm.methods.xds_read_aux((ds), (dim), (ttl), (grp), \
					 (callback), (arg)))
#define ds_subcpreset(ds, grp, nbytes, encoder, arg) \
	(nusxds_subcpreset((ds), (grp), (nbytes), (encoder), (arg)))
#define ds_subc_namecheck(ds, grp, nbytes) \
	((ds)->comm.methods.xds_subc_namecheck((ds), (grp), (nbytes)))
#define ds_btlist(ds, data, data_nelems, verb) \
	((ds)->comm.methods.xds_btlist((ds), (data), (data_nelems), (verb)))
#define ds_vtlist(ds, data, data_nelems, basetime, verb) \
	((ds)->comm.methods.xds_vtlist((ds), (data), (data_nelems), \
				       (basetime), (verb)))
#define ds_write_grid(ds, dim, proj, size, projparam, value) \
	(nusxds_write_grid((ds), (dim), (proj), (size), (projparam), (value)))
/** ������: dds_inq_aux() */
#define ds_inq_aux(ds, dims, query, group, buf, bufnelems) \
	((ds)->comm.methods.xds_inq_aux((ds), (dims), (query), (group), \
					(buf), (bufnelems)))
/** ������: dds_inq_cntl() */
#define ds_inq_cntl(ds, dims, query, buf, arg, keep_open) \
	((ds)->comm.methods.xds_inq_cntl((ds), (dims), (query), \
					(buf), (arg), (keep_open)))
/** ������: dds_write_aux() */
#define ds_write_aux(ds, dim, ttl, grp, nbytes, callback, arg) \
	((ds)->comm.methods.xds_write_aux((ds), (dim), (ttl), (grp), \
					 (nbytes), (callback), (arg)))
/** ������: dds_inq_def() */
#define ds_inq_def(ds, param, data, datasize) \
	((ds)->comm.methods.xds_inq_def((ds), (param), (data), (datasize)))
/** ������: dds_inq_grid() */
#define ds_inq_grid(ds, info) \
	((ds)->comm.methods.xds_inq_grid((ds), (info)))

/** @brief SUBC ��Ͽ�ν����
 */
struct ds_preset_t {
	/** ���롼��̾ */
	sym4_t		sp_grp;
	/** �Х��ȿ� */
	unsigned	sp_nbytes;
	/** ��Ͽ���� */
	N_UI1		*sp_contents;
};

/** @brief �Ƽ�ǡ������åȤ���ͭ�������.
 */
struct ds_common_t {
	nustype_t		nustype;
	int			nrd;
	struct ds_functab	methods;
	/** SUBC ETA ��Ͽ�Υǥե������
	 * dds_init �� NULL �����, nusxds_subcpreset ������ */
	struct ds_preset_t	*sc_eta;
	/** SUBC SIGM ��Ͽ�Υǥե������
	 * dds_init �� NULL �����, nusxds_subcpreset ������ */
	struct ds_preset_t	*sc_sigm;
	/** SUBC ZHYB ��Ͽ�Υǥե������
	 * dds_init �� NULL �����, nusxds_subcpreset ������ */
	struct ds_preset_t	*sc_zhyb;
	/** SUBC RGAU ��Ͽ�Υǥե������
	 * dds_init �� NULL �����, nusxds_subcpreset ������ */
	struct ds_preset_t	*sc_rgau;
	/** SUBC DELT ��Ͽ�Υǥե������
	 * dds_init �� NULL �����, nusxds_subcpreset ������ */
	struct ds_preset_t	*sc_delt;
	/** ưŪ����ѥ�᥿
	 * dds_init �ǽ����, ����API���Ѱ�ͽ�� */
	struct nusxds_param_t	param;
	/** �ǡ���̵���ե饰 
	 * dds_init �� 0 �˽���� */
	int dead_flag;
};


/** @brief ����ե����뷿�ǡ������åȤΥե�����̾Ÿ���ƥ�ץ졼��.
 */
struct dds_template {
	/** @brief fullpath ��Ĺ�� (��ü�̥������ʤ�) */
	size_t	pathlen;
	/** @brief �ե�����̾�ե�ѥ�
	 *
	 * ��������������С����оݻ��������.
	 * ����: nusdds_build_template(), ����: ̤�� */
	char	*fullpath;
	/** @brief ���Хѥ����ե��å�
	 *
	 * fullpath + rel �� NRD ��������Хѥ��ˤʤ� */
	int	rel;
	/** @brief �����������߰���
	 *
	 * ������� fullpath + b ���������ޤ�롣����ʤ��� -1 */
	int	b;
	/** @brief ���С�̾�����߰���
	 *
	 * ���С��� fullpath + m ���������ޤ�롣����ʤ��� -1 */
	int	m;
	/** @brief �оݻ��������߰���
	 *
	 * �оݻ���� fullpath + v ���������ޤ�롣����ʤ��� -1 */
	int	v;
};

/** ����� dds_dftab.c */
struct dds_dftab_t;

/** @brief ����ե����뷿�ǡ������å�
 * @todo tar ���ǡ������åȤϤɤ����褦?
 */
struct dds_t {
	/** @brief �ǡ������åȶ��̾���
	 *
	 * (��Ƭ���ǤǤʤ���Фʤ�ʤ�) */
	struct ds_common_t	comm;
	/** @brief ����ե��������
	 *
	 * �ͤ� dds_open() �� nusdef_init()/nusdef_readfile() �����
	 * �ƽ꤫��ƤФ�� nusdef_endparse() �����ꤵ���.
	 * */
	nusdef_t		def;
	/** @brief NRD �ǥ��쥯�ȥ�̾.
	 *
	 * ����: dds_init(), �˴�: dds_delete()(����).
	 * */
	const char		*dirname;
	/** @brief �ե�����̾Ÿ���ƥ�ץ졼��
	 */
	struct dds_template	tmpl;
	/** @brief �ե����륭��å���
	 *
	 * ����: dds_init(), �˴�: dds_delete()(����).
	 */
	struct dds_dftab_t	*dftab;
};

/** ����� ndf_codec.h */
struct ndf_codec_t;

/** @brief �ѥ�ɥ�ǡ������å�
 */
struct pds_t {
	struct ds_common_t	comm;
	char			*server;
	unsigned		port;
	char			*path;
	struct ndf_codec_t	*codec;
};

/** @brief ���ѥǡ������å�
 */
typedef union nusdset_t {
	struct ds_common_t	comm;
	struct dds_t		dds;
	struct pds_t		pds;
} nusdset_t;

/* === class DDS < XDS === */
/* --- dds_open.c --- */
extern nusdset_t *
dds_open(const char *dirname, const char *deffile, int nrd);
extern int dds_endparse(struct dds_t *dds);
extern union nusdfile_t *
nusdds_open_dfile(nusdset_t *ds, char *fullpath, const nusdims_t *dim,
		int open_flags);

/* --- dds_times.c --- */
extern N_SI4
nusdds_btlist(nusdset_t *ds, N_SI4 *data, N_SI4 data_nelems, int verbose);
extern N_SI4
nusdds_vtlist(nusdset_t *ds, N_SI4 *data, N_SI4 data_nelems,
		N_SI4 basetime, int verbose);

/* --- dds_find.c --- */
extern int nusdds_scan(void);

/* === class nusdds_template === */
/* --- dds_tmpl.c --- */
extern int nusdds_build_template(struct dds_t *dds, N_SI4 fill_basetime);
extern char *nusdds_tmpl_expand(struct dds_template *tmpl,
		const nusdims_t *dim);
extern N_UI4 *nusdds_tmpl_scanglob(struct dds_template *tmpl);

/* === class Nusdef === */
/* --- def_read.c --- */
extern int nusdef_readfile(const char *filename, nusdef_t *def);
extern int nusdef_init(nusdef_t *def);
extern int nusdef_endparse(nusdef_t *def);
extern sym4_t nusdef_projcode(nusdef_t *def);
extern sym4_t nusdef_projcode2(nusdef_t *def);

#define nusdef_immature(defptr)		((defptr)->lexbuffer)

/* === class dds_dftab === */
/* --- dds_dftab.c --- */
extern struct dds_dftab_t *dds_dftab_ini(unsigned nbins);
extern void dds_dftab_delete(struct dds_dftab_t *hp);
extern int dds_dftab_put(struct dds_dftab_t *hp, char *key,
		union nusdfile_t *val);
extern union nusdfile_t *dds_dftab_get(struct dds_dftab_t *hp,
		const char *key);
typedef int (*dftab_each_callback)(char *k, union nusdfile_t *v, void *arg);
extern int dds_dftab_each(struct dds_dftab_t *hp,
		dftab_each_callback callback, void *arg);
extern void dds_dftab_reject(struct dds_dftab_t *hp,
		int (*callback)(union nusdfile_t *val));

/* === class PDS < XDS === */
/* --- pds_open.c --- */
extern int nuspds_scan(void);

/* === class XDS === */
/* --- dset.c -- */
extern int
nusxds_subcpreset(union nusdset_t *ds, sym4_t grp, size_t nbytes,
		int (*encoder)(void *rec, N_UI4 siz, void *arg,
			union nusdset_t *ds), void *arg);

extern int
nusxds_write_grid(union nusdset_t *ds, nusdims_t *dim,
		const char proj[4],
		const N_SI4 size[2],
		const float projparam[14],
		const char value[4]);
