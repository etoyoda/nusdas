/** @file
 * @brief ���顼����
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

/** @brief �ǥХ�����
 *
 * nusdas_iocntl(N_IO_WARNING_OUT, 1�ʲ�) �ΤȤ��޻ߤ���롣
 */
#ifdef USE_NUS_DEBUG
# define nus_debug(args) \
		(nus_dbg_enabled ? \
		(nus_dbg_file = __FILE__, nus_dbg_line = __LINE__, \
		nuserr_dbg args) : 0)
#else
# define nus_debug(args)	 
#endif

/** @brief �ٹ����
 *
 * nusdas_iocntl(N_IO_WARNING_OUT, 0) �ΤȤ��޻ߤ���롣
 */
# define nus_warn(args) \
		(nus_wrn_enabled ? \
		(nus_dbg_file = __FILE__, nus_dbg_line = __LINE__, \
		nuserr_wrn args) : 0)

/** @brief ���顼��å�����
 */
# define nus_err(args) \
		(nus_dbg_file = __FILE__, nus_dbg_line = __LINE__, \
		 nuserr_err args)

/** @brief API �ѥ��顼������
 */
#define NUS_ERRNO (nus_errno)
#define NUS_ERR_CODE() ((signed char)nus_errno)

#ifdef USE_NUS_DEBUG
# define SETERR(errkey) \
	(nus_debug(("SETERR(%s)", #errkey)), nus_errno = (errkey))
#else
# define SETERR(errkey) (nus_errno = (errkey))
#endif

/* ���������Ȥ����ֹ� */
#define NUSERR_INITVALUE       -127  /* API �Ϥ��Υ��顼���֤��ƤϤʤ�ʤ� */

/* �ޤ�������Ƥ��Ƥ��ʤ��ֹ� */
#define NUSERR_NotImplemented   -88
#define NUSERR_Internal         -88
#define NUSERR_OpenRFailed	-87 /* API �Ϥ��Υ��顼���֤��ƤϤʤ�ʤ� */
/* nusdas_onefile_close ���ֵ��� */
#define NUSERR_OC_NoOpenDfile	1
/* ������ */
/* malloc �˼��� */
#define NUSERR_MemShort		-10
/* type1 �ξ�ά�������֤˼��� */
#define NUSERR_BadType1		-13
/* �ǡ����ե�����ΥС�������ֹ椬���� */
#define NUSERR_BadVersion       -18
/* ���ꤵ�줿TYPE�Υǡ������åȤ����Ĥ���ʤ��ä� */
#define NUSERR_DsetNotFound	-21
#define NUSERR_DfileOpenFail	-21
#define NUSERR_DefOrder		-33
#define NUSERR_BadElementmap	-34
/* ����ե����������ͽ������ EOF �����ä� */
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
/* NUSD ���ɤ߹��ߤ˼��� */
#define NUSERR_NUSDReadFail	-54
/* CNTL ���ɤ߹��ߤ˼��� */
#define NUSERR_CNTLReadFail	-55
/* INDX ���ɤ߹��ߤ˼��� */
#define NUSERR_INDXReadFail	-56
/* END ���ɤ߹��ߤ˼��� */
#define NUSERR_ENDReadFail	-57
/* DATA �ɤ߹��ߤ˼��� */
#define NUSERR_DATAReadFail	-59

#define	NUSERR_WildcardBTFail	-58

/* �����쥳����Ĺ���쥳����Ĺ��Ĺ��                            
   (NuSDaS1.3����Ͻ�������˸��ꤷ�ʤ�)
*/
#define NUSERR_ForcedRlen	-63
/* info file ���ɤ߹��ߤ˼��� */
#define NUSERR_NoInfoFile	-64
/* close ���� NUSD �ν񤭹��ߤ˼��� */
#define NUSERR_NUSDWriteFail    -65
/* close ���� CNTL or INDX �ν񤭹��ߤ˼��� */
#define NUSERR_CNTLWriteFail    -66
#define NUSERR_INDXWriteFail    -66
/* close ���� END �ν񤭹��ߤ˼��� */
#define NUSERR_ENDWriteFail     -67

#define	NUSERR_DfileUnwritable	-68 /* source: doc; not tested 11 */
#define NUSERR_Over4GiB		-80
#define NUSERR_BadGrid		-81 /* new in 1.3 */
#define NUSERR_BuildTemplFail   -82 /* new in 1.3 */
#define NUSERR_ReadOnly		-69 /* source: doc; not tested 11 */
#define NUSERR_NoZLib		-98 /* new in 1.3 */
#define NUSERR_IO		-99 /* source: doc; not tested 11 */
/* nusdas_read �Υ��顼 */
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
/* nusdas_write �Υ��顼 */
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
/* �ǡ����� encode �˼��Ԥ��� */
#define NUSERR_WR_EncodeFail    -8
/* nusdas_inq_XXX �Υ��顼 */
#define NUSERR_IQ_ShortBuf	-1
#define NUSERR_IQ_DestNull	-2
#define NUSERR_IQ_BadParam	-3
/* nusdas_subc_XXXX �Υ��顼 */
#define NUSERR_SC_BadGroup	-4
#define NUSERR_SC_BadArg	-6
#define NUSERR_SC_BadInput	-4
#define NUSERR_SC_ShortBuf	-3
#define NUSERR_SC_SizeMismatch	-3
#define NUSERR_SC_NoRecToWrite  -2
#define NUSERR_SC_Uninitialized	-2
#define NUSERR_SC_PeekFailed	-2
/* nusdas_info �Υ��顼 (subc �Ȱ㤦���) */
#define NUSERR_IN_PeekFailed    -1
/* nusdas_subc_preset1 �Υ��顼 */
#define NUSERR_SP_BadGroup	-1
#define NUSERR_SP_MemShort	-2
/* nusdas_make_mask �Υ��顼 */
#define NUSERR_MM_SmallBuf	-1
#define NUSERR_MM_BadType	-5
/* ����ե�������ɻ��Υ��顼 */
#define NUSERR_DF_BadNumber	-160
/* nusdas_grid �Υ��顼 */
#define NUSERR_GD_BadParam      -5
