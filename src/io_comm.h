/** @file
 * @brief 各種入出力モジュールの共通ヘッダ
 */

#ifndef NUSDAS_CONFIG_H
# error please include "config.h"
#endif

/* 定義は下 */
union nusio_t;

/** @brief 各種入出力モジュールに共通するメソッドの表 */
struct io_functab {
	/** @brief 閉じる @see io_close() */
	int (*xio_close)(union nusio_t *io, N_SI8 totalsize, const char* filename);
	/** @brief 読込予約 @see io_load() */
	int (*xio_load)(union nusio_t *io, N_SI8 ofs, N_SI8 size);
	/** @brief 読込結果の参照 @see io_peek() */
	void *(*xio_peek)(union nusio_t *io, N_SI8 ofs, N_SI8 size);
	/** @brief 即時読込 */
	N_SI8 (*xio_read)(union nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf);
	/** @brief 即時書込 */
	int (*xio_write)(union nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf);
	/** @brief 書込予約 @see io_getwbuf() */
	void *(*xio_getwbuf)(union nusio_t *io, N_SI8 ofs, N_SI8 size);
	/** @brief 書込開始 @see io_issue() */
	int (*xio_issue)(union nusio_t *io, N_SI8 ofs, N_SI8 size);
};

/** @brief 閉じる
 *
 * ファイルハンドル (nusio_t) @p io の指すファイルを閉じる。
 * このハンドルはこの先使用できない。
 * @retval 0 正常終了
 * @retval -1 エラー
 * @see sio_close(), pio_close(), eio_close()
 */
#define io_close(io,size,filename) \
	((io)->comm.methods.xio_close((io),(size),(filename)))

/** @brief 読込予約
 *
 * ファイル (nusio_t) @p io の
 * 先頭から @p ofs バイト目から @p size バイトを
 * @p io オブジェクト内部に確保したバッファに読み込んでおくよう指示する。
 * データは io_peek() で取り出す。
 * この関数のリターン時に読込が完結している保証はない、つまり、
 * 一部のエラーは io_peek() の異常終了によって通知される。
 * @retval 0 正常終了
 * @retval -1 エラー
 */
#define io_load(io, ofs, size)	\
	((io)->comm.methods.xio_load((io), (ofs), (size)))

/** @brief 読込結果の参照
 *
 * ファイル (nusio_t) @p io の先頭から @p ofs バイト目から
 * @p size バイトの内容が
 * 書き込まれているメモリへのポインタを返す。
 * 範囲は直前の io_load() で指示した範囲内であれば高速である。
 * ここで返されたポインタは、同じ @p io を引数とする io_load(),
 * io_getwbuf(), io_close(), io_write() のいずれかが呼ばれるまでは有効である。
 * @retval NULL エラー
 * @retval その他 正常終了
 * @note この関数の返すポインタを勝手に nus_free() してはならない。
 */
#define io_peek(io, ofs, size)	\
	((io)->comm.methods.xio_peek((io), (ofs), (size)))

/** @brief 即時読込
 *
 * ファイル (nusio_t) @p io の先頭から @p ofs バイト目から @p size バイトを
 * 利用者が用意したメモリブロック @p buf に読み込む。
 * @retval 非負 読み込まれたバイト数
 * @retval (N_SI8)-1 エラー
 */
#define io_read(io, ofs, size, buf)	\
	((io)->comm.methods.xio_read((io), (ofs), (size), (buf)))

/** @brief 即時書き込み
 *
 * ファイル (nusio_t) @p io の
 * 先頭から @p ofs バイト目から
 * @p size バイトを利用者が用意したメモリブロック @p buf から書き込む。
 * @retval 0 正常終了
 * @retval -1 エラー
 *    (write(2) が正常終了したが全部のデータを書き切れていない場合を含む)
 */
#define io_write(io, ofs, size, buf)	\
	((io)->comm.methods.xio_write((io), (ofs), (size), (buf)))

/** @brief 書込予約
 *
 * ファイル (nusio_t) @p io の先頭から @p ofs バイト目から @p size バイトを
 * 書き込むためのバッファを確保し、そのポインタを返す。
 * ファイルイメージをそのメモリに書き込んだ後、
 * io_issue() によってファイルへの書出が開始する。
 * ここで返されたポインタは、同じ @p io を引数とする io_issue(), io_load(),
 * io_getwbuf(), io_close(), io_write() のいずれかが呼ばれるまでは有効である。
 * @retval NULL エラー
 * @retval その他 正常終了
 */
#define io_getwbuf(io, ofs, size) \
	((io)->comm.methods.xio_getwbuf((io), (ofs), (size)))

/** @brief 書込開始
 *
 * ファイル (nusio_t) @p io の先頭から @p ofs バイト目に関連付けて
 * あらかじめ io_getwbuf() で確保したバッファの先頭から @p size バイトを
 * ファイルに書き込み開始する。
 * この関数のリターン時に書き込みが完了している保証はない、すなわち
 * 一部のエラーは後続する io 関数の異常終了によって通知される。
 * したがって、 io_issue() 発行後は
 * io_getwbuf() の返したポインタの指すメモリ領域に書き込んではならない。
 * @retval 0 正常終了
 * @retval その他 エラー
 */
#define io_issue(io, ofs, size) \
	((io)->comm.methods.xio_issue((io), (ofs), (size)))

/** @brief 各種入出力モジュールが共有する情報 */
struct io_common_t {
	/** @brief 関数テーブル */
	struct io_functab methods;
	/** @brief io_open() に渡したフラグ, および
	 * io_open() によって通知される情報 */
	int	flags;
};

/* フラグビット定義 --- io_open() の引数に渡される */
/** 読み込み可能ビット */
#define IO_READABLE	(1<<0)
/** 書き込み可能ビット */
#define IO_WRITABLE	(1<<1)
/** 読み書き両方可のビット組合せ */
#define IO_READWRITE		(IO_READABLE | IO_WRITABLE)

/** ファイルが既存ではないことを示すフラグ. df_open() が設定 */
#define IO_CREATED	(1<<8)
/** Fortran と似て非なるシーケンシャルファイルを示すフラグ.
 * df_open() が設定 */
#define IO_OLDSEQF	(1<<9)
/** エラーメッセージを抑止するフラグ.
 * dds_findfile() が設定 */
#define IO_ERRMSG_OFF	(1<<10)

struct io_buffer_t {
	/** バッファ状態 */
	enum io_buffer_status {
		/** @brief バッファに有意義な情報が入っていない状態
		 *
		 * 無断 nus_free 可、i o_peek() 不可。
		 * 重複する領域に対する io_write() が行われた場合等。
		 */
		IOB_STALE,
		/** @brief バッファに最新のデータが読み込まれている状態
		 *
		 * 無断 nus_free 可、i o_peek() 可。
		 * 具体的には同期型 io_load() または
		 * 非同期型 io_peek() によって入力が完結した状態。
		 */
		IOB_FRESH,
		/** @brief バッファに捨てられない情報があるかもしれない状態
		 *
		 * 無断 nus_free 不可、 io_peek() 不可。
		 * 具体的には io_getwbuf() 後の状態。
		 */
		IOB_MODIFIED,
		/** @brief バッファを用いた非同期入力が行われている状態 
		 *
		 * 無断 nus_free 不可、 io_peek() 不可。
		 * 初回の io_peek() 呼び出しで IOB_FRESH に遷移する。
		 * 具体的には asyncio_load() [廃止] 実行後の状態だが、
		 * いまのところ、この状態にはならない。
		 */
		IOB_READING
	} iob_stat;
	/** バッファ */
	void	*iob_ptr;
	/** 最初のバイトのファイル中でのオフセット */
	N_SI8	iob_head;
	/** 最後の次のバイトのオフセット (つまり iob_head + バッファサイズ) */
	N_SI8	iob_fence;
	/** io_issue または io_load で意味のあるデータが
	 * 入ることと指定された最後の次のバイトオフセット */
	N_SI8	iob_tail;
	/** このバッファがファイル末尾である場合非零 */
	int	iob_eof;
};

#ifdef NEED_IOBUF_NEW
/** @brief 構造体 io_buffer_t の初期化割付
 * @note 一部のフィールドは不定値のままとなる。
 * @retval NULL エラー(メモリ不足のみ)
 * @retval その他 正常終了
 */
	INLINE struct io_buffer_t *
iobuf_new(size_t buflen, N_SI8 head, N_SI8 size)
{
	struct io_buffer_t *iob;
	iob = nus_malloc(buflen + sizeof(*iob));
	if (iob == NULL) {
		return NULL;
	}
	iob->iob_ptr = (char *)(iob + 1);
	iob->iob_head = head;
	iob->iob_fence = si8_add(iob->iob_head, size);
	/* iob->iob_stat は未定義のまま */
	/* iob->iob_tail は未定義のまま */
	/* iob->iob_eof は未定義のまま */
	return iob;
}
#endif

#ifdef NEED_IOBUF_OUTSIDE
/** @brief バッファが要求範囲 [@p ofs, ofs + size) をカバーできるか
 * @retval 0 要求範囲はすべてバッファに載っている。
 * @retval 1 要求範囲はバッファに載っていない。
 *   またはバッファが IOB_FRESH 状態でない。
 *   おそらく再度 sio_load() を呼べば要求範囲が読める。
 * @retval -1 要求範囲はバッファに載っておらずバッファがファイル末尾にある。
 *   つまりこれ以上 sio_load() を呼んでもファイルが延びない限り
 *   要求範囲を読み取ることはできない。
 * @warning 出力用ルーチンでは使わないこと (バッファ状態が違うため)
 */
	INLINE int
iobuf_outside(struct io_buffer_t *iob, N_SI8 ofs, N_SI8 size)
{
	N_SI8	tail;
	if (!iob) return 1;
	if (iob->iob_stat != IOB_FRESH) return 1;
	if (si8_lessthan(ofs, iob->iob_head)) return 1;
	tail = si8_add(ofs, size);
	if (si8_morethan(tail, iob->iob_tail)) {
		return (iob->iob_eof ? -1 : 1);
	}
	return 0;
}
#endif

/** @brief バッファが指定範囲 [ofs, ofs + size) をカバーできるか
 * @warning 出力用ルーチンでは使わないこと [iobuf_outside() を呼ぶマクロのため]
 */
#define iobuf_inside(iob, ofs, size) \
	!iobuf_outside((iob), (ofs), (size))

/** @brief バッファが指定範囲 [ofs, ofs + size) をカバーできるか、
 * またはファイル末尾に到達しているか
 * @warning 出力用ルーチンでは使わないこと [iobuf_outside() を呼ぶマクロのため]
 */
#define iobuf_inside_or_eof(iob, ofs, size) \
	(iobuf_outside((iob), (ofs), (size)) <= 0)

#ifdef NEED_IOBUF_OUTOFFENCE
/** @brief バッファが指定範囲 [@p ofs, @p ofs + @p size) をカバーするか
 *
 * 有効長 iob_tail ではなくメモリ割り付け長 iob_fence を用いる
 * @retval 0 カバーされている
 * @retval 1 はみだしている
 */
	INLINE int
iobuf_outoffence(struct io_buffer_t *iob, N_SI8 ofs, N_SI8 size)
{
	if (!iob) return 1;
	if (si8_lessthan(ofs, iob->iob_head)) return 1;
	if (si8_morethan(si8_add(ofs, size), iob->iob_fence)) return 1;
	return 0;
}
#endif

#ifdef NEED_IOBUF_OVERLAP_ANY
/** @brief バッファが指定範囲 [@p ofs, @p ofs + @p size) に重なるか
 * @retval 0 重ならない
 * @retval 1 重なる
 */
	INLINE int
iobuf_overlap_any(struct io_buffer_t *iob, N_SI8 ofs, N_SI8 size)
{
	if (!iob) return 0;
	if (si8_moreeq(ofs, iob->iob_tail)) return 0;
	if (si8_lesseq(si8_add(ofs, size), iob->iob_head)) return 0;
	return 1;
}
#endif

/** @brief POSIX (システムコール) ファイル入出力 */
struct ioposix_t {
	/** @brief IO モジュール共通部 (必ず先頭に置くこと) */
	struct io_common_t	comm;
	/** @brief ファイル記述子 */
	int			fd;
	/** @brief 現在位置 */
	off_t			pos;
	/** @brief 入出力バッファ
	 *
	 * 初期値はヌル。
	 * GlobalConfig(pc_wbuffer) が非零のとき開かれたファイルについて
	 * Set_Buffer_anyway() が割り付け、 Free_Buffer() が開放
	 */
	struct io_buffer_t	*iob;
};

/** @brief stdio ライブラリ使用 */
struct iostd_t {
	/** @brief IO モジュール共通部 (必ず先頭に置くこと) */
	struct io_common_t	comm;
	/** @brief ファイル
	 *
	 * 本当は FILE * だが stdio.h 依存性を伝播させないため型を落とした */
	void			*fp;
	/** @brief 現在位置 */
	long			pos;
	/** @brief 入出力バッファ
	 *
	 * 初期値はヌル。
	 * GlobalConfig(pc_wbuffer) が非零のとき開かれたファイルについて
	 * Set_Buffer_anyway() が割り付け、 Free_Buffer() が開放
	 */
	struct io_buffer_t	*iob;
	/** @brief 直前に実行したのが fread(3) か fwrite(3) か
	 *
	 * 読書間切替えでは位置が変わらなくても fseek(3) を
	 * 発行する必要があるため。 */
	enum sio_status_t {
		SIO_INITIAL,
		SIO_READING,
		SIO_WRITING
	}			sio_status;
};

#ifdef USE_CSES

# ifndef HAVE_OFFSET_T
typedef off_t offset_t;
# endif

/** @brief ES ファイル入出力 */
struct ioesf_t {
	/** @brief IO モジュール共通部 (必ず先頭に置くこと) */
	struct io_common_t	comm;
	/** @brief ファイル記述子 */
	int			fd;
	/** @brief 現在位置 */
	offset_t		pos;
	/** @brief 書換後のファイル名 */
	char			fnam[256];
	/** @brief 入出力バッファ
	 *
	 * 初期値はヌル。
	 * GlobalConfig(pc_wbuffer) が非零のとき開かれたファイルについて
	 * Set_Buffer_anyway() が割り付け、 Free_Buffer() が開放
	 */
	struct io_buffer_t	*iob;
};
#endif
/* USE_CSES */

/** @brief 汎用データファイル */
typedef union nusio_t {
	struct io_common_t	comm;
	struct ioposix_t	pio;
	struct iostd_t		sio;
#ifdef USE_CSES
	struct ioesf_t		eio;
#endif
} nusio_t;

extern nusio_t * sio_open(const char *filename, int flags);
extern nusio_t * pio_open(const char *filename, int flags);
extern nusio_t * eio_open(const char *filename, int flags);
