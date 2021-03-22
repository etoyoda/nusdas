#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "nusdas.h"
#include "stringplus.h"

#if 0
static char rcsid[] = "$Id: stringplus.c,v 1.1 2007-02-26 00:10:47 suuchi43 Exp $";
#endif

/* streqi - ��ʸ����ʸ����̵�뤷��ʸ�����Ʊ��
 */
	int
streqi(const char *a, const char *b)
{
	int	ia, ib;
	for (;;) {
		ia = toupper(*a);
		ib = toupper(*b);
		if ((ia == 0) && (ib == 0))
			return 1;
		if (ia != ib)
			return 0;
		a++;
		b++;
	}
}

/* memeqi - ��ʸ����ʸ����̵�뤷��ʸ�����Ʊ��
 */
	int
memeqi(const char *a, const char *b, size_t n)
{
	int	ia, ib;
	size_t	i;
	for (i = 0; i < n; i++) {
		ia = toupper(*a);
		ib = toupper(*b);
		if ((ia == 0) && (ib == 0))
			return 1;
		if (ia != ib)
			return 0;
		a++;
		b++;
	}
	return 1;
}

/* strdup3 - ʸ�����Ϣ��ȳ���դ�
 * ����3�Ĥ�Cʸ�����Ϣ�뤷��malloc �ǳ���դ����ΰ�˳�Ǽ�����֤���
 * �ʥ�ݥ��󥿤϶�ʸ�����Ʊ��뤵��롣
 */
	char *
strdup3(const char *a, const char *b, const char *c) {
	char 		*result;
	unsigned	la, lb, lc;
	if (a == NULL) a = "";
	if (b == NULL) b = "";
	if (c == NULL) c = "";
	la = strlen(a);
	lb = strlen(b);
	lc = strlen(c);
	result = malloc(la + lb + lc + 1);
	if (!result) return NULL;
	strncpy(result, a, la);
	strncpy(result + la, b, lb);
	strncpy(result + la + lb, c, lc);
	result[la + lb + lc] = '\0';
	return result;
}

/* indexc - Fortran �� INDEX �ؿ����б�ʪ
 * C ʸ���� string �����ʸ�� c ������С����κǽ�νи����֤Υ��ե��å�
 * ���֤����ߤĤ���ʤ���С����뤤�� string ���ʥ�ݥ��󥿤��ä��顢
 * -1 ���֤���
 * Fortran �� INDEX �Ȱۤʤꡢ�ֵ��� 0 �ϡֺǽ��ʸ���� c ���פǤ��롣
 */

	int
indexc(const char *string, char c)
{
	char	*p;
	if (string == NULL) return -1;
	p = strchr(string, c);
	if (p == NULL) return -1;
	return p - string;
}

/* string_copy - �������� strncpy(3)
 * src ���� size-1 �Х��Ȥ� dest ��ʣ�̤���ɬ�� NUL �ߤ᤹�롣
 * ���� C ��ʸ���󥳥ԡ��Ȥ��ƻȤ���٤����󥿡��ե�������
 */
	char *
string_copy(char *dest, size_t size, const char *src)
{
	size_t	len;
	len = strlen(src);
	if (len > size - 1)
		len = size - 1;
	strncpy(dest, src, len);
	dest[len] = '\0';
	return dest + len;
}

/* string_to_fstring - C ʸ���󤫤� Fortran ʸ����ؤ��Ѵ�
 * C ʸ���� src ����Ĺ�� size �� Fortran ʸ���� dest ��ʣ�̤��롣
 * Fortran ��ʸ������ʸ��Ʊ�͡�src ��û����Х��ڡ�������ᡢ
 * src ��Ĺ����б�¦���ڤ�ΤƤ롣
 */
	void
string_to_fstring(char *dest, size_t size, const char *src)
{
	size_t len;
	len = strlen(src);
	if (len > size)
		len = size;
	strncpy(dest, src, len);
	if (len < size)
		memset(dest + len, ' ', size - len);
}

/* fstring_to_fstring - Fortran ʸ����ɤ�����ʣ��
 * Ĺ�� ssize �� Fortran ʸ���� src ����Ĺ�� dsize ���ΰ� dest ��ʣ�̤��롣
 * Fortran ��ʸ������ʸ��Ʊ�͡�src ��û����Х��ڡ�������ᡢ
 * src ��Ĺ����б�¦���ڤ�ΤƤ롣
 */
	void
fstring_to_fstring(char *dest, const size_t dsize, const char *src, const size_t ssize)
{
	size_t len;
	len = ssize;
	if (len > dsize)
		len = dsize;
	memmove(dest, src, len);
	if (dsize > len)
		memset(dest + len, ' ', dsize - len);
}

/* fstring_to_string - Fortran ʸ���󤫤� C ʸ����ؤ��Ѵ�
 * Ĺ�� ssize �� Fortran ʸ���� src ����Ĺ�� dsize ���ΰ� dest ��ʣ�̤��롣
 * src �����ζ�����ڤ�ΤƤ�졢NUL �ߤᤵ��� C ʸ����Ȥʤ롣
 * src ��Ĺ����б�¦���ڤ�ΤƤ롣�ֵ��ͤ� dest ��ü�Υݥ��󥿡�
 */
	char *
fstring_to_string(char *dest, const size_t dsize, const char *src, const size_t ssize)
{
	size_t		len, ofs;
	int		spc;
	len = dsize - 1;
	if (len > ssize)
		len = ssize;
	dest[len] = '\0';
	ofs = len - 1;
	spc = 1;
	while (1) {
		if (!spc) {
			dest[ofs] = src[ofs];
		} else if (src[ofs] == ' ' || src[ofs] == '\0') {
			dest[ofs] = '\0';
			len = ofs;
		} else {
			spc = 0;
			dest[ofs] = src[ofs];
		}
		if (ofs == 0) break;
		ofs--;
	}
	return dest + len;
}

/* replace_space - Fortran ʸ���󤫤�ɽ���� C ʸ����ؤΤ��ξ��Ѵ�
 * buf ���� len ʸ�����ϰϤ���޷�ʸ���� '_' ���Ѵ�����NUL �ߤ᤹�롣
 * buf �ˤ� len + 1 ʸ���ν񤭹��߲�ǽ�ΰ褬�ʤ���Фʤ�ʤ���
 */
	int
replace_space(char *buf, unsigned len)
{
	char	*p;
	for (p = buf; p < (buf + len); p++) {
		if (*p == '\0') break;
		if (!isgraph(*p)) *p = '_';
	}
	while (p < buf + len) {
		 *p++ = '_';
	}
	buf[len] = '\0';
	return 0;
}

/* fmatch - ������������ɽ���ѥå�����
 * text �� pattern �˥ޥå�����ʤ鿿(����)���֤�
 */

	/* fmatch �Ѥκ���ѥ��֥롼���� */
	static int
match_star(const char *pattern, const char *text)
{
	for (;;) {
		if (fmatch(pattern, text))
			return 1;
		if (*text == '\0')
			return 0;
		text++;
	}
}

	int
fmatch(const char *pattern, const char *text)
{
	int	r;
	if (pattern[0] == '\0')
		return *text == '\0';
	if (pattern[0] == '*')
		return match_star(pattern+1, text);
	if (*text == '\0')
		return 0;
	if (*pattern == '?' || *pattern == *text) {
		r = fmatch(pattern+1, text+1);
		return r;
	}
	return 0;
}
