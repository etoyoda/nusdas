/** @file
 * @brief データセットに関する型宣言・関数原型.
 */

#ifndef NULL
# error "please use stddef.h for size_t"
#endif
#ifndef INTERNAL_TYPES_H
# error "include internal_types.h for sym4_t, N_SI4 etc."
#endif

/** @brief nusdef_t 内部に置かれる SUBC または INFO 記録の情報
 */
typedef struct nusdef_subcinfo_t {
	/** SUBC 記録のサイズ (INFO ではゼロ)  */
	N_SI4	size;
	/**  グループ名 */
	sym4_t	group;
	/** INFO 記録の初期化ファイル名 */
	char	*filename;
	/** 次のグループ名へのポインタ */
	struct nusdef_subcinfo_t *next;
} nusdef_subcinfo_t;

/** @brief NuSDaS 定義ファイルのデコード結果
 */
typedef struct {
	/** バージョン番号 */
	int		version;
	/** データ種別 */
	nustype_t	nustype;
	/** 基準時刻を固定する場合その通算分数値、通常は -1 */
	N_SI4		basetime;
	/** NRD 内のデータファイルディレクトリのテンプレート (ヌル終端) */
	char		path[256];
	/** NRD 内のデータファイル名のテンプレート (ヌル終端) */
	char		filename[80];
	/** データ作成者名称 (ヌル終端).
	 * 実際には 72 バイトしか使わないが、ファイル形式11を
	 * 書き出す際に不定結果を伝えないためにこうしている */
	char		creator[80];
	/** メンバー数 */
	int		n_mb;
	/** 真ならば、別のメンバーの記録は別のファイルに書かれる */
	int		mb_out;
	/** メンバー名の配列 */
	sym4_t		*mb;
	/** 対象時刻の数 */
	int		n_vt;
	/**
	 * 対象時刻によるデータファイル分割方法
	 * \li\c 0 すべての対象時刻が同一のデータファイルに書かれる
	 * \li\c 1 それぞれの対象時刻が個別のデータファイルに書かれる
	 * \li\c >1 対象時刻 @p vt_out 個分のデータが一つのファイルに書かれる
	 */
	int		vt_out;
	/** 予報時刻1の配列 */
	int		*ft1;
	/** 予報時刻2の配列 */
	int		*ft2;
	/** 対象時刻の単位 */ 
	sym4_t		ftunits;
	/** 面の数 */
	int		n_lv;
	/** 面1 */
	sym8_t		*lv1;
	/** 面2 */
	sym8_t		*lv2;
	/** 要素の数 */
	int		n_el;
	/** 要素名の配列 */
	sym8_t		*el;
	/** メンバー×対象時刻×面×要素 で制約を載せる表 */
	N_UI4		*elementmap;
	/** X(たいてい東西)格子数 */
	int		nx;
	/** Y(たいてい南北)格子数 */
	int		ny;
	/** 投影法諸元 */
	float		projparam[7][2];
	/** 各格子点値が場をいかに代表するか */
	sym4_t		value;
	/** パック・圧縮方式 */
	sym4_t		packing;
	/** 欠損値表現法 */
	sym4_t		missing;
	/** SUBC 記録の情報 */
	struct nusdef_subcinfo_t	*subctab;
	/** INFO 記録の情報 */
	struct nusdef_subcinfo_t	*infotab;
	/** SUBC 記録の個数 */
	int		n_subc;
	/** INFO 記録の個数 */
	int		n_info;
	/** 固定長記録オプション
	 * \li\c 0: デフォルトの可変長記録
	 * \li\c >0: 固定長記録化。数値は記録長
	 * */
	long		forcedrlen;
	/** オプション設定文字列
	 * ヌル初期化: nusdef_init(), OPTIONS 文があれば引数が設定される。
	 */
	char		options[256];
	/** 読みかけのファイルデータへのポインタ、
	 * 兼 解読未完了フラグ。
	 * def_read.c 以外では struct lex_buffer は未定義であり
	 * 内部構造は参照できない(必要もないと思うが)。
	 */
	struct lex_buffer		*lexbuffer;
} nusdef_t;

/* 定義は下にある */
union nusdset_t;
/* 定義は dfile.h にある */
struct ibuffer_t;
struct obuffer_t;
union nusdfile_t;

/* ds_inq_grid の問い合わせパラメタ */
struct inq_grid_info {
	nusdims_t nusdims; /**< 次元 */
	char *proj; /**< 投影法名称の格納先 (長さ4文字列) */
	N_SI4 *gridsize; /**< 格子サイズの格納先 (2要素配列) */
	float *gridinfo; /**< 投影法パラメタの格納先 (14要素配列) */
	char *value; /**< 空間代表性名称の格納先 (長さ4文字列) */
};

/** @brief 各種データセットに共通するメソッドの表。
 */
struct ds_functab {
	/** 所属ファイルを全部閉じる */
	int (*xds_close)(union nusdset_t *ds, int flags);
	/** 全リソースを開放し完全に消滅する */
	int (*xds_delete)(union nusdset_t *ds);
	/** キャッシュなど不要なリソースを全部廃棄して身軽になる */
	int (*xds_compact)(union nusdset_t *ds);
	/** データファイルをみつける関数 */
	union nusdfile_t *(*xds_findfile)(union nusdset_t *ds,
			const nusdims_t *dim, int open_flags);
	/** データファイルをみつけて1記録読む関数 */
	int (*xds_readdata)(union nusdset_t *ds, nusdims_t *dim,
			struct ibuffer_t *buf);
	/** データファイル中のデータ記録に対する問合せ */
	int (*xds_inq_data)(union nusdset_t *ds, nusdims_t *dim,
			int item, void *buf, N_UI4 buf_nelems);
	/** データファイルをみつけて1記録書く関数 */
	int (*xds_writedata)(union nusdset_t *ds, nusdims_t *dim,
			struct obuffer_t *buf);
	/** データファイルをみつけて SUBC/INFO 記録を1つ読む関数 */
	int (*xds_read_aux)(union nusdset_t *ds, const nusdims_t *dim,
		sym4_t rectitle, sym4_t recgroup,
		int (*callback)(const void *rec, N_UI4 siz, void *arg,
			union nusdset_t *dset, N_SI4 ofs_flg),
		void *arg);
	/** データセットの定義ファイルに指定の補助管理部が許容されるか.
	 *
	 * @param grp 補助管理部グループ名
	 * @param nbytes 補助管理部のバイト数 (共通部除く"定義ファイルの長さ")
	 * @retval 0 問題なし
	 * @retval 他 当該グループ名は許されていない (nus_err 送出済み)
	 * @retval 他 バイト数が異なる (nus_err 送出済み)
	 * */
	int (*xds_subc_namecheck)(union nusdset_t *ds, sym4_t grp,
			size_t nbytes);
	/** データセットに属する基準時刻リストを取得
	 * @param data 結果格納配列
	 * @param data_nelems 結果格納配列要素数
	 * @param verbose 経過表示フラグ (非零のとき経過を表示)
	 * @retval 負 エラー
	 * @retval 0 データセットはあるが探索対象がみつからない
	 * @retval 正 結果配列に格納すべき情報の個数
	 *    (data_nelems を超える場合、data_nelems 以上は書き込まれない)
	 */
	int (*xds_btlist)(union nusdset_t *ds,
			N_SI4 data[], N_SI4 data_nelems, int verbose);
	/** データセットに属する対象時刻リストを取得
	 * @param data 結果格納配列
	 * @param data_nelems 結果格納配列要素数
	 * @param basetime 指定基準時刻、-1 ならば限定なし
	 * @param verbose 経過表示フラグ (非零のとき経過を表示)
	 * @retval 負 エラー
	 * @retval 0 データセットはあるが探索対象がみつからない
	 * @retval 正 結果配列に格納すべき情報の個数
	 *    (data_nelems を超える場合、data_nelems 以上は書き込まれない)
	 */
	int (*xds_vtlist)(union nusdset_t *ds,
			N_SI4 data[], N_SI4 data_nelems,
			N_SI4 basetime, int verbose);
	/** SUBC/INFO レコードに関する問合せ
	 *
	 * @param ds データセット
	 * @param dims ファイル特定のための次元
	 * @param query 問合せコード
	 * @param group レコードの群名
	 * @param buf 結果格納配列
	 * @param arg いろいろ (問合せコードによる)
	 * @param keep_open 非零にするとファイルを閉じない
	 * @retval 正 結果配列に格納された要素数
	 * @retval 負 エラー
	 */
	int (*xds_inq_aux)(union nusdset_t *ds, const nusdims_t *dims,
			int query, sym4_t group, void *buf, N_UI4 bufnelems);
	/** データファイルに関する問い合わせ
	 */
	int (*xds_inq_cntl)(union nusdset_t *ds, const nusdims_t *dims,
			int query, void *buf, void *arg,
			int keep_open);
	/** データファイルをみつけて SUBC/INFO 記録を1つ書く */
	int (*xds_write_aux)(union nusdset_t *ds, const nusdims_t *dim,
		sym4_t rectitle, sym4_t recgroup,
		size_t nbytes,
		int (*callback)(void *rec, N_UI4 siz, void *arg,
			union nusdset_t *dset),
		void *arg);
	/** ファイルを1つだけ閉じる関数 */
	int (*xds_close_file)(union nusdset_t *ds, const nusdims_t *dim);
	/** 定義ファイルへの問合せ */
	int (*xds_inq_def)(union nusdset_t *ds, N_SI4 param,
		void *data, N_SI4 datasize);
	/** データファイルに関する問い合わせ (GRID 限定)
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
/** 具体例: dds_close() */
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
/** 具体例: dds_read_aux() */
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
/** 具体例: dds_inq_aux() */
#define ds_inq_aux(ds, dims, query, group, buf, bufnelems) \
	((ds)->comm.methods.xds_inq_aux((ds), (dims), (query), (group), \
					(buf), (bufnelems)))
/** 具体例: dds_inq_cntl() */
#define ds_inq_cntl(ds, dims, query, buf, arg, keep_open) \
	((ds)->comm.methods.xds_inq_cntl((ds), (dims), (query), \
					(buf), (arg), (keep_open)))
/** 具体例: dds_write_aux() */
#define ds_write_aux(ds, dim, ttl, grp, nbytes, callback, arg) \
	((ds)->comm.methods.xds_write_aux((ds), (dim), (ttl), (grp), \
					 (nbytes), (callback), (arg)))
/** 具体例: dds_inq_def() */
#define ds_inq_def(ds, param, data, datasize) \
	((ds)->comm.methods.xds_inq_def((ds), (param), (data), (datasize)))
/** 具体例: dds_inq_grid() */
#define ds_inq_grid(ds, info) \
	((ds)->comm.methods.xds_inq_grid((ds), (info)))

/** @brief SUBC 記録の初期値
 */
struct ds_preset_t {
	/** グループ名 */
	sym4_t		sp_grp;
	/** バイト数 */
	unsigned	sp_nbytes;
	/** 記録内容 */
	N_UI1		*sp_contents;
};

/** @brief 各種データセットが共有する情報.
 */
struct ds_common_t {
	nustype_t		nustype;
	int			nrd;
	struct ds_functab	methods;
	/** SUBC ETA 記録のデフォルト値
	 * dds_init で NULL 初期化, nusxds_subcpreset で設定 */
	struct ds_preset_t	*sc_eta;
	/** SUBC SIGM 記録のデフォルト値
	 * dds_init で NULL 初期化, nusxds_subcpreset で設定 */
	struct ds_preset_t	*sc_sigm;
	/** SUBC ZHYB 記録のデフォルト値
	 * dds_init で NULL 初期化, nusxds_subcpreset で設定 */
	struct ds_preset_t	*sc_zhyb;
	/** SUBC RGAU 記録のデフォルト値
	 * dds_init で NULL 初期化, nusxds_subcpreset で設定 */
	struct ds_preset_t	*sc_rgau;
	/** SUBC DELT 記録のデフォルト値
	 * dds_init で NULL 初期化, nusxds_subcpreset で設定 */
	struct ds_preset_t	*sc_delt;
	/** 動的設定パラメタ
	 * dds_init で初期化, 設定APIを用意予定 */
	struct nusxds_param_t	param;
	/** データ無効フラグ 
	 * dds_init で 0 に初期化 */
	int dead_flag;
};


/** @brief 定義ファイル型データセットのファイル名展開テンプレート.
 */
struct dds_template {
	/** @brief fullpath の長さ (終端ヌルを数えない) */
	size_t	pathlen;
	/** @brief ファイル名フルパス
	 *
	 * ただし基準時刻・メンバー・対象時刻は伏字.
	 * 割付: nusdds_build_template(), 開放: 未定 */
	char	*fullpath;
	/** @brief 相対パスオフセット
	 *
	 * fullpath + rel が NRD からの相対パスになる */
	int	rel;
	/** @brief 基準時刻埋め込み位置
	 *
	 * 基準時刻は fullpath + b から埋め込まれる。さもなくば -1 */
	int	b;
	/** @brief メンバー名埋め込み位置
	 *
	 * メンバーは fullpath + m から埋め込まれる。さもなくば -1 */
	int	m;
	/** @brief 対象時刻埋め込み位置
	 *
	 * 対象時刻は fullpath + v から埋め込まれる。さもなくば -1 */
	int	v;
};

/** 定義は dds_dftab.c */
struct dds_dftab_t;

/** @brief 定義ファイル型データセット
 * @todo tar 型データセットはどうしよう?
 */
struct dds_t {
	/** @brief データセット共通情報
	 *
	 * (先頭要素でなければならない) */
	struct ds_common_t	comm;
	/** @brief 定義ファイル情報
	 *
	 * 値は dds_open() 内 nusdef_init()/nusdef_readfile() および
	 * 各所から呼ばれる nusdef_endparse() で設定される.
	 * */
	nusdef_t		def;
	/** @brief NRD ディレクトリ名.
	 *
	 * 割付: dds_init(), 破棄: dds_delete()(仮称).
	 * */
	const char		*dirname;
	/** @brief ファイル名展開テンプレート
	 */
	struct dds_template	tmpl;
	/** @brief ファイルキャッシュ
	 *
	 * 割付: dds_init(), 破棄: dds_delete()(仮称).
	 */
	struct dds_dftab_t	*dftab;
};

/** 定義は ndf_codec.h */
struct ndf_codec_t;

/** @brief パンドラデータセット
 */
struct pds_t {
	struct ds_common_t	comm;
	char			*server;
	unsigned		port;
	char			*path;
	struct ndf_codec_t	*codec;
};

/** @brief 汎用データセット
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
