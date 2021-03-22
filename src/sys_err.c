#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#define NEED_VSNPRINTF
#include "sys_err.h"
#include <unistd.h>

#ifndef MSGBUFLEN
# define MSGBUFLEN 30000
#endif

# define NMARK		2
# define PREFIXLEN	20
# define MAXPRINT	512

static char MessageBuf[MSGBUFLEN];
static unsigned MessageSize = 0;
static unsigned MessageMark[NMARK] = { 0, 0 };
static unsigned MessageMax = MSGBUFLEN;

/** @brief �ǥХå����ϤΥե�����̾
 */
const char *nus_dbg_file = "?";

/** @brief �ǥХå����Ϥι��ֹ�
 */
int nus_dbg_line = 0;

/** @brief nus_debug() �ޥ���ɸ�२�顼���Ϥ˰������뤫�ݤ�
 */
int nus_dbg_enabled = 0;

/** @brief nus_warn() �ޥ���ɸ�२�顼���Ϥ˰������뤫�ݤ�
 */
int nus_wrn_enabled = 1;

/** @brief ���顼������
 * ���ߤ� NuSDaS 1.1 �ߴ� API �Υ��顼�����ɤ�����Ƥ��롣
 */
int nus_errno = 0;

	static const char *
BaseName(const char *fullpath)
{
	const char *basename;
	basename = strrchr(fullpath, '/');
	if (basename && basename[1]) {
		return basename + 1;
	} else {
		return fullpath;
	}
}

	int
nuserr_flush(void)
{
	int r = 0;
	if (MessageSize) {
		r = write(STDERR_FILENO, MessageBuf, MessageSize);
	}
	MessageMark[0] = MessageMark[1] = MessageSize = 0;
	return r;
}

	int
nuserr_final(void)
{
	const char *msg = "E [BUG] unflushed buffer\n";
	if (!MessageSize)
		return 0;
	write(STDERR_FILENO, msg, strlen(msg));
	nuserr_flush();
#if 0
	/** �ե�å��夷˺��Х����դ����硢������ͭ��������
	 * �桼����٥�ץ�ե�����󥰤򤫤��� */
	exit(127);
#endif
	return -1; /* fake */
}

	int
nuserr_mark(int index)
{
	return MessageMark[index] = MessageSize;
}

	int
nuserr_cancel(int index)
{
	int i;
	if (nus_dbg_enabled)
		return 0;
	MessageSize = MessageMark[index];
	for (i = 0; i < NMARK; i++) {
		if (MessageMark[i] > MessageSize)
			MessageMark[i] = MessageSize;
	}
	return MessageSize;
}

	static char *
GetBuffer(void)
{
	if (MessageSize + MAXPRINT >= MessageMax) {
		nuserr_flush();
	}
	return MessageBuf + MessageSize;
}

	static int
AdvanceBuffer(size_t len)
{
	MessageSize += len;
	return len;
}

#ifdef USE_NUS_DEBUG
/** @brief �ǥХå����� (ľ�ܸƤ֤����� nus_debug() �ޥ������Ѥ��뤳��)
 */
	int
nuserr_dbg(const char *fmt, ...)
{
    va_list ap;
    int len;
    char *buf = GetBuffer();
    nusdas_snprintf(buf, PREFIXLEN + 4, "D %12.12s:%-4d    ",
	BaseName(nus_dbg_file), nus_dbg_line);
    va_start(ap, fmt);
    len = nus_vsnprintf(buf + PREFIXLEN, MAXPRINT - PREFIXLEN - 1, fmt, ap);
    va_end(ap);
    buf[PREFIXLEN + len] = '\n';
    AdvanceBuffer(PREFIXLEN + len + 1);
    return 0;
}
#endif

/** @brief �ٹ��å�������ɸ�२�顼���Ϥ�ɽ����
 */
	int
nuserr_wrn(const char *fmt, ...)
{
    va_list ap;
    int len;
    char *buf = GetBuffer();
    nusdas_snprintf(buf, PREFIXLEN + 4, "W %12.12s:%-4d    ",
	BaseName(nus_dbg_file), nus_dbg_line);
    va_start(ap, fmt);
    len = nus_vsnprintf(buf + PREFIXLEN, MAXPRINT - PREFIXLEN - 1, fmt, ap);
    va_end(ap);
    buf[PREFIXLEN + len] = '\n';
    AdvanceBuffer(PREFIXLEN + len + 1);
    return 0;
}

/** @brief ���顼��å�������ɸ�२�顼���Ϥ�ɽ����
 *
 * @return errkey ���������Τޤ��֤���롣
 */
int nuserr_err(int errkey, const char *fmt, ...)
{
    va_list ap;
    int len;
    char *buf = GetBuffer();
    if (errkey) {
	    nus_errno = errkey;
    }
    nusdas_snprintf(buf, PREFIXLEN + 4, "E %12.12s:%-4d    ",
	BaseName(nus_dbg_file), nus_dbg_line);
    va_start(ap, fmt);
    len = nus_vsnprintf(buf + PREFIXLEN, MAXPRINT - PREFIXLEN - 1, fmt, ap);
    va_end(ap);
    buf[PREFIXLEN + len] = '\n';
    AdvanceBuffer(PREFIXLEN + len + 1);
    return errkey;
}
