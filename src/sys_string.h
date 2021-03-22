/** @file
 * @brief 文字列 (ヌル終端 char *) の処理 (標準ライブラリにないもの)
 */

#ifndef NUSDAS_CONFIG_H
# error "include config.h"
#endif
#ifndef toupper
#  include <ctype.h>
#endif

/** ヌル終端文字列の比較 */
#define streq(a, b)	(strcmp((a), (b)) == 0)

#ifdef NEED_STRING_UPCASE
/** ヌル終端文字列を大文字に変換する。
 * \warning 文字列を上書きする。
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
/** @brief strncpy(3) の改良版 (C の文字列の複写)
 *
 * n バイト以上書き込むことはない。必ずヌル終端する。
 * つまり、最大で n - 1 バイトの文字列が得られる。
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
/** @brief strncat(3) の改良版 (C の文字列の連結)
 *
 * 標準 strncat の第3引数とは意味が違う.
 * dest のは n バイトの配列で、必ずヌル終端する。
 * つまり、最大で n - 1 バイトの文字列が得られる。
 * @retval dest コピーができた。
 * @retval NULL コピーするには不十分な容量しかなかった。
 * @note dest + strlen(dest) は書き込み可と仮定している。
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
/** @brief 固定長文字列の複製 (内部でnus_mallocする)
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
/** @brief C文字列の複製 (内部でnus_mallocする)
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
