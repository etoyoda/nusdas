/** \file
 * \brief �����Х�ǡ������Ф�������δؿ����
 *
 * �ǡ������åȤ����դ��äưʹߤν����� DS �⥸�塼��ǹԤ���Τǡ�
 * �����ˤϥǡ������åȤ�õ���ȥ饤�֥�����Τ�����˷���
 * �����ؿ� (���� API �� nusdas.h �򸫤�) ���ܤ뤳�Ȥˤʤ롣
 */

#ifndef INTERNAL_TYPES_H
# error "include internal_types.h before glb.h"
#endif

/** @brief �����Х�����
 *
 * NuSDaS �饤�֥��λ��Ȥ���ǻ��Ȥ�����ѿ��������˽�����Ƥ��롣
 * �㳰�� sys_err.h ��  nus_dbg_enabled �� nus_wrn_enabled
 */
extern struct nusglb_param_t {
	int	gp_io_mark_end;
	int	gp_io_w_fclose;
	int	gp_io_r_fclose;
	int	gp_io_badgrid;
	N_SI4	gp_pc_missing_si4;
	float	gp_pc_missing_r4;
	double	gp_pc_missing_r8;
#define NRD_UNFIX	-1
	int	gp_nrd_override;
	/** @brief ����ե������ version ʸ���ʤ��Ȥ��ο����ե�������ǿ� */
	int	gp_pc_filever;
	/** @brief ��ü���뤫�ݤ����� */
	unsigned gp_pc_alignment;
	/** @brief nusdas_make_mask() �Ǻ��줿�ӥåȥޥ������� */
	const N_UI1	*gp_saved_mask;
	/** @brief nusdas_make_mask() �Ǻ��줿�ӥåȥޥ��������Ĺ�� */
	unsigned gp_saved_mask_size;
	/** @brief �ǡ����ե�������Ĥ����Ȥ�����򥭥�å��夷�Ƥ����� */
	int gp_pc_keep_closed_file;
	/* ������� internal_types.h */
	struct nusxds_param_t	gp_ds_param;
	/** @brief setvbuf(3) �����ͤΥХåե�Ĺ (kB ñ�̡������� 512) */
	N_UI4	gp_io_setvbuf;
	/** @brief ���� PATH ���� */
	char	gp_dds_forcedpath[64];
	/** @brief ES�ν�������� (MB) */
	N_UI4	gp_eio_psize;
	/** @brief ES����ʬ������ (MB) */
	N_UI4	gp_eio_ssize;
	/** @brief timestamp��񤭽Ф��ʤ����� */
	int     gp_no_timestamp;
	/* �ѥǥ��󥰤�̵ͭ������ˤϤʤ�ʤ��Ϥ���������̣�������Τ�
	 * ��¤���������֤��Ƥ��� */
	N_SI2	gp_pc_missing_si2;
	N_UI1	gp_pc_missing_ui1;
} nusglb_param;

/** @brief �����Х�����γƹ��ܤ򻲾Ȥ������
 */
#define GlobalConfig(name) (nusglb_param.gp_ ## name)
#define GlobalDSConfig(name) (GlobalConfig(ds_param).dsp_ ## name)

#define DynamicParam(param, name) \
	(DynParam(param, name) ? DynParam(param, name) : GlobalDSConfig(name))

/** ����� typ_dstab.c */
struct nustype_dstab;
/** ����� glb_typetab.c */
struct glb_typetab_t;
/** ����� dset.h */
union nusdset_t;

/* glb_dsscan.c */
extern int nusglb_pushdset(union nusdset_t *ds, int nrd);
extern int nusglb_dsscan(int (*callback)(union nusdset_t *ds, void *arg),
		void *arg);
extern union nusdset_t *nusglb_dsscan2(void);
extern int nusglb_dsscan_nustype(
		int (*callback)(union nusdset_t *ds, void *arg),
		nustype_t *nustype, void *arg);
extern union nusdset_t *nusglb_find_dset(nustype_t *nustype);
extern int nusglb_allds_push(void);

/* glb_typetab.c */
extern struct glb_typetab_t *glb_typetab_ini(unsigned nbins);
extern void glb_typetab_delete(struct glb_typetab_t *hp);
extern int glb_typetab_put(struct glb_typetab_t *hp, nustype_t *key,
		struct nustype_dstab *val);
extern struct nustype_dstab *
glb_typetab_get(struct glb_typetab_t *hp, nustype_t *key);

/* typ_dstab.c */
extern struct nustype_dstab *nustype_dstab_ini(void);
extern int nustype_dstab_each(struct nustype_dstab *dst,
		int (*callback)(union nusdset_t *, void *), void *arg);
extern union nusdset_t *nustype_dstab_first(struct nustype_dstab *dst);
extern int nustype_dstab_push(struct nustype_dstab *dst,
		union nusdset_t *ds);
extern int nustype_dstab_movehead(struct nustype_dstab *dst, 
				  union nusdset_t *ds);

/* glb_param.c */
extern int nusglb_cfgdone;
extern int nusglb_config(void);
#define NUSDAS_INIT (void)(nusglb_cfgdone || nusglb_config())

extern int nusdas_opts(struct nusxds_param_t *param, const unsigned char *str);

/* dset.c */
extern int nusdset_opt(struct nusxds_param_t *param, sym4_t key,
		const char *val);
extern int nusdset_scan(void);

/* io_opts.c */
extern int nusio_opt(struct nusxds_param_t *param, sym4_t key,
		const char *val);

/* glb_gc.c */
extern int nusglb_garbage_collect(void);

/* glb_type1.c */
extern int nusglb_intp_type1(sym8_t *type1);
