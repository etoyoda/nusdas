/** @file
 * @brief �ꥹ�Ȥ�����ʤɤ����
 */

/** @brief �ݥ�������������󥯥ꥹ�ȤΥ���ȥ� */
struct listp_entry {
	void *obj;
	struct listp_entry *next;
};

/** @brief �ݥ����ѥꥹ�� */
typedef struct listp_t {
	unsigned num_entries;
	struct listp_entry *next;
	struct listp_entry *last;
} listp_t;

extern listp_t *listp_ini(void);
extern int listp_push(listp_t *c, void *obj);
extern int listp_each(listp_t *c,
		int (*callback)(void *obj, void *arg), void *arg);

/** @brief N_UI4 �ѡָ�������� (sys_ary4.c ����) */
typedef struct array4_t {
	/** ����Ĺ */
	unsigned num_entries;
	/** �ϥå���ơ��֥��Ĺ�� */
	unsigned num_hashtab;
	/** �ͤ����� (array4_ini() �ǳ��ݡ�array4_delete() ���˴�) */
	N_UI4		*list;
	/** �ϥå���ơ��֥� (�ͤ� num_hashtab �ǳ�ä���ΤˤĤ��ơ�
	 * �����ͤ��ǽ�˸���� list ��ź��) */
	unsigned	*hash;
} array4_t;

extern array4_t *array4_ini(unsigned nEntries);
extern int array4_set(array4_t *ary, unsigned idx, N_UI4 val);
extern N_UI4 array4_get_value(array4_t *ary, unsigned idx);
extern int array4_get_index(array4_t *ary, N_UI4 val);
#define array4_delete(ary) nus_free((ary))
#define array4_includep(ary, val) (array4_get_index(ary, val) != -1)

/** @brief N_UI4 �ѡֲ���Ĺ��������� (sys_ary4v.c ����) */
typedef struct array4v_t {
	/** �ºݤ˻Ȥ��Ƥ������ǿ� */
	unsigned num_entries;
	/** �������ݤ��줿���ǿ� */
	unsigned num_allocated;
	/** �ͤ����� (array4v_ini() �ǳ��ݡ�array4v_push() �ǳ�ĥ��
	 * array4v_delete() ���˴�) */
	N_UI4		*list;
	/** ����������Ӵؿ� */
	int		(*cmpfunc)(const void *a, const void *b);
} array4v_t;

extern array4v_t *
array4v_ini(unsigned tsize, int (*cmpfunc)(const void *a, const void *b));
extern void array4v_delete(array4v_t *ary);
extern int array4v_push(array4v_t *ary, N_UI4 val);
extern int array4v_get_index(array4v_t *ary, N_UI4 val);
extern void array4v_sort(array4v_t *ary);
#define array4v_size(ary) ((ary)->num_entries)
#define array4v_includep(ary, val) (array4v_get_index(ary, val) != -1)

/** @brief N_UI8 �ѡָ�������� (sys_ary8.c ����) */
typedef struct array8_t {
	/** ����Ĺ */
	unsigned num_entries;
	/** �ϥå���ơ��֥��Ĺ�� */
	unsigned num_hashtab;
	/** �ͤ����� (array8_ini() �ǳ��ݡ�array8_delete() ���˴�) */
	N_UI8		*list;
	/** �ϥå���ơ��֥� (�ͤ� num_hashtab �ǳ�ä���ΤˤĤ��ơ�
	 * �����ͤ��ǽ�˸���� list ��ź��) */
	unsigned	*hash;
} array8_t;

extern array8_t *array8_ini(unsigned nEntries);
extern int array8_set(array8_t *ary, unsigned idx, N_UI8 val);
extern N_UI8 array8_get_value(array8_t *ary, unsigned idx);
extern int array8_get_index(array8_t *ary, N_UI8 val);
#define array8_delete(ary) nus_free((ary))
#define array8_includep(ary, val) (array8_get_index(ary, val) != -1)

/** @brief ���ѥݥ����ѡָ�������� (sys_aryp.c ����) */
typedef struct arrayp_t {
	/** �ºݤ˻Ȥ��Ƥ������ǿ� */
	unsigned	num_entries;
	/** �������ݤ��줿���ǿ� */
	unsigned	num_allocated;
	/** �ͤ���ݥ��󥿤����� (arrayp_ini() �ǳ��ݡ� arrayp_push() ��
	 * ��ĥ�� arrayp_delete() ���˴�) */
	void		**list;
	/** ����������Ӵؿ� */
	int		(*cmpfunc)(const void *a, const void *b);
} arrayp_t;

extern arrayp_t * arrayp_ini(unsigned tsize,
	int (*cmpfunc)(const void * a, const void * b));
extern void arrayp_delete(arrayp_t *ary);
extern int arrayp_set(arrayp_t *ary, unsigned idx, void * val);
extern int arrayp_push(arrayp_t *ary, void * val);
extern void *arrayp_get_value(arrayp_t *ary, unsigned idx);
extern int arrayp_get_index(arrayp_t *ary, void * val);
extern void arrayp_sort(arrayp_t *ary);
extern int arrayp_movehead(arrayp_t *ary, void *target);
#define arrayp_size(ary) ((ary)->num_entries)

/** @brief char* ���� char* �Υϥå��� (pds_cntltab.c ����) */
typedef struct pds_cntltab_t pds_cntltab_t;
extern pds_cntltab_t *pds_cntltab_ini(unsigned nbins /** ����Ȣ�ο� */);
extern void pds_cntltab_delete(pds_cntltab_t *hp);
extern int pds_cntltab_put(pds_cntltab_t *hp, char *key, char *val);
extern char *pds_cntltab_get(pds_cntltab_t *hp, char *key);
