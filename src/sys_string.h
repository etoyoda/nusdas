/** @file
 * @brief ʸ���� (�̥뽪ü char *) �ν��� (ɸ��饤�֥��ˤʤ����)
 */

#ifndef NUSDAS_CONFIG_H
# error "include config.h"
#endif
#ifndef toupper
#  include <ctype.h>
#endif

/** �̥뽪üʸ�������� */
#define streq(a, b)	(strcmp((a), (b)) == 0)

#ifdef NEED_STRING_UPCASE
/** �̥뽪üʸ�������ʸ�����Ѵ����롣
 * \warning ʸ������񤭤��롣
 */
INLINE
void
string_upcase(char *str)
{
	while (*str) {
		*str = toupper(*str);
		str++;
	}
}
#endif

#ifdef NEED_STRING_COPY
/** @brief strncpy(3) �β����� (C ��ʸ�����ʣ��)
 *
 * n �Х��Ȱʾ�񤭹��ळ�ȤϤʤ���ɬ���̥뽪ü���롣
 * �Ĥޤꡢ����� n - 1 �Х��Ȥ�ʸ���������롣
 */
INLINE
char *
string_copy(char * const dest, const char *src, const size_t n)
{
	char *p = dest;
	while (p < (dest + n - 1)) {
		*p++ = *src++;
		if (*src == '\0')
			break;
	}
	*p = '\0';
	return dest;
}
#endif

#ifdef NEED_STRING_CAT
/** @brief strncat(3) �β����� (C ��ʸ�����Ϣ��)
 *
 * ɸ�� strncat ����3�����Ȥϰ�̣���㤦.
 * dest �Τ� n �Х��Ȥ�����ǡ�ɬ���̥뽪ü���롣
 * �Ĥޤꡢ����� n - 1 �Х��Ȥ�ʸ���������롣
 * @retval dest ���ԡ����Ǥ�����
 * @retval NULL ���ԡ�����ˤ��Խ�ʬ�����̤����ʤ��ä���
 * @note dest + strlen(dest) �Ͻ񤭹��߲ĤȲ��ꤷ�Ƥ��롣
 */
INLINE
char *
string_cat(char * const dest, const char *src, const size_t n)
{
	char *p;
	const char *q;
	p = dest + strlen(dest);
	q = src;
	while (1) {
		*p = *q;
		if (*p == 0) {
			return dest;
		} else if (p == dest + n - 1) {
			*p = 0;
			return NULL;
		}
		p++, q++;
	}
}
#endif

#ifdef NEED_CHARS_DUP
/** @brief ����Ĺʸ�����ʣ�� (������nus_malloc����)
 */
INLINE
char *
chars_dup(const char *string, size_t n)
{
	char *p;
	p = nus_malloc(n);
	if (p) {
		memcpy(p, string, n);
	}
	return p;
}
#endif

#ifdef NEED_STRING_DUP
/** @brief Cʸ�����ʣ�� (������nus_malloc����)
 */
INLINE
char *
string_dup(const char *string)
{
	size_t n;
	char *p;
	p = nus_malloc(n = strlen(string) + 1);
	if (p) {
		memcpy(p, string, n);
	}
	return p;
}
#endif

#ifdef NEED_MEMCPY4
INLINE void
memcpy4(char *dest, const char *src)
{
	int i;
	for (i = 0; i < 4; i++) dest[i] = src[i];
}
#endif

#ifdef NEED_MEMCPY6
INLINE void
memcpy6(char *dest, const char *src)
{
	int i;
	for (i = 0; i < 6; i++) dest[i] = src[i];
}
#endif

#ifdef NEED_MEMCPY8
INLINE void
memcpy8(char *dest, const char *src)
{
	int i;
	for (i = 0; i < 8; i++) dest[i] = src[i];
}
#endif

#ifdef NEED_MEMCPY12
INLINE void
memcpy12(char *dest, const char *src)
{
	int i;
	for (i = 0; i < 12; i++) dest[i] = src[i];
}
#endif
