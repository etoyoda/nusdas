#define _FILE_OFFSET_BITS 64
#include "config.h"
#include "nusdas.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h> /* for nus_malloc */
#include <string.h> /* for memcpy */
# define NEED_MEMCPY_HTON4
#include "sys_endian.h"
# define NEED_LONG_TO_SI8
#include "sys_int.h"
#include "sys_mem.h"

struct nusdas_bigfile {
	int	bf_fd;
	int	bf_mode;
	int	bf_active;
};

#define POOLSIZE 16

/* ���������򤢤Ƥˤ��Ƥ��� */
static N_BIGFILE Pool[POOLSIZE];

/** @brief �ե�����򳫤�
 *
 * �ե�����򳫤���
 * OS �����ݡ��Ȥ��Ƥ���� 32�ӥåȴĶ��Ǥ�
 * 2GB �ޤ��� 4GB ��Ķ����顼���ե�����򳫤����Ȥ��Ǥ��롣
 *
 * @retval NULL ����
 * @retval ¾ ���������Υݥ��󥿤򺣸�Υե����������Ѥ���
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 * */
	N_BIGFILE *
bfopen(const char *pathname, /**< �ե�����̾ */
	const char *mode /**< �⡼�ɻ��� */)
{
	N_BIGFILE *bf;
	const char *p;
	int i;
	if (mode == NULL)
		return NULL;
	for (i = 0; i < POOLSIZE; i++) {
		if (Pool[i].bf_active == 0) {
			bf = &(Pool[i]);
			bf->bf_active = 1;
			goto Found;
		}
	}
	return NULL;
Found:
	for (p = mode; *p; p++) {
		switch (*p) {
			case 'r':
				bf->bf_mode = O_RDONLY;
				break;
			case 'w':
				bf->bf_mode = O_WRONLY | O_CREAT;
				break;
			case '+':
				if (bf->bf_mode & O_APPEND) {
					bf->bf_mode = O_RDWR | O_CREAT
						| O_APPEND;
				} else {
					bf->bf_mode = O_RDWR | O_CREAT;
				}
				break;
			case 'a':
				bf->bf_mode = O_WRONLY | O_CREAT | O_APPEND;
				bf->bf_active = 0;
				return NULL;
			default:
				/* do nothing */;
		}
	}
	if ((bf->bf_fd = open(pathname, bf->bf_mode, 0666)) == -1) {
		bf->bf_active = 0;
		return NULL;
	}
	return bf;
}

/** @brief �ե�������Ĥ���
 *
 * ���餫���� bfopen() �ǳ����줿
 * �ե����� @p bf ���Ĥ��롣����ʸ�ݥ��� @p bf �򻲾Ȥ��ƤϤʤ�ʤ���
 * @retval 0 ���ｪλ
 * @retval -1 ���顼
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 */
	int
bfclose(N_BIGFILE *bf) /**< �ե����� */
{
	bf->bf_mode = 0;
	return close(bf->bf_fd);
}

/** @brief �ե���������
 *
 * ���餫���� bfopen() �ǳ����줿
 * �ե����� @p bf ���� @p ptr ���ؤ��Хåե��� @p nbytes �Х����ɤ߽Ф���
 * @retval �� �ɤ߽Ф��줿�Х��ȿ� (�ե����������ʤɤǤ� @p nbytes ��꾯�ʤ�)
 * @retval 0 ���礦�ɥե��������������ɤ߽Ф����Ȥ����������顼
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 */
	unsigned long
bfread(void *ptr, /**< �ɤߤ�����Хåե� */
	unsigned long nbytes, /**< �Х��ȿ� */
	N_BIGFILE *bf) /**< �ե����� */
{
	ssize_t r;
	r = read(bf->bf_fd, ptr, nbytes);
	if (r < 0) {
		return 0;
	}
	return r;
}

/** @brief �ե��������
 *
 * ���餫���� bfopen() �ǳ����줿
 * �ե����� @p bf ���Ф��� @p ptr ���ؤ��Хåե����� @p nbytes �Х���
 * �񤭽Ф���
 * @retval �� �񤭹��ޤ줿�Х��ȿ� (���顼���� @p nbytes ��꾯�ʤ����Ȥ�����)
 * @retval 0 ���礦�ɽ񤭹��߳��ϻ��˥��顼�������ä�
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 */
	unsigned long
bfwrite(void *ptr, /**< �񤭹��߸��Хåե� */
	unsigned long nbytes, /**< �Х��ȿ� */
	N_BIGFILE *bf) /**< �ե����� */
{
	ssize_t r;
	r = write(bf->bf_fd, ptr, nbytes);
	if (r < 0) {
		return 0;
	}
	return r;
}

/** @brief �Х��ȥ��������Ѵ��դ��ե���������
 *
 * ���餫���� bfopen() �ǳ����줿
 * �ե����� @p bf ������ @size �Х��ȤΥ��֥������Ȥ� @p nmemb ���ɽФ���
 * �ե�����ˤϥӥå�����ǥ�����ǽ񤫤�Ƥ��뤳�Ȥ����ꤵ�졢
 * ��̤ϵ����˼����ʥХ��ȥ��������� @p ptr �˽񤭽Ф���롣
 * @retval �� �ɤ߹��ޤ줿���֥������ȿ� (���顼���� nmemb ��꾯�ʤ����Ȥ�����)
 * @retval 0 ���礦�ɥե��������������ɽФ����Ȥ����������顼
 * <H3>����</H3>
 * <UL>
 * <LI>���� @p size �ˤդ��路���ͤ� sizeof �黻�Ҥˤ�ä������롣
 * </UL>
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 */
	unsigned long
bfread_native(void *ptr, /**< �ɽФ���Хåե� */
		unsigned long size, /**< ���֥������Ȥ��� */
		unsigned long nmemb, /**< ���֥������ȤθĿ� */
		N_BIGFILE *bf /**< �ե����� */)
{
	unsigned long nbytes, r;
	nbytes = size * nmemb;
	r = bfread(ptr, nbytes, bf);
	if (r < nbytes) {
		return r / size;
	}
	switch (size) {
		case 1:
			/* do nothing */;
			break;
		case 2:
			endian_swab2(ptr, nmemb);
			break;
		case 4:
			endian_swab4(ptr, nmemb);
			break;
		case 8:
			endian_swab8(ptr, nmemb);
		default:
			return 0;
	}
	return r / size;
}

/** @brief �Х��ȥ��������Ѵ��դ��ե��������
 *
 * ���餫���� bfopen() �ǳ����줿
 * �ե����� @p bf ���� @size �Х��ȤΥ��֥������Ȥ� @p nmemb �Ľ񤭹��ࡣ
 * �񤭹���ǡ����ϵ����˼����ʥХ��ȥ��������� @p ptr �����ɤ߹��ޤ졢
 * �ե�����ˤϥӥå�����ǥ�����ǽ񤫤�롣
 * @retval �� �񤭽Ф��줿���֥������ȿ� (���顼���� nmemb ��꾯�ʤ����Ȥ�����)
 * @retval 0 ���顼
 * <H3>����</H3>
 * <UL>
 * <LI>���� @p size �ˤդ��路���ͤ� sizeof �黻�Ҥˤ�ä������롣
 * </UL>
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 */
	unsigned long
bfwrite_native(void *ptr, /**< �ǡ��� */
		unsigned long size, /**< ���֥�������Ĺ */
		unsigned long nmemb, /**< ���֥������ȿ� */
		N_BIGFILE *bf /**< �ե����� */)
{
	unsigned long r;
	size_t nbytes = size * nmemb;
	void *buf;
	if ((buf = nus_malloc(nbytes)) == NULL) {
		return 0;
	}
	switch (size) {
		case 1:
			memcpy(buf, ptr, nmemb);
			break;
		case 2:
			memcpy(buf, ptr, nbytes);
			endian_swab2(ptr, nmemb);
			break;
		case 4:
			memcpy_hton4(buf, ptr, nmemb);
			break;
		case 8:
			memcpy(buf, ptr, nbytes);
			endian_swab8(ptr, nmemb);
			break;
		default:
			nus_free(buf);
			return 0;
	}
	r = bfwrite(buf, nbytes, bf);
	nus_free(buf);
	return r / size;
}

/** @brief �ե�������ּ���
 *
 * ���餫���� bfopen() �ǳ����줿
 * �ե����� @p bf �θ��߰��֤� @p pos �˽񤭽Ф���
 * @retval 0 ���ｪλ
 * @retval -1 ���顼
 * <H3>���</H3>
 * <UL>
 * <LI>64 �ӥå��������ʤ�����ѥ����Ѥ� configure �������
 * N_SI8 �Ϲ�¤�ΤǤ��껻�ѱ黻���Ѥ��뤳�ȤϤǤ��ʤ���
 * </UL>
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 */
	int
bfgetpos(N_BIGFILE *bf, /**< �ե����� */
		N_SI8 *pos /**< ���� */)
{
	off_t opos;
	opos = lseek(bf->bf_fd, 0, SEEK_CUR);
	if (opos == (off_t)-1) {
		return -1;
	}
	*pos = off_to_si8(opos);
	return 0;
}

/** @brief �ե������������
 *
 * ���餫���� bfopen() �ǳ����줿
 * �ե����� @p bf �θ��߰��֤�
 * ���餫���� bfgetpos() ������줿���� @p pos �����ꤹ�롣
 * @retval 0 ���ｪλ
 * @retval -1 ���顼
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 */
	int
bfsetpos(N_BIGFILE *bf /**< �ե����� */,
		N_SI8 pos /**< ���� */)
{
	off_t opos;
	opos = si8_to_off(pos);
	opos = lseek(bf->bf_fd, opos, SEEK_SET);
	if (opos == (off_t)-1) {
		return -1;
	}
	return 0;
}

/** @brief �ե������������
 *
 * ���餫���� bfopen() �ǳ����줿
 * �ե����� @p bf �θ��߰��֤����ꤹ�롣
 *
 * �ե�������֤ε��������� @p whence �ˤ�äưۤʤ롣
 * <DL>
 * <DT>SEEK_SET<DD>�ե�������Ƭ���� @p offset �Х��� (����) �ʤ������
 * <DT>SEEK_CUR<DD>���߰��֤��� @p offset �Х��� (��Ǥ�褤) �ʤ������
 * <DT>SEEK_END<DD>�ե������������� @p offset �Х��ȿʤ������
 * </DL>
 * @retval 0 ���ｪλ
 * @retval -1 ���顼
 * <H3>���</H3>
 * <UL>
 * <LI>long �� 32 �ӥå����ξ�硢2 �����Х��Ȥ�Ķ����ե�����Ǥ�
 * ����Ǥ��ʤ���꤬���롣
 * <LI>@p whence �� SEEK_END ����ꤷ������ @p offset ����ꤷ������
 * ��ư�ˤĤ��Ƥ� OS �� lseek(2) ���Υޥ˥奢��򻲾Ȥ��줿����
 * </UL>
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ���ɲä��줿��
 */
	int
bfseek(N_BIGFILE *bf, /**< �ե����� */
		long offset, /**< ���а��� */
		int whence /**< ���� */)
{
	off_t opos;
	opos = lseek(bf->bf_fd, offset, whence);
	if (opos == (off_t)-1) {
		return -1;
	}
	return 0;
}
