/** @file
 * @brief エラー管理
 */

#ifndef NULL
# error please include either stddef.h or stdio.h for size_t
#endif

#ifdef NEED_VSNPRINTF
extern int
nus_vsnprintf(char *buf, size_t bufsize, const char *format, va_list ap);
#endif

/* sys_err.c */
extern int nuserr_flush(void);
#define NUSDAS_CLEANUP	nuserr_flush()

extern const char *nus_dbg_file;
extern int nus_dbg_line;
extern int nus_dbg_enabled;
extern int nuserr_dbg(const char *fmt, ...);
extern int nus_wrn_enabled;
extern int nuserr_wrn(const char *fmt, ...);
extern int nus_errno;
extern int nuserr_err(int errkey, const char *fmt, ...);
extern int nuserr_flush(void);
#define MARK_FOR_DSET	0
#define MARK_FOR_INFO	1
extern int nuserr_mark(int index);
extern int nuserr_cancel(int index);
extern int nuserr_final(void);

/** @brief デバグ出力
 *
 * nusdas_iocntl(N_IO_WARNING_OUT, 1以下) のとき抑止される。
 */
#ifdef USE_NUS_DEBUG
# define nus_debug(args) \
		(nus_dbg_enabled ? \
		(nus_dbg_file = __FILE__, nus_dbg_line = __LINE__, \
		nuserr_dbg args) : 0)
#else
# define nus_debug(args)	 
#endif

/** @brief 警告出力
 *
 * nusdas_iocntl(N_IO_WARNING_OUT, 0) のとき抑止される。
 */
# define nus_warn(args) \
		(nus_wrn_enabled ? \
		(nus_dbg_file = __FILE__, nus_dbg_line = __LINE__, \
		nuserr_wrn args) : 0)

/** @brief エラーメッセージ
 */
# define nus_err(args) \
		(nus_dbg_file = __FILE__, nus_dbg_line = __LINE__, \
		 nuserr_err args)

/** @brief API 用エラーコード
 */
#define NUS_ERRNO (nus_errno)
#define NUS_ERR_CODE() ((signed char)nus_errno)

#ifdef USE_NUS_DEBUG
# define SETERR(errkey) \
	(nus_debug(("SETERR(%s)", #errkey)), nus_errno = (errkey))
#else
# define SETERR(errkey) (nus_errno = (errkey))
#endif

/* 初期化するときの番号 */
#define NUSERR_INITVALUE       -127  /* API はこのエラーを返してはならない */

/* まだ割り当てられていない番号 */
#define NUSERR_NotImplemented   -88
#define NUSERR_Internal         -88
#define NUSERR_OpenRFailed	-87 /* API はこのエラーを返してはならない */
/* nusdas_onefile_close の返却値 */
#define NUSERR_OC_NoOpenDfile	1
/* 共通部 */
/* malloc に失敗 */
#define NUSERR_MemShort		-10
/* type1 の省略指定の補間に失敗 */
#define NUSERR_BadType1		-13
/* データファイルのバージョン番号が不正 */
#define NUSERR_BadVersion       -18
/* 指定されたTYPEのデータセットが見つからなかった */
#define NUSERR_DsetNotFound	-21
#define NUSERR_DfileOpenFail	-21
#define NUSERR_DefOrder		-33
#define NUSERR_BadElementmap	-34
/* 定義ファイル解読中予期せぬ EOF があった */
#define NUSERR_EOFinDef		-35
#define	NUSERR_Deffile		-36
#define NUSERR_DefMissingType1	-40
#define NUSERR_DefMissingType2	-41
#define NUSERR_DefMissingType3	-42
#define NUSERR_DefMissingNV	-43
#define NUSERR_DefMissingVL	-44
#define NUSERR_DefMissingNZ	-45
#define NUSERR_DefMissingZL	-46
#define NUSERR_DefMissingNE	-47
#define NUSERR_DefMissingEL	-48
#define NUSERR_DefMissingSize	-49
#define NUSERR_BadVtime		-50
#define NUSERR_DeffileHasNoMemberlist	-50
#define NUSERR_NoDfileToRead	-51
#define NUSERR_CreatFailed	-53
/* NUSD の読み込みに失敗 */
#define NUSERR_NUSDReadFail	-54
/* CNTL の読み込みに失敗 */
#define NUSERR_CNTLReadFail	-55
/* INDX の読み込みに失敗 */
#define NUSERR_INDXReadFail	-56
/* END の読み込みに失敗 */
#define NUSERR_ENDReadFail	-57
/* DATA 読み込みに失敗 */
#define NUSERR_DATAReadFail	-59

#define	NUSERR_WildcardBTFail	-58

/* 強制レコード長よりレコード長が長い                            
   (NuSDaS1.3からは初期化時に限定しない)
*/
#define NUSERR_ForcedRlen	-63
/* info file の読み込みに失敗 */
#define NUSERR_NoInfoFile	-64
/* close 時に NUSD の書き込みに失敗 */
#define NUSERR_NUSDWriteFail    -65
/* close 時に CNTL or INDX の書き込みに失敗 */
#define NUSERR_CNTLWriteFail    -66
#define NUSERR_INDXWriteFail    -66
/* close 時に END の書き込みに失敗 */
#define NUSERR_ENDWriteFail     -67

#define	NUSERR_DfileUnwritable	-68 /* source: doc; not tested 11 */
#define NUSERR_Over4GiB		-80
#define NUSERR_BadGrid		-81 /* new in 1.3 */
#define NUSERR_BuildTemplFail   -82 /* new in 1.3 */
#define NUSERR_ReadOnly		-69 /* source: doc; not tested 11 */
#define NUSERR_NoZLib		-98 /* new in 1.3 */
#define NUSERR_IO		-99 /* source: doc; not tested 11 */
/* nusdas_read のエラー */
#define NUSERR_RD_(val)		((-1 ^ 0xFF) | (0x100 + val))
#define	NUSERR_RD_NoCodec	NUSERR_RD_(-5)
#define	NUSERR_RD_BadPackMiss	NUSERR_RD_(-5)
#define	NUSERR_RD_SmallBuf	NUSERR_RD_(-4)
#define NUSERR_RD_BadElem	NUSERR_RD_(-2)
#define NUSERR_RD_BadPlane	NUSERR_RD_(-2)
#define NUSERR_RD_BadMember	NUSERR_RD_(-2)
#define NUSERR_RD_OpenFail	NUSERR_RD_(-2)
#define NUSERR_RD_IndexKills	NUSERR_RD_(-2)
#define NUSERR_RD_NoDrec	NUSERR_RD_(0)
#define NUSERR_RD_BadCutRegion  NUSERR_RD_(-8)
/* nusdas_write のエラー */
#define NUSERR_WR_BadMember	-2
#define NUSERR_WR_BadElem	-2
#define NUSERR_WR_BadPlane	-2
#define NUSERR_WR_Elementmap	-2
#define NUSERR_WR_SmallBuf	-3
#define	NUSERR_WR_NoCodec	-4
#define NUSERR_WR_ForcedRlen	-63
#define	NUSERR_WR_BadPackMiss	-6
#define NUSERR_WR_MaskMissing	-7
#define NUSERR_WR_Inconsistent	-83
/* データの encode に失敗した */
#define NUSERR_WR_EncodeFail    -8
/* nusdas_inq_XXX のエラー */
#define NUSERR_IQ_ShortBuf	-1
#define NUSERR_IQ_DestNull	-2
#define NUSERR_IQ_BadParam	-3
/* nusdas_subc_XXXX のエラー */
#define NUSERR_SC_BadGroup	-4
#define NUSERR_SC_BadArg	-6
#define NUSERR_SC_BadInput	-4
#define NUSERR_SC_ShortBuf	-3
#define NUSERR_SC_SizeMismatch	-3
#define NUSERR_SC_NoRecToWrite  -2
#define NUSERR_SC_Uninitialized	-2
#define NUSERR_SC_PeekFailed	-2
/* nusdas_info のエラー (subc と違うもの) */
#define NUSERR_IN_PeekFailed    -1
/* nusdas_subc_preset1 のエラー */
#define NUSERR_SP_BadGroup	-1
#define NUSERR_SP_MemShort	-2
/* nusdas_make_mask のエラー */
#define NUSERR_MM_SmallBuf	-1
#define NUSERR_MM_BadType	-5
/* 定義ファイル解読時のエラー */
#define NUSERR_DF_BadNumber	-160
/* nusdas_grid のエラー */
#define NUSERR_GD_BadParam      -5
