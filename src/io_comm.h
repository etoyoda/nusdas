/** @file
 * @brief �Ƽ������ϥ⥸�塼��ζ��̥إå�
 */

#ifndef NUSDAS_CONFIG_H
# error please include "config.h"
#endif

/* ����ϲ� */
union nusio_t;

/** @brief �Ƽ������ϥ⥸�塼��˶��̤���᥽�åɤ�ɽ */
struct io_functab {
	/** @brief �Ĥ��� @see io_close() */
	int (*xio_close)(union nusio_t *io, N_SI8 totalsize, const char* filename);
	/** @brief �ɹ�ͽ�� @see io_load() */
	int (*xio_load)(union nusio_t *io, N_SI8 ofs, N_SI8 size);
	/** @brief �ɹ���̤λ��� @see io_peek() */
	void *(*xio_peek)(union nusio_t *io, N_SI8 ofs, N_SI8 size);
	/** @brief ¨���ɹ� */
	N_SI8 (*xio_read)(union nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf);
	/** @brief ¨����� */
	int (*xio_write)(union nusio_t *io, N_SI8 ofs, N_SI8 size, void *buf);
	/** @brief ���ͽ�� @see io_getwbuf() */
	void *(*xio_getwbuf)(union nusio_t *io, N_SI8 ofs, N_SI8 size);
	/** @brief ������� @see io_issue() */
	int (*xio_issue)(union nusio_t *io, N_SI8 ofs, N_SI8 size);
};

/** @brief �Ĥ���
 *
 * �ե�����ϥ�ɥ� (nusio_t) @p io �λؤ��ե�������Ĥ��롣
 * ���Υϥ�ɥ�Ϥ�������ѤǤ��ʤ���
 * @retval 0 ���ｪλ
 * @retval -1 ���顼
 * @see sio_close(), pio_close(), eio_close()
 */
#define io_close(io,size,filename) \
	((io)->comm.methods.xio_close((io),(size),(filename)))

/** @brief �ɹ�ͽ��
 *
 * �ե����� (nusio_t) @p io ��
 * ��Ƭ���� @p ofs �Х����ܤ��� @p size �Х��Ȥ�
 * @p io ���֥������������˳��ݤ����Хåե����ɤ߹���Ǥ����褦�ؼ����롣
 * �ǡ����� io_peek() �Ǽ��Ф���
 * ���δؿ��Υ꥿��������ɹ������뤷�Ƥ����ݾڤϤʤ����Ĥޤꡢ
 * �����Υ��顼�� io_peek() �ΰ۾ｪλ�ˤ�ä����Τ���롣
 * @retval 0 ���ｪλ
 * @retval -1 ���顼
 */
#define io_load(io, ofs, size)	\
	((io)->comm.methods.xio_load((io), (ofs), (size)))

/** @brief �ɹ���̤λ���
 *
 * �ե����� (nusio_t) @p io ����Ƭ���� @p ofs �Х����ܤ���
 * @p size �Х��Ȥ����Ƥ�
 * �񤭹��ޤ�Ƥ������ؤΥݥ��󥿤��֤���
 * �ϰϤ�ľ���� io_load() �ǻؼ������ϰ���Ǥ���й�®�Ǥ��롣
 * �������֤��줿�ݥ��󥿤ϡ�Ʊ�� @p io ������Ȥ��� io_load(),
 * io_getwbuf(), io_close(), io_write() �Τ����줫���ƤФ��ޤǤ�ͭ���Ǥ��롣
 * @retval NULL ���顼
 * @retval ����¾ ���ｪλ
 * @note ���δؿ����֤��ݥ��󥿤򾡼�� nus_free() ���ƤϤʤ�ʤ���
 */
#define io_peek(io, ofs, size)	\
	((io)->comm.methods.xio_peek((io), (ofs), (size)))

/** @brief ¨���ɹ�
 *
 * �ե����� (nusio_t) @p io ����Ƭ���� @p ofs �Х����ܤ��� @p size �Х��Ȥ�
 * ���ѼԤ��Ѱդ�������֥�å� @p buf ���ɤ߹��ࡣ
 * @retval ���� �ɤ߹��ޤ줿�Х��ȿ�
 * @retval (N_SI8)-1 ���顼
 */
#define io_read(io, ofs, size, buf)	\
	((io)->comm.methods.xio_read((io), (ofs), (size), (buf)))

/** @brief ¨���񤭹���
 *
 * �ե����� (nusio_t) @p io ��
 * ��Ƭ���� @p ofs �Х����ܤ���
 * @p size �Х��Ȥ����ѼԤ��Ѱդ�������֥�å� @p buf ����񤭹��ࡣ
 * @retval 0 ���ｪλ
 * @retval -1 ���顼
 *    (write(2) �����ｪλ�����������Υǡ�������ڤ�Ƥ��ʤ�����ޤ�)
 */
#define io_write(io, ofs, size, buf)	\
	((io)->comm.methods.xio_write((io), (ofs), (size), (buf)))

/** @brief ���ͽ��
 *
 * �ե����� (nusio_t) @p io ����Ƭ���� @p ofs �Х����ܤ��� @p size �Х��Ȥ�
 * �񤭹��ि��ΥХåե�����ݤ������Υݥ��󥿤��֤���
 * �ե����륤�᡼���򤽤Υ���˽񤭹�����塢
 * io_issue() �ˤ�äƥե�����ؤν�Ф����Ϥ��롣
 * �������֤��줿�ݥ��󥿤ϡ�Ʊ�� @p io ������Ȥ��� io_issue(), io_load(),
 * io_getwbuf(), io_close(), io_write() �Τ����줫���ƤФ��ޤǤ�ͭ���Ǥ��롣
 * @retval NULL ���顼
 * @retval ����¾ ���ｪλ
 */
#define io_getwbuf(io, ofs, size) \
	((io)->comm.methods.xio_getwbuf((io), (ofs), (size)))

/** @brief �������
 *
 * �ե����� (nusio_t) @p io ����Ƭ���� @p ofs �Х����ܤ˴�Ϣ�դ���
 * ���餫���� io_getwbuf() �ǳ��ݤ����Хåե�����Ƭ���� @p size �Х��Ȥ�
 * �ե�����˽񤭹��߳��Ϥ��롣
 * ���δؿ��Υ꥿������˽񤭹��ߤ���λ���Ƥ����ݾڤϤʤ������ʤ��
 * �����Υ��顼�ϸ�³���� io �ؿ��ΰ۾ｪλ�ˤ�ä����Τ���롣
 * �������äơ� io_issue() ȯ�Ը��
 * io_getwbuf() ���֤����ݥ��󥿤λؤ������ΰ�˽񤭹���ǤϤʤ�ʤ���
 * @retval 0 ���ｪλ
 * @retval ����¾ ���顼
 */
#define io_issue(io, ofs, size) \
	((io)->comm.methods.xio_issue((io), (ofs), (size)))

/** @brief �Ƽ������ϥ⥸�塼�뤬��ͭ������� */
struct io_common_t {
	/** @brief �ؿ��ơ��֥� */
	struct io_functab methods;
	/** @brief io_open() ���Ϥ����ե饰, �����
	 * io_open() �ˤ�ä����Τ������� */
	int	flags;
};

/* �ե饰�ӥå���� --- io_open() �ΰ������Ϥ���� */
/** �ɤ߹��߲�ǽ�ӥå� */
#define IO_READABLE	(1<<0)
/** �񤭹��߲�ǽ�ӥå� */
#define IO_WRITABLE	(1<<1)
/** �ɤ߽�ξ���ĤΥӥå��ȹ礻 */
#define IO_READWRITE		(IO_READABLE | IO_WRITABLE)

/** �ե����뤬��¸�ǤϤʤ����Ȥ򼨤��ե饰. df_open() ������ */
#define IO_CREATED	(1<<8)
/** Fortran �Ȼ�����ʤ륷�����󥷥��ե�����򼨤��ե饰.
 * df_open() ������ */
#define IO_OLDSEQF	(1<<9)
/** ���顼��å��������޻ߤ���ե饰.
 * dds_findfile() ������ */
#define IO_ERRMSG_OFF	(1<<10)

struct io_buffer_t {
	/** �Хåե����� */
	enum io_buffer_status {
		/** @brief �Хåե���ͭ�յ��ʾ������äƤ��ʤ�����
		 *
		 * ̵�� nus_free �ġ�i o_peek() �Բġ�
		 * ��ʣ�����ΰ���Ф��� io_write() ���Ԥ�줿�������
		 */
		IOB_STALE,
		/** @brief �Хåե��˺ǿ��Υǡ������ɤ߹��ޤ�Ƥ������
		 *
		 * ̵�� nus_free �ġ�i o_peek() �ġ�
		 * ����Ū�ˤ�Ʊ���� io_load() �ޤ���
		 * ��Ʊ���� io_peek() �ˤ�ä����Ϥ����뤷�����֡�
		 */
		IOB_FRESH,
		/** @brief �Хåե��˼ΤƤ��ʤ����󤬤��뤫�⤷��ʤ�����
		 *
		 * ̵�� nus_free �Բġ� io_peek() �Բġ�
		 * ����Ū�ˤ� io_getwbuf() ��ξ��֡�
		 */
		IOB_MODIFIED,
		/** @brief �Хåե����Ѥ�����Ʊ�����Ϥ��Ԥ��Ƥ������ 
		 *
		 * ̵�� nus_free �Բġ� io_peek() �Բġ�
		 * ���� io_peek() �ƤӽФ��� IOB_FRESH �����ܤ��롣
		 * ����Ū�ˤ� asyncio_load() [�ѻ�] �¹Ը�ξ��֤�����
		 * ���ޤΤȤ������ξ��֤ˤϤʤ�ʤ���
		 */
		IOB_READING
	} iob_stat;
	/** �Хåե� */
	void	*iob_ptr;
	/** �ǽ�ΥХ��ȤΥե�������ǤΥ��ե��å� */
	N_SI8	iob_head;
	/** �Ǹ�μ��ΥХ��ȤΥ��ե��å� (�Ĥޤ� iob_head + �Хåե�������) */
	N_SI8	iob_fence;
	/** io_issue �ޤ��� io_load �ǰ�̣�Τ���ǡ�����
	 * ���뤳�ȤȻ��ꤵ�줿�Ǹ�μ��ΥХ��ȥ��ե��å� */
	N_SI8	iob_tail;
	/** ���ΥХåե����ե����������Ǥ��������� */
	int	iob_eof;
};

#ifdef NEED_IOBUF_NEW
/** @brief ��¤�� io_buffer_t �ν��������
 * @note �����Υե�����ɤ������ͤΤޤޤȤʤ롣
 * @retval NULL ���顼(������­�Τ�)
 * @retval ����¾ ���ｪλ
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
	/* iob->iob_stat ��̤����Τޤ� */
	/* iob->iob_tail ��̤����Τޤ� */
	/* iob->iob_eof ��̤����Τޤ� */
	return iob;
}
#endif

#ifdef NEED_IOBUF_OUTSIDE
/** @brief �Хåե����׵��ϰ� [@p ofs, ofs + size) �򥫥С��Ǥ��뤫
 * @retval 0 �׵��ϰϤϤ��٤ƥХåե��˺ܤäƤ��롣
 * @retval 1 �׵��ϰϤϥХåե��˺ܤäƤ��ʤ���
 *   �ޤ��ϥХåե��� IOB_FRESH ���֤Ǥʤ���
 *   �����餯���� sio_load() ��Ƥ٤��׵��ϰϤ��ɤ�롣
 * @retval -1 �׵��ϰϤϥХåե��˺ܤäƤ��餺�Хåե����ե����������ˤ��롣
 *   �Ĥޤꤳ��ʾ� sio_load() ��Ƥ�Ǥ�ե����뤬��Ӥʤ��¤�
 *   �׵��ϰϤ��ɤ߼�뤳�ȤϤǤ��ʤ���
 * @warning �����ѥ롼����ǤϻȤ�ʤ����� (�Хåե����֤��㤦����)
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

/** @brief �Хåե��������ϰ� [ofs, ofs + size) �򥫥С��Ǥ��뤫
 * @warning �����ѥ롼����ǤϻȤ�ʤ����� [iobuf_outside() ��Ƥ֥ޥ���Τ���]
 */
#define iobuf_inside(iob, ofs, size) \
	!iobuf_outside((iob), (ofs), (size))

/** @brief �Хåե��������ϰ� [ofs, ofs + size) �򥫥С��Ǥ��뤫��
 * �ޤ��ϥե�������������ã���Ƥ��뤫
 * @warning �����ѥ롼����ǤϻȤ�ʤ����� [iobuf_outside() ��Ƥ֥ޥ���Τ���]
 */
#define iobuf_inside_or_eof(iob, ofs, size) \
	(iobuf_outside((iob), (ofs), (size)) <= 0)

#ifdef NEED_IOBUF_OUTOFFENCE
/** @brief �Хåե��������ϰ� [@p ofs, @p ofs + @p size) �򥫥С����뤫
 *
 * ͭ��Ĺ iob_tail �ǤϤʤ��������դ�Ĺ iob_fence ���Ѥ���
 * @retval 0 ���С�����Ƥ���
 * @retval 1 �Ϥߤ����Ƥ���
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
/** @brief �Хåե��������ϰ� [@p ofs, @p ofs + @p size) �˽Ťʤ뤫
 * @retval 0 �Ťʤ�ʤ�
 * @retval 1 �Ťʤ�
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

/** @brief POSIX (�����ƥॳ����) �ե����������� */
struct ioposix_t {
	/** @brief IO �⥸�塼�붦���� (ɬ����Ƭ���֤�����) */
	struct io_common_t	comm;
	/** @brief �ե����뵭�һ� */
	int			fd;
	/** @brief ���߰��� */
	off_t			pos;
	/** @brief �����ϥХåե�
	 *
	 * ����ͤϥ̥롣
	 * GlobalConfig(pc_wbuffer) ������ΤȤ������줿�ե�����ˤĤ���
	 * Set_Buffer_anyway() ������դ��� Free_Buffer() ������
	 */
	struct io_buffer_t	*iob;
};

/** @brief stdio �饤�֥����� */
struct iostd_t {
	/** @brief IO �⥸�塼�붦���� (ɬ����Ƭ���֤�����) */
	struct io_common_t	comm;
	/** @brief �ե�����
	 *
	 * ������ FILE * ���� stdio.h ��¸�������Ť����ʤ����᷿����Ȥ��� */
	void			*fp;
	/** @brief ���߰��� */
	long			pos;
	/** @brief �����ϥХåե�
	 *
	 * ����ͤϥ̥롣
	 * GlobalConfig(pc_wbuffer) ������ΤȤ������줿�ե�����ˤĤ���
	 * Set_Buffer_anyway() ������դ��� Free_Buffer() ������
	 */
	struct io_buffer_t	*iob;
	/** @brief ľ���˼¹Ԥ����Τ� fread(3) �� fwrite(3) ��
	 *
	 * �ɽ�����ؤ��Ǥϰ��֤��Ѥ��ʤ��Ƥ� fseek(3) ��
	 * ȯ�Ԥ���ɬ�פ����뤿�ᡣ */
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

/** @brief ES �ե����������� */
struct ioesf_t {
	/** @brief IO �⥸�塼�붦���� (ɬ����Ƭ���֤�����) */
	struct io_common_t	comm;
	/** @brief �ե����뵭�һ� */
	int			fd;
	/** @brief ���߰��� */
	offset_t		pos;
	/** @brief �񴹸�Υե�����̾ */
	char			fnam[256];
	/** @brief �����ϥХåե�
	 *
	 * ����ͤϥ̥롣
	 * GlobalConfig(pc_wbuffer) ������ΤȤ������줿�ե�����ˤĤ���
	 * Set_Buffer_anyway() ������դ��� Free_Buffer() ������
	 */
	struct io_buffer_t	*iob;
};
#endif
/* USE_CSES */

/** @brief ���ѥǡ����ե����� */
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
