/** @file
 * @brief 各種データファイルに関する型宣言・関数原型宣言
 */

/** @brief データ配列読み込み時の切出し指定.
 *
 * cr_xofs が負の場合切出しは無効つまり全面読みだしを表わす
 * */
struct cut_rectangle {
	int	cr_xofs;	/**< X 方向開始位置ずれ (0始まり添字と等価) */
	int	cr_xnelems;	/**< X 方向要素数 */
	int	cr_yofs;	/**< Y 方向開始位置ずれ (0始まり添字と等価) */
	int	cr_ynelems;	/**< Y 方向要素数 */
};

#define cut_rectangle_disable(p) \
	((p)->cr_xofs = (p)->cr_xnelems = (p)->cr_yofs = (p)->cr_ynelems = -1)
#define cut_rectangle_disabled(p) \
	((p)->cr_xofs < 0)
#define cut_rectangle_size(p) \
	((p)->cr_xnelems * (p)->cr_ynelems)

/** @brief 利用者から渡される read バッファ */
struct ibuffer_t {
	void		*ib_ptr;	/**< ポインタ */
	sym4_t		ib_fmt;		/**< データ型 */
	N_UI4		nelems;		/**< 要素数 @todo N_NC の場合の意味 */
	struct cut_rectangle	ib_cut;	/**< 切出し指定 */
};

/** @brief 利用者から渡される write バッファ */
struct obuffer_t {
	const void	*ob_ptr;	/**< ポインタ */
	sym4_t		ob_fmt;		/**< データ型 */
	N_UI4		nelems;		/**< 要素数 @todo N_NC の場合の意味 */
	const N_UI1	*ob_mask;	/**< ビットマスク. ds_write が設定 */
};

/* 定義は下 */
union nusdfile_t;
/* 定義は dset.h */
union nusdset_t;

/** @brief 各種データファイルに共通するメソッドの表 */
struct df_functab {
	/** @brief 閉じる関数 (再オープン可能) */
	int (*xdf_close)(union nusdfile_t *df);
	/** @brief ndf 配下の全資源を開放し完全に消滅させる */
	int (*xdf_delete)(union nusdfile_t *df);
	/** @brief 記録読みだし関数 */
	int (*xdf_read)(union nusdfile_t *df, nusdims_t *dim,
			struct ibuffer_t *buf);
	/** @brief 記録書き込み関数 */
	int (*xdf_write)(union nusdfile_t *df, nusdims_t *dim,
			struct obuffer_t *buf);
	/** @brief 一度閉じたファイルを開き直す関数 */
	int (*xdf_reopen)(union nusdfile_t *df, int open_flags);
	/** @brief SUBC/INFO 記録読みだし関数 */
	int (*xdf_read_aux)(union nusdfile_t *df, sym4_t ttl, sym4_t grp,
			int (*callback)(const void *rec, N_UI4 siz,
				void *arg, union nusdset_t *ds, N_SI4 ofs_flg),
			void *arg, union nusdset_t *ds);
	/** @brief SUBC/INFO 記録書き込み関数 */
	int (*xdf_write_aux)(union nusdfile_t *df, sym4_t ttl, sym4_t grp,
			size_t nbytes,
			int (*encoder)(void *rec, N_UI4 siz,
				void *arg, union nusdset_t *ds),
			void *arg, union nusdset_t *ds);
	/** @brief CNTL 記録問合せ関数. */
	int (*xdf_inq_cntl)(union nusdfile_t *df, sym4_t query, void *dest,
			void *arg);
	/** @brief 格子情報書き込み関数 */
	int (*xdf_write_grid)(union nusdfile_t *df,
			const char proj[4], const N_SI4 size[2],
			const float projparam[14], const char value[4]);
	/** @brief DATA 記録問合せ */
	int (*xdf_inq_data)(union nusdfile_t *df, nusdims_t *dim,
			int item, void *buf, N_UI4 bufnelems);
	/** @brief 書き込みを完結させる関数 */
	int (*xdf_flush)(union nusdfile_t *df);
	/** @brief SUBC/INFO 記録問合せ関数 */
	int (*xdf_inq_aux)(union nusdfile_t *df, int query,
			sym4_t group, void *buf, N_UI4 bufnelems);
};

/** @brief データファイルを閉じる. ndf_close() 等
 * @param df 閉じようとするデータファイル (nusdfile_t *)
 * @note ファイルハンドル以外の資源 (特にメモリ)
 * は開放されないので df_close() したあとポインタを捨てないこと。
 * 本当に削除するには df_delete() を使う。
 */
#define df_close(df) \
	((df)->comm.methods.xdf_close((df)))

#define df_delete(df) \
	((df)->comm.methods.xdf_delete((df)))

/** @brief データファイルから1記録を読む. ndf_read() 等
 * @param df 読もうとするデータファイル (nusdfile_t *)
 * @param dim 記録の指定 (nusdims_t *)
 * @param buf 入力バッファ (ibuffer_t *)
 */
#define df_read(df, dim, buf) \
	((df)->comm.methods.xdf_read((df), (dim), (buf)))

/** @brief データファイルに1記録を書く. ndf_write() 等
 * @param df 書こうとするデータファイル (nusdfile_t *)
 * @param dim 記録の指定 (nusdims_t *)
 * @param buf 出力バッファ (obuffer_t *)
 */
#define df_write(df, dim, buf) \
	((df)->comm.methods.xdf_write((df), (dim), (buf)))

/** @brief 閉じたデータファイルを開き直す. ndf_reopen() 等
 * @param df 一度閉じたデータファイル
 * @param open_flags フラグ
 */
#define df_reopen(df, open_flags) \
	((df)->comm.methods.xdf_reopen((df), (open_flags)))

/** @brief データファイルからSUBC/INFO記録を読む. nusndf_read_aux() 等
 * @param df データファイル (nusdfile_t *)
 * @param ttl 記録種別 (sym4_t; SYM4_SUBC または SYM4_INFO)
 * @param grp グループ名 (sym4_t; SYM4_ETA など)
 * @param decoder 読んだ記録を処理する関数
 *                 (int (*)(const void *rec, N_UI4 siz, void *arg,
 *                 union nusdset_t *ds))
 *                 渡される rec, siz は各種記録共通部や grp を除いたもので
 *                 あり、SUBC 記録については定義ファイルの subcntl 文
 *                 に書く長さに相当する。
 * @param arg デコーダに渡す引数 (void *)
 * @param ds デコーダに渡すデータセット (union nusdset_t *)
 */
#define df_read_aux(df, ttl, grp, decoder, arg, ds) \
	((df)->comm.methods.xdf_read_aux((df), (ttl), (grp), (decoder), \
		(arg), (ds)))

/** @brief データファイルに SUBC/INFO 記録を書き込む. nusndf_write_aux() 等
 * @param df データファイル (nusdfile_t *)
 * @param ttl 記録種別 (sym4_t; SYM4_SUBC または SYM4_INFO)
 * @param grp グループ名 (sym4_t; SYM4_ETA など)
 * @param size 記録のバイト数
 * @param encoder 記録を作成する関数
 *                 (int (*)(void *rec, N_UI4 siz, void *arg,
 *                 union nusdset_t *ds))
 *                 渡される rec, siz は各種記録共通部や grp を除いたもので
 *                 あり、SUBC 記録については定義ファイルの subcntl 文
 *                 に書く長さに相当する。
 * @param arg エンコーダに渡す引数 (void *)
 * @param ds エンコーダに渡すデータセット (union nusdset_t *)
 */
#define df_write_aux(df, ttl, grp, size, encoder, arg, ds) \
	((df)->comm.methods.xdf_write_aux((df), (ttl), (grp), (size), \
					  (encoder), (arg), (ds)))

/** @brief CNTL 記録問合せ関数
 *
 * 問合せ種類 query がスカラの場合
 * dest はデータ書き込み先ポインタ,
 * 配列のばあい各要素について呼び出される関数へのポインタ。
 * arg は必要ないばあい NULL をいれる。
 * */
#define df_inq_cntl(df, query, dest, arg) \
	((df)->comm.methods.xdf_inq_cntl((df), (query), (dest), (arg)))

/** @brief CNTL 記録上書き関数
 *
 * 地図投影パラメタを上書きする。
 */
#define df_write_grid(df, proj, size, projparam, value) \
	((df)->comm.methods.xdf_write_grid((df), \
				  (proj), (size), (projparam), (value)))

/** @brief DATA 記録問合せ関数
 */
#define df_inq_data(df, dims, item, buf, bufnelems) \
	((df)->comm.methods.xdf_inq_data((df), (dims), \
					 (item), (buf), (bufnelems)))

/** @brief データファイルの未完了書き込みを終える. ndf_flush() 等
 * @param df 閉じようとするデータファイル (nusdfile_t *)
 */
#define df_flush(df) \
	((df)->comm.methods.xdf_flush((df)))

/** @brief データファイルの SUBC/INFO レコードに関する問合せ
 *
 * 例: nusndf_inq_aux
 */
#define df_inq_aux(df, query, group, buf, bufnelems) \
	((df)->comm.methods.xdf_inq_aux((df), (query), (group), \
					(buf), (bufnelems)))

/** @brief 各種データセットが共有する情報 */
struct df_common_t {
	/** @brief 関数テーブル */
	struct df_functab methods;
	/** @brief 書き込み可フラグ */
	int	flags;
	/** @brief ファイル名.
	 *
	 * 割付: ndfopen_initfields() */
	const char *filename;
	/** @brief 種別名
	 */
	nustype_t nustype;
	/** @brief 開かれているとき真 */
	int	is_open;
	/** @brief データセットの設定
	 * @warning これあるが故にデータファイルを残してデータセットが
	 * 消えることは許されない.
	 * */
	struct nusxds_param_t *ds_param;
};

#define df_is_closed(df)	((df)->comm.is_open == 0)
#define df_is_writable(df)	((df)->comm.flags & IO_WRITABLE)
#define df_param(df, name)	DynamicParam((df)->comm.ds_param, name)

/** @brief 記録順番探査用情報
 *
 * おおざっぱにいって stdio の FILE 構造体に相当する。
 */
struct nsf_t {
	/** @brief ファイル */
	union nusio_t *sf_io;
	/** @brief 記録長加算量
	 *
	 * レコード先頭の 4 バイトを読み取ったあと、この値を加算して
	 * レコード長を得る。
	 * 標準的な Fortran シーケンシャルファイルは 8, NuSDaS 1.0 は 0.
	 * */
	N_SI4	sf_reclplus;
	/** @brief 直前に読み込まれた記録の位置
	 *
	 * ファイルポインタとは一致しないので注意
	 * */
	N_SI8	sf_pos;
	/** @brief 現在読み込まれている入力バッファの起点
	 *
	 * 初期値 -1 (nusnsf_ini() で設定)
	 * */
	N_SI8	sf_base;
	/** @brief 直前の入力バッファ
	 *
	 * 初期値 -1 (nusnsf_ini() で設定)
	 */
	N_SI8	sf_base_prev;
	/** @brief 入力バッファのサイズ */
	N_SI8	sf_size;
	/** @brief 入力バッファのポインタ */
	N_UI4	*sf_rec;
	/** 現在探査中の記録の長さ (端から端まで) */
	N_UI4	sf_recl;
	/** 読み取りバッファ長を得るためにシフトする数 */
	N_UI4	sf_rbuffer;
	/** 整端するか否か */
	N_UI4	sf_align;
	/** 整端用バッファ */
	N_UI4	*sf_alibuf;
	/** 整端用バッファの長さ */
	N_UI4	sf_alibuflen;
};

/** @brief 伝統的 NuSDaS データファイル */
struct ndf_t {
	/** @brief データセット共通情報 */
	struct df_common_t	comm;
	/** @brief 低水準ファイル
	 * 割付: ndf_open(), ヌル化: ndf_close().
	 */
	struct nsf_t		seqf;
	/** @brief ファイルバージョン */
	N_UI4			filever;
#define ndf_is_largefile(ndfp)  (ndfp->filever >= 13)
	/** @brief 基準時刻の数 といっても今は必ず1
	 *
	 * 設定: ndfopen_initfields(), ndfopen_loadfile() が上書きするかも
	 */
	N_UI4			nb;
	/** @brief メンバーの数
	 *
	 * 設定: ndfopen_initfields(), ndfopen_loadfile() が上書きするかも
	 */
	N_UI4			nm;
	/** @brief メンバーの位置
	 *
	 * member out の場合定義ファイルの位置
	 */
	int				om;
	/** @brief 対象時刻の数
	 *
	 * 設定: ndfopen_initfields(), ndfopen_loadfile() が上書きするかも
	 */
	N_UI4			nv;
	/** @brief 対象時刻の位置
	 *
	 * validtime out の場合定義ファイルの位置
	 */
	int				ov;
	/** @brief 面の数
	 *
	 * 設定: ndfopen_initfields(), ndfopen_loadfile() が上書きするかも
	 */
	N_UI4			nz;
	/** @brief 要素の数
	 *
	 * 設定: ndfopen_initfields(), ndfopen_loadfile() が上書きするかも
	 */
	N_UI4			ne;
	/** @brief 基準時刻のリスト といっても今は必ず1要素
	 *
	 * 割付: ndfopen_loadfile(), 失敗したら ndf_creat().
	 */
	struct array4_t		*btime;
	/** メンバーのリスト
	 *
	 * 割付: ndfopen_loadfile(), 失敗したら ndf_creat().
	 */
	struct array4_t		*member;
	/** 対象時刻のリスト (vt1, vt2 をパック)
	 *
	 * 割付: ndfopen_loadfile(), 失敗したら ndf_creat().
	 */
	struct array8_t		*vtime;
	/** 面のリスト
	 *
	 * 割付: ndfopen_loadfile(), 失敗したら ndf_creat().
	 * @note plane2 が欠けている */
	struct array8_t		*plane;
	/** 要素のリスト */
	struct array8_t		*element;
	/** indx に割り当てたバイト数 */
	size_t			indxsize;
	/** INDX 記録全体 @note nus_free するときに使う
	 *
	 * 割付: ndfopen_loadfile(), 失敗したら ndf_creat().
	 */
	unsigned char		*indx;
	/** ファイルバージョン11までの INDX テーブル
	 *
	 * メモリは indx と一体的に確保。
	 * ファイルバージョン13では INDX が8バイト単位となるため
	 * この要素はヌルとなる。
	 *
	 * 割付: ndfopen_loadfile(), 失敗したら ndf_creat().
	 */
	N_UI4			*indx4;
	/** ファイルバージョン 13 以降の記録長テーブル */
	N_UI4			*indy_recl;
	/** ファイルバージョン 13 以降の要素数テーブル */
	N_UI4			*indy_nelems;
	/** x 方向の格子数 */
	N_UI4			nx;
	/** y 方向の格子数 */
	N_UI4			ny;
	/** コーデックのキャッシュ (直前に使ったコーデック) */
	struct ndf_codec_t	*codec;
	/** nusdcntl に割り当てたバイト数 */
	size_t			nusdsize;
	size_t			cntlsize;
	/** NUSD, CNTL 記録 (出力時のみ使用) */
	unsigned char		*nusd;
	unsigned char		*cntl;
	/** 次に DATA 記録を書ける位置。
	 * ndfopen_initfields() がゼロに設定、
	 * ndfopen_loadfile() または ndf_creat() が設定。
	 * ndf_write が更新。
	 */
	N_UI8			wpos;
	/** ファイルの全サイズ。
	 * 書き込み処理最後のndfclose_writeheadが設定する。
	 * 読み込み時は設定されない。
	 */
	N_UI8			totalsize;
	/** 総記録数 */
	N_UI4			reccount;
	/** データパック方式 */
	sym4_t			packing;
	/** 欠損表現方式 */
	sym4_t			missing;
	/** INFO 記録数.
	 *
	 * ndfopen_loadfile でロードされるか、
	 * ndf_creat で定義ファイルに初期値ファイルを
	 * 省略せずに書かれている information 文の数が設定される。
	 * (sitab のエントリ数ではなく、実際にファイルに書かれた数)
	 * 随時追加可。
	 * */
	N_UI4			infocount;
	/** SUBC 記録数.
	 *
	 * ndfopen_loadfile でロードされるか、
	 * ndf_creat で定義ファイルにある数が設定される。
	 * その後の追加は許されない。
	 * */
	N_UI4			subccount;
	/** CNTL 記録の時刻 */
	N_UI4			cntltime;
	/** 出力バッファ: ndfopen_initfields() が GlobalConfig(pc_wbuffer)
	 * に従い割り付けまたは NULL 化. NULL ならバッファリングしない. */
	struct ndf_wbuf_t {
		/** ndf_open() 時の GlobalConfig(pc_wbuffer) の値となる */
		N_UI4		siz;
		N_UI4		pos;
		N_SI8		ofs;
		unsigned char	*ptr;
	}			*wbuf;
	/** SUBC 記録の表.
	 *
	 * SUBC 記録の数はファイル作成時に決定しているので、
	 * subctab が指す ndf_aux_t 型実体は ndf_creat() または
	 * ndf_auxtab_load() で一括してアロケートする。
	 */
	struct ndf_auxtab_t *subctab;
	/** INFO 記録の表.
	 *
	 * INFO 記録の数は決まっていないので、ndf_aux_t 型実体は
	 * なんらかの形でデータ内容が ndf モジュールに届いたときに
	 * 個別にアロケートされる。
	 */
	struct ndf_auxtab_t *infotab;
	/** 固定長記録にする場合非零となる
	 */
	long forcedrlen;
};

/** @brief 汎用データファイル
 *
 * 実体は今のところ ndf_t
 */
typedef union nusdfile_t {
	struct df_common_t	comm;
	struct ndf_t		ndf;
} nusdfile_t;

/** 定義は dset.h */
struct dds_t;

/* 定義は ndf_open.c */
extern nusdfile_t *
ndf_open(union nusio_t *io, const char *filename,
		int flags, struct dds_t *dds, const nusdims_t *dim);

/* 定義は dfile.c */
extern nusdfile_t *
df_open(const char *filename, int flags, struct dds_t *dds, const nusdims_t *dim);
