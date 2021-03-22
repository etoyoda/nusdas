/** @file
 * @brief �ե�����ν���
 */

#include "config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h> /* for LONG_MAX */
#include "sys_mem.h"
#include "sys_file.h"
#ifdef HAVE_GPFS_GPL_H
#include <gpfs_gpl.h>
#endif

/** file_read ���ޤ��ɤ�Ǥߤ�Ĺ����
 * ����� NAPS8 �롼���������ե�����κ���Ĺ�� 8192 ����������
 * �����Τ��ᤳ��2�ܤ�Ȥä���Ρ�
 */
#define BUFINI	16384	

/** @brief Ϳ����줿�ե����� pathname �����Τ��ɤ���֤���
 *
 * size �ˤϥե������Ĺ�������롣
 * �ֵ��ͤϥե���������ƤǤ��롣
 * ����ʸ����� nus_malloc(3) ���줿��Τǡ�ǰ�Τ��� NULL ��ü����Ƥ��뤬��
 * ����ʳ��ξ��� NUL �Х��Ȥ�ޤ��ǽ�������롣
 * �ʤ������ϥ��顼���ˤϾ����Ǥ��ɤ�Ƥ���Ф�����֤��Τǡ�
 * �ե������Ĺ�������Τ�ɽ���ʤ���ǽ�������롣
 */
	unsigned char *
file_read(const char *pathname, size_t *size)
{
	int	fd = -1;
	unsigned char	*buf = NULL;
	ssize_t	r;
	buf = nus_malloc(BUFINI);
	if (buf == NULL)
		goto NG;
	if (check_tape(pathname))
		goto NG;
	fd = open(pathname, O_RDONLY);
	if (fd == -1)
		goto NG;
	r = read(fd, buf, BUFINI);
	if (r < 0) {
		goto NG;
	} else if (r < BUFINI) {
		buf = nus_realloc(buf, r + 1);
		if (buf == NULL)
			goto NG;
		*size = r;
		goto OK;
	} else {
		struct stat st;
		r = fstat(fd, &st);
		if (r < 0) {
			*size = BUFINI - 1;
			goto OK;
		}
		buf = nus_realloc(buf, st.st_size + 1);
		if (buf == NULL)
			goto NG;
		r = read(fd, buf + BUFINI, st.st_size - BUFINI);
		if (r < 0) {
			*size = BUFINI;
			goto OK;
		}
		*size = BUFINI + r;
		goto OK;
	}
OK:
	close(fd);
	buf[*size] = '\0';
	return buf;
NG:
	if (fd >= 0)
		close(fd);
	if (buf)
		nus_free(buf);
	return NULL;
}

/** @brief �ե�������ɤ� (Ĺ������)
 * �ºݤ��ɤ߹��᤿�Х��ȿ����֤���0 �ϥ��顼��
 * �ܴؿ�����Ū�� INFO �쥳���ɽ�����ʤΤǡ������˥���ˤʤ뤳�ȤϤʤ���
 */
	size_t
file_read_size(unsigned char *buf, size_t size, const char *pathname)
{
	int	fd = -1;
	ssize_t	r;
	if (check_tape(pathname))
		return 0;
	fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		return 0;
	}
	r = read(fd, buf, size);
	close(fd);
	if (r < 0) {
		/* read error */
		return 0;
	} else if (r < size) {
		/* file was shorter than buffer */
		memset(buf + r, '\0', size - r);
	}
	return r;
}

/** @bfief �ե�����Ĺ��Ĵ�٤�
 * 0 �ϥ��顼��
 * �ܴؿ�����Ū�� INFO �쥳���ɽ�����ʤΤǡ������˥���ˤʤ뤳�ȤϤʤ���
 */
	size_t
file_size(const char *pathname)
{
	struct stat st;
	if (stat(pathname, &st) == -1) {
		return 0;
	}
	return st.st_size;
}

static int
make_dirs_recursive(char *pathname)
{
	char *slash;
	int	r;
	r = mkdir(pathname, 0777);
	if (r == -1 && errno == EEXIST) {
		r = 0;
	}
	if (r == -1 && errno == ENOENT) {
		slash = strrchr(pathname, '/');
		if (slash == NULL) {
			return -1;
		}
		*slash = '\0';
		r = make_dirs_recursive(pathname);
		*slash = '/';
		if (r == 0) {
			r = mkdir(pathname, 0777);
			if (r == -1 && errno == EEXIST) {
				r = 0;
			}
		}
	}
	return r;
}

/** @brief Ϳ����줿�ե�ѥ��οƤΥǥ��쥯�ȥ��Ƶ�Ū�˺������롣
 */
int
make_dirs(const char *pathname)
{
	char *buf;
	char *slash;
	int r;
	if ((buf = nus_malloc(strlen(pathname) + 1)) == NULL) {
		return -1;
	}
	strcpy(buf, pathname);
	slash = strrchr(buf, '/');
	if (slash == NULL) {
		r = -1;
	} else {
		*slash = '\0';
		r = make_dirs_recursive(buf);
	}
	nus_free(buf);
	return r;
}

/** @brief �ơ��פ����Ѥ���Ķ����оݤ��ơ��׾�Υե����뤫��ǧ����
 *
 * �ơ��פˤ����ե����뤬�ʤ��ʤ�1, ����ʳ���0���֤�
 */
int
check_tape(const char* pathname)
{
#ifdef USE_GPFS_ARCHIVE_CHK
	gpfs_iattr64_t attr;
	unsigned int st_litemaskP;
	if (gpfs_stat_x(pathname, &st_litemaskP, &attr, sizeof(attr))) {
		return 0;
	} else if (attr.ia_winflags & GPFS_WINATTR_OFFLINE) {
		return 1;
	}
#endif	
	return 0;
}
