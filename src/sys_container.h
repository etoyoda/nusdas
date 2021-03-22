/** @file
 * @brief リストや配列などの操作
 */

/** @brief ポインタ用片方向リンクリストのエントリ */
struct listp_entry {
	void *obj;
	struct listp_entry *next;
};

/** @brief ポインタ用リスト */
typedef struct listp_t {
	unsigned num_entries;
	struct listp_entry *next;
	struct listp_entry *last;
} listp_t;

extern listp_t *listp_ini(void);
extern int listp_push(listp_t *c, void *obj);
extern int listp_each(listp_t *c,
		int (*callback)(void *obj, void *arg), void *arg);

/** @brief N_UI4 用「賢い配列」 (sys_ary4.c 参照) */
typedef struct array4_t {
	/** 配列長 */
	unsigned num_entries;
	/** ハッシュテーブルの長さ */
	unsigned num_hashtab;
	/** 値の配列 (array4_ini() で確保、array4_delete() で破棄) */
	N_UI4		*list;
	/** ハッシュテーブル (値を num_hashtab で割ったものについて、
	 * その値が最初に現れる list の添字) */
	unsigned	*hash;
} array4_t;

extern array4_t *array4_ini(unsigned nEntries);
extern int array4_set(array4_t *ary, unsigned idx, N_UI4 val);
extern N_UI4 array4_get_value(array4_t *ary, unsigned idx);
extern int array4_get_index(array4_t *ary, N_UI4 val);
#define array4_delete(ary) nus_free((ary))
#define array4_includep(ary, val) (array4_get_index(ary, val) != -1)

/** @brief N_UI4 用「可変長賢い配列」 (sys_ary4v.c 参照) */
typedef struct array4v_t {
	/** 実際に使われている要素数 */
	unsigned num_entries;
	/** メモリを確保された要素数 */
	unsigned num_allocated;
	/** 値の配列 (array4v_ini() で確保、array4v_push() で拡張、
	 * array4v_delete() で破棄) */
	N_UI4		*list;
	/** ソート用比較関数 */
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

/** @brief N_UI8 用「賢い配列」 (sys_ary8.c 参照) */
typedef struct array8_t {
	/** 配列長 */
	unsigned num_entries;
	/** ハッシュテーブルの長さ */
	unsigned num_hashtab;
	/** 値の配列 (array8_ini() で確保、array8_delete() で破棄) */
	N_UI8		*list;
	/** ハッシュテーブル (値を num_hashtab で割ったものについて、
	 * その値が最初に現れる list の添字) */
	unsigned	*hash;
} array8_t;

extern array8_t *array8_ini(unsigned nEntries);
extern int array8_set(array8_t *ary, unsigned idx, N_UI8 val);
extern N_UI8 array8_get_value(array8_t *ary, unsigned idx);
extern int array8_get_index(array8_t *ary, N_UI8 val);
#define array8_delete(ary) nus_free((ary))
#define array8_includep(ary, val) (array8_get_index(ary, val) != -1)

/** @brief 汎用ポインタ用「賢い配列」 (sys_aryp.c 参照) */
typedef struct arrayp_t {
	/** 実際に使われている要素数 */
	unsigned	num_entries;
	/** メモリを確保された要素数 */
	unsigned	num_allocated;
	/** 値たるポインタの配列 (arrayp_ini() で確保、 arrayp_push() で
	 * 拡張、 arrayp_delete() で破棄) */
	void		**list;
	/** ソート用比較関数 */
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

/** @brief char* から char* のハッシュ (pds_cntltab.c 参照) */
typedef struct pds_cntltab_t pds_cntltab_t;
extern pds_cntltab_t *pds_cntltab_ini(unsigned nbins /** 仕訳箱の数 */);
extern void pds_cntltab_delete(pds_cntltab_t *hp);
extern int pds_cntltab_put(pds_cntltab_t *hp, char *key, char *val);
extern char *pds_cntltab_get(pds_cntltab_t *hp, char *key);
